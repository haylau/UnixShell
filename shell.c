#include <stdio.h>
#include <unistd.h>

#define MAX_LINE 80 /* The maximum length command */

int main(void)
{
  char **args[MAX_LINE/2 + 1]; /* command line arguments */
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
            perror("Fork Error");
            exit(1);
            break;
        }
        case 0 : {
            // new fork
            if (args[0] == NULL) {
                printf("No commands in recent history. \n");
            }

            // redirect stuff
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
