
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 80

char*** convert(char** input) {

    // output
    char*** ret = (char***)malloc(sizeof(char***) * BUFSIZE);
    memset(ret, '\0', (sizeof(char***) * BUFSIZE));

    // hold individual cmd
    char** cmd = (char**)malloc(sizeof(cmd[0]) * BUFSIZE);
    memset(cmd, '\0', (sizeof(cmd[0]) * BUFSIZE));

    // parse
    int i = 0, j = 0, k = 0;
    while (input[i] != NULL) {
        if (strcmp(input[i], "|") == 0) {
            cmd[j] = '\0';
            // add string arr to return
            ret[k] = cmd; // cmd will leave function malloc'd
            cmd = (char**)malloc(sizeof(cmd[0]) * BUFSIZE);
            memset(cmd, '\0', BUFSIZE);
            ++k;
            j = 0;
        }
        else {
            // add token to string arr
            cmd[j] = strdup(input[i]); // cmd will leave function holding malloc
            ++j;
        }
        ++i;
    }
    
    // last cmd
    cmd[j] = NULL;
    ret[k] = cmd; // cmd will leave function malloc'd

    return ret;
}

int main() {

    char* input[] = { "cmd1", "arg1", "|", "cmd2", "arg2", "arg3", "|", "cmd3", NULL };
    char*** out = convert(input);

    // free cmd
    int i = 0, j = 0;
    while(out[i] != NULL) {
        while(out[i][j] != NULL) {
            free(out[i][j]);
            ++j;
        }
        free(out[i]);
        j = 0;
        ++i;
    }
    free(out);

    return 0;
}