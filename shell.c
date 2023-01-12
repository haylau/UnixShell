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
* The following commands have been tested:
* ls
* ls -al
* ls & whoami ;
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
enum { false, true };
enum { READ, WRITE };

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

// function to parse piping. An example is shown here:
// turn {"cat", "file.txt", "|", "grep", "hello", "|", "head", "-1"} 
// into {{"cat", "file.txt"}, {"grep", "hello"}, {"head", "-1"}
char*** parsePipe(char** args) {
    // all char* involed are malloc'd!, call freePipe() on args after

    // output
    char*** ret = (char***)malloc(sizeof(char***) * MAX_LINE);
    memset(ret, '\0', (sizeof(char***) * MAX_LINE));

    // hold individual cmd
    char** cmd = (char**)malloc(sizeof(cmd[0]) * MAX_LINE);
    memset(cmd, '\0', (sizeof(cmd[0]) * MAX_LINE));

    // parse
    int i = 0, j = 0, k = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            cmd[j] = NULL;
            // add string arr to return
            ret[k] = cmd; // cmd will leave function malloc'd
            cmd = (char**)malloc(sizeof(cmd[0]) * MAX_LINE);
            memset(cmd, '\0', MAX_LINE);
            ++k;
            j = 0;
        }
        else {
            // add token to string arr
            cmd[j] = strdup(args[i]); // cmd will leave function holding malloc
            ++j;
        }
        ++i;
    }

    // last cmd
    cmd[j] = NULL;
    ret[k] = cmd; // cmd will leave function malloc'd

    return ret;
}

// free parsed pipe
void freePipe(char*** cmds) {
    // free cmds
    int l = 0, k = 0;
    while (cmds[l] != NULL) {
        while (cmds[l][k] != NULL) {
            free(cmds[l][k]);
            ++k;
        }
        free(cmds[l]);
        k = 0;
        ++l;
    }
    free(cmds);
}

// function where piped system commands are executed
// 0 is read end and 1 is write end
void createPipeProc(char*** args)
{
    // count num commands (one pipe per command - 1)
    int numCommands = 0;
    while (args[numCommands] != NULL) {
        ++numCommands;
    }
    // create a fd for every pipe needed
    int fd[numCommands][2];

    // create pipes between every command 
    for (int i = 0; i < numCommands; i++)
    {
        if (i != numCommands - 1) {
            if (pipe(fd[i]) < 0) {
                perror("Pipe not created\n");
                return;
            }
        }
        if (fork() == 0) { // child process
            // all pipes but the last need to write to stdout
            if (i != numCommands - 1) {
                dup2(fd[i][WRITE], STDOUT_FILENO);
                close(fd[i][READ]);
                close(fd[i][WRITE]);
            }
            // all pipes but the first need to read from stdin
            if (i != 0) {
                dup2(fd[i - 1][READ], STDIN_FILENO);
                close(fd[i - 1][WRITE]);
                close(fd[i - 1][READ]);
            }
            // run command
            execvp(args[i][0], args[i]);
            perror("invalid input ");
            exit(1);
        }
        // close child-end of pipe
        if (i != 0) {
            close(fd[i - 1][READ]);
            close(fd[i - 1][WRITE]);
        }
    }
    // wait for every child to terminate
    for (int i = 0; i < numCommands; i++) {
        wait(NULL);
    }
}

// function that creates child processes
void createChildProc(char** args, bool doWait) {
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
        if (doWait) {
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

        if (token == NULL) {
            free(input);
            free(temp);
            continue; // no cmd was entered
        }

        // most recent cmd requested
        if (strcmp(token, "exit") == 0) {
            if (input != NULL) free(input);
            if (temp != NULL) free(temp);
            should_run = false;
            break; // exit shell
        }
        else if (strcmp(token, "ascii") == 0) {
            // todo ascii extra credit
            printf("  |\\_/|        ****************************    (\\_/)\n / @ @ \\       *  \"Purrrfectly pleasant\"  *   (='.'=)\n( > º < )      *       Poppy Prinz        *   (\")_(\")\n `>>x<<´       *   (pprinz@example.com)   *\n /  O  \\       ****************************\n");
        }
        else {
            if (strcmp(token, "!!") == 0) {
                if (history == NULL) {
                    printf("No command history.");
                }
                else {
                    if (input != NULL) {
                        free(input);
                    }
                    if (temp != NULL) {
                        free(temp);
                    }
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

            // parse tokens to find individual commands
            char* cmd[MAX_LEN];
            bool createPipe;
            i = 0;
            int j = 0;
            while (args[i] != NULL) {
                // found end of command
                if (strcmp(args[i], ";") == 0 || strcmp(args[i], "&") == 0) {
                    cmd[j] = NULL;
                    if (createPipe) { // if a pipe is needed, create one
                        char*** cmds = parsePipe(cmd); // returns malloc'd ptrs!
                        createPipe = false;
                        createPipeProc(cmds);
                        freePipe(cmds); // free's malloc'd ptrs
                    }
                    else { // else execute cmd
                        createChildProc(cmd, (args[i][0] == ';'));
                    }
                    // empty cmd
                    cmd[0] = NULL;
                    j = 0;
                }
                // found pipe operator
                else if (strcmp(args[i], "|") == 0) {
                    createPipe = true; // send command to createPipeProc() instead
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
                char*** cmds = parsePipe(cmd); // returns malloc'd ptrs!
                createPipe = false;
                createPipeProc(cmds);
                freePipe(cmds); // free's malloc'd ptrs
            }
            else { // else execute cmd
                createChildProc(cmd, true);
            }
            // add cmd to history
            if (history != NULL)
            {
                free(history);
            }
            history = strdup(input);
            // free memory
            free(input);
            free(temp);
        }
    }
    // free 
    if (history != NULL)
    {
        free(history);
    }
    return 0;
}

