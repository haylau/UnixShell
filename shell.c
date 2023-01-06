#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MAX_LINE 80 /* The maximum length command */

int main(void)
{
  char **args[MAX_LINE/2 + 1]; /* command line arguments */
  char **history[MAX_LINE/2 +1]; // command line history
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
            
            int j = 0;
            while(args[j] != NULL)
            {
                // redirect operators
                if (strcmp(args[j], ">") == 0)
                {
                    int outFile = open(args[j+1], O_WRONLY | O_CREAT);
                    dup2(outFile, STDOUT_FILENO);
                    close(outFile);
                    args[j] = NULL;
                    args[j+1] = NULL;
                }
                else if (stcmp(args[j], "<") == 0)
                {
                    int inFile = open(args[j+1], O_WRONLY | O_CREAT);
                    dup2(inFile, STDIN_FILENO);
                    close(inFile);
                    args[j] = NULL;
                    args[j+1] = NULL;
                }
                ++j;
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
