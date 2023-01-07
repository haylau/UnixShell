#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length command */

// function to redirect out to file
void redirectOut(char* fileName)
{
    int outFile = open(fileName, O_WRONLY | O_CREAT); // opening file to output
    dup2(outFile, STDOUT_FILENO); // duplicating stdout with outFile
    close(outFile); // close the outFile
}

// function to redirect in to file
void redirectIn(char* fileName)
{
    int inFile = open(fileName, O_RDONLY | O_CREAT); // opening file to read input
    dup2(inFile, STDIN_FILENO); // duplicating stdin with inFile
    close(inFile); // close the inFile
}

void createChildProc(char** args, char cmdTerm) {
    pid_t pid = fork();
    switch (pid) {
    case -1: {
        // fork failed
        fprintf(stderr, "Forking error, errno = %d\n", errno);
        exit(1);
        break;
    }
    case 0: {
        // child fork
        if (args[0] == NULL) {
            printf("No commands in history. \n");
        }

        int i = 0;
        while (args[i] != NULL)
        {
            // redirect operators
            if (strcmp(args[i], ">") == 0) // redirecting out
            {
                redirectOut(args[i + 1]); // sending token after '>' to redirect out function
            }
            else if (strcmp(args[i], "<") == 0) // redirecting in
            {
                redirectIn(args[i + 1]); // sending token after '<' to redirect in function
            }
            ++i;
        }

        execvp(args[0], args); // invoking execvp
        break;
    }
    default: {
        // parent fork
        if (cmdTerm == ';') {
            wait(NULL);
        }
        break;
    }
    }
}

// main function
int main(void)
{
    char* args[MAX_LINE / 2 + 1]; /* command line arguments */
    char* history[MAX_LINE / 2 + 1]; // command line history
    int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        // fetch user cmd
        char* input[MAX_LINE];
        size_t len = MAX_LINE;
        ssize_t lineSize = 0;
        lineSize = getline(input, &len, stdin);
        if (lineSize > 0) {
            (*input)[lineSize - 1] = '\0';
        }

        // tokenize user cmd
        const char delim[2] = " ";
        int i = 0;
        char* token = strtok(*input, delim);
        args[i] = token;
        while (token != NULL) {
            // grab token
            token = strtok(NULL, delim);
            ++i;
            args[i] = token;
        }

        // fork for every command found
        char* cmd[MAX_LINE / 2 + 1];
        i = 0;
        int j = 0;
        while (args[i] != NULL) {
            // found end of command
            if (strcmp(args[i], ";") == 0 || strcmp(args[i], "&") == 0) {
                cmd[j] = NULL;
                createChildProc(cmd, args[i][0]);
                // empty cmd
                cmd[0] = NULL;
                j = 0;
            }
            // found pipe operator
            else if (strcmp(args[i], "|") == 0) {
                // #TODO pipe?? 
            }
            // else found an arguement
            else {
                cmd[j] = args[i];
            }
            ++i;
            ++j;
        }
        // end of input
        cmd[j] = NULL;
        createChildProc(cmd, ';');

        /**
        * After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait() unless command included &
        */

    }
    return 0;
}

