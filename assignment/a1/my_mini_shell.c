#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/fcntl.h>


char* line = NULL;
ssize_t len = 1;
int type = 0;   // 0-normal, 1-input redirection, 2-output redirection, 3-input&output redirection, 4 pipe
char infile[1000], outfile[1000];

void identify(int counter, char **tokens) {
    for(int i = 0; i < counter; i++) {
        if(!strcmp(tokens[i], "|")) {
            type = 4;
            return;
        }
        if(!strcmp(tokens[i], "<")) {
            strcpy(infile, tokens[i+1]);
            tokens[i] = NULL;
            if(type == 0)
                type = 1;
            else if(type == 2)
                type = 3;
        }
        if(!strcmp(tokens[i], ">")) {
            strcpy(outfile, tokens[i+1]);
            tokens[i] = NULL;
            if(type == 0)
                type = 2;
            else if(type == 1)
                type = 3;
        }
    }
}

int main() {
    while(true) {
        printf("$ ");
        getline(&line, &len, stdin);
        type = 0;
        memset(infile, 0, sizeof(infile));
        memset(outfile, 0, sizeof(outfile));
        char *token;
        char **tokens = malloc(len * sizeof(char*));
        token = strtok(line, "\n\t ");
        if(!strcmp(token, "exit")) {
            free(tokens);
            exit(0);
        }
        int counter = 0;

        while(token != NULL) {
            tokens[counter] = token;
            counter++;

            if(counter >= len) {
                len *= 2;
                tokens = realloc(tokens, len * sizeof(char*));
            }

            token = strtok(NULL, "\n\t ");

        }
        tokens[counter] = NULL;

        identify(counter, tokens);

        pid_t pid;
        int retval;
        pid = fork();
        if(pid < 0) {
            fprintf(stderr, "ERROR! pid negative\n");
            exit(-1);
        }
        if(pid == 0) {  // children
            if(type == 0) {
                if(execvp(tokens[0], tokens) == -1) {
                    fprintf(stderr, "ERROR! execvp fail\n");
                    return 1;
                }
            } else if (type == 1) {
                int f = open(infile, O_CREAT);
                dup2(f, 0);
                execvp(tokens[0], tokens);
                exit(0);
            } else if (type == 2) {
                int f = open(outfile, O_CREAT|O_TRUNC);
                dup2(f, 1);
                execvp(tokens[0], tokens);
                exit(0);
            }

        } else {
            //parent
            waitpid(pid, &retval, 0);
        }
        free(tokens);
    }

    return 0;
}
