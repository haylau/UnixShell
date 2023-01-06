#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MAX_LINE 80 /* The maximum length command */

// function to redirect out to file
void redirectOut(char *fileName) 
{
    int outFile = open(fileName, O_WRONLY | O_CREAT); // opening file to output
    dup2(outFile, STDOUT_FILENO); // duplicating stdout with outFile
    close(outFile); // close the outFile
}

// function to redirect in to file
void redirectIn(char *fileName)
{
    int inFile = open(fileName, O_RDONLY | O_CREAT); // opening file to read input
    dup2(inFile, STDIN_FILENO); // duplicating stdin with inFile
    close(inFile); // close the inFile
}

// main function
int main(void)
{
  char *args[MAX_LINE/2 + 1]; /* command line arguments */
  char *history[MAX_LINE/2 +1]; // command line history
  int should_run = 1; /* flag to determine when to exit program */

  while (should_run) {
    printf("osh>");
    fflush(stdout);

    // fetch user cmd
    char* input[MAX_LINE];
    size_t len = MAX_LINE;
    ssize_t lineSize = 0;
    lineSize = getline(input, &len, stdin);
    if (len > 0) {
        (*input)[len - 1] = '\0';
    }

    // tokenize user cmd
    char* token = strtok(input, ' ');
    int i = 0;
    *args[i] = token;
    while(token != NULL) {
        // grab token
        token = strtok(token, ' ');
        ++i;    
        *args[i] = token;
    }
    
    pid_t pid = fork();
    switch(pid) {
        case -1 : {
            // fork failed
            fprintf(stderr, "Forking error, errno = %d\n", errno);
            exit(1);
            break;
        }
        case 0 : {
            // new fork
            if (args[0] == NULL) {
                printf("No commands in history. \n");
            }
            
            int i = 0;
            while(args[i] != NULL)
            {
                // redirect operators
                if (strcmp(args[i], ">") == 0) // redirecting out
                {
                    redirectOut(args[i+1]); // sending token after '>' to redirect out function
                }
                else if (stcmp(args[i], "<") == 0) // redirecting in
                {
                    redirectIn(args[i+1]); // sending token after '<' to redirect in function
                }
                ++i;
            }
        
            execvp(args[0], args); // invoking execvp
            break;
        }
        default : {
            // old fork 

            break;
        }
    }

    /**
    * After reading user input, the steps are:
    * (1) fork a child process using fork()
    * (2) the child process will invoke execvp()
    * (3) parent will invoke wait() unless command included &
    */

  }
  return 0;
}

