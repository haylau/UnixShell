/*
*
* CSS 430
* Due Date: 1/18/23
* UNIX Shell
* shell.c
* Authors: Hayden Lauritzen and Danny Kha
*
* This program emulates a UNIX shell in C that accepts user commands and can execute
* each command in seperate processes
*
* The following commands can be preformed:
* ls
* ls -al
* ls & whoami;
* !!
* ls > junk.txt
* cat < junk.txt
* ls | wc
* ps auxf | cat | tac | cat | tac | grep 'whoami'
* ascii
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length command */
#define MAX_LEN (MAX_LINE / 2 + 1) /* buffer size */

typedef int bool;
const int true = 1;
const int false = 0;

// function to redirect out to file
void redirectOut(char* fileName)
{
    int outFile = open(fileName, O_WRONLY | O_CREAT, 0777); // opening file to output
    dup2(outFile, STDOUT_FILENO); // duplicating stdout with outFile
    close(outFile); // close the outFile
}

// function to redirect in to file
void redirectIn(char* fileName)
{
    int inFile = open(fileName, O_RDONLY | O_CREAT, 0777); // opening file to read input
    dup2(inFile, STDIN_FILENO); // duplicating stdin with inFile
    close(inFile); // close the inFile
}


char** parsePipe(char** args) {
    // turn {"cat", "file.txt", "|", "grep", "hello", "|", "head", "-1"} 
    // into {{"cat", "file.txt"}, {"grep", "hello"}, {"head", "-1"}
}

// function where piped system commands are executed
// 0 is read end and 1 is write end
void createPipeProc(char** args)
{
    pid_t pid; // create a new pid
    int fd[2]; // create a fd

    pipe(fd); // create a pipe

    switch (pid = fork()) { // switch case for the new process
    case 0: // child 1 is executing and only needs to write 
        
        // createChildPipe(right side of pipe)

        
        // dup2(fd[0], 0);
        // close(fd[1]);
        // execvp(args[2], args);
        // perror(args[2]);
    default: // parent is executing and only needs the read end
        dup2(fd[1], 1);
        close(fd[0]);
        execvp(args[0], args);
        perror(args[0]);
    case -1:
        perror("fork");
        exit(1);
    }
}

void createChildPipe(char** args) {
    pid_t pid; // create a new pid
    int fd[2]; // create a fd

    pipe(fd); // create a pipe

    switch (pid = fork()) { // switch case for the new process
    case 0: // child 1 is executing and only needs to write 
        
        // if num pipes left > 2 createChildPipe(right side of pipe)

        // else 
        // dup2(fd[0], 0);
        // close(fd[1]);
        // execvp(args[2], args);
        // perror(args[2]);
    default: // parent is executing and only needs the read end
        dup2(fd[1], 1);
        close(fd[0]);
        execvp(args[0], args);
        perror(args[0]);
    case -1:
        perror("fork");
        exit(1);
    }
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
        if (args[0] == NULL) break;
        int i = 0;
        while (args[i] != NULL && args[i + 1] != NULL)
        {
            // redirect operators
            if (strcmp(args[i], ">") == 0) // redirecting out
            {
                redirectOut(args[i + 1]); // sending token after '>' to redirect out function
                args[i] = NULL;
                args[i + 1] = NULL;
            }
            else if (strcmp(args[i], "<") == 0) // redirecting in
            {
                redirectIn(args[i + 1]); // sending token after '<' to redirect in function
                args[i] = NULL;
                args[i + 1] = NULL;
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
    char* args[MAX_LEN]; /* tokenized command line arguments */
    char* history = NULL; /* command line history; contains malloc'd ptrs */
    bool should_run = true; /* flag to determine when to exit program */

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        // fetch user cmd
        size_t len = MAX_LINE;
        ssize_t lineSize = 0; // length of string
        char* input = (char*)malloc(len);
        lineSize = getline(&input, &len, stdin);
        if (lineSize > 0) {
            (input)[lineSize - 1] = '\0';
        }

        // tokenize user cmd
        char* temp = strdup(input); // preserve original input
        const char delim[2] = " ";
        char* token = strtok(temp, delim);
        // most recent cmd requested
        if (strcmp(token, "exit") == 0) {
            if (input != NULL) free(input);
            if (temp != NULL) free(temp);
            break; // exit shell
        }
        if (strcmp(token, "ascii") == 0) {
            // todo ascii extra credit

        }
        if (strcmp(token, "!!") == 0) {
            if (history == NULL) {
                printf("No command history.");
            }
            else {
                if (input != NULL) free(input);
                if (temp != NULL) free(temp);
                // fetch from history
                input = strdup(history);
                temp = strdup(input);
                // update new first token
                token = strtok(temp, delim);
                // print prev cmd
                printf("%s\n", input);
            }
        }
        // grab rest of input tokens
        int i = 0;
        args[i] = token;
        while (token != NULL) {
            // grab token
            token = strtok(NULL, delim);
            ++i;
            args[i] = token;
        }

        /**
        * After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait() unless command included &
        */
        char* cmd[MAX_LEN];
        bool createPipe;
        i = 0;
        int j = 0;
        while (args[i] != NULL) {
            // found end of command
            if (strcmp(args[i], ";") == 0 || strcmp(args[i], "&") == 0) {
                cmd[j] = NULL;
                if (createPipe) { // if a pipe is needed, create one
                    parsePipe(cmd);
                    createPipe = false;
                }
                else { // else execute cmd
                    createChildProc(cmd, args[i][0]);
                }
                // empty cmd
                cmd[0] = NULL;
                j = 0;
            }
            // found pipe operator
            else if (strcmp(args[i], "|") == 0) {
                createPipe = true;
                cmd[j] = args[i];
                ++j;
            }
            // else found an arguement
            else {
                cmd[j] = args[i];
                ++j;
            }
            ++i;
        }
        // end of input
        cmd[j] = NULL;
        if (createPipe) { // if a pipe is needed, create one
            parsePipe(cmd);
        }
        else { // else execute cmd
            createChildProc(cmd, ";");
        }
        // add cmd to history
        if (history != NULL) free(history);
        history = strdup(input);
        // free memory
        free(input);
        free(temp);
    }
    // free 
    if (history != NULL) free(history);
    return 0;
}

