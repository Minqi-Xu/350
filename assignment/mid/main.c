/* main.c
 * ----------------------------------------------------------
 *  CS350
 *  Midterm Programming Assignment
 *
 *  Purpose:  - Use Linux programming environment.
 *            - Review process creation and management
 * ----------------------------------------------------------
 */
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
size_t lenn = 1;
int type = 0;   // 0-normal, 1-input redirection, 2-output redirection, 3-input&output redirection
char infile[1000], outfile[1000];
int childproc[1000000];
int child_counter = 0;
int retval;
int ret_get = 0;
char error_message[30] = "An error has occurred\n";
bool batchmode = false;

void identify(int counter, char **tokens) {
    for(int i = 0; i < counter; i++) {
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

int main( int argc, char ** argv )
{
    if (argc > 1) {
        batchmode = true;
        int f = open(argv[1], O_CREAT);
        if(f < 0) {
            write(STDOUT_FILENO, line, strlen(line));
            exit(0);
        }
        dup2(f, 0);
    }
    ret_get = getline(&line, &lenn, stdin);
    while(ret_get != -1) {
        bool needwait = true;
//        fprintf(stderr, "> ");
//        ret_get = getline(&line, &lenn, stdin);
/*        if(batchmode) {
            write(STDOUT_FILENO, line, strlen(line));
        } else {
            fprintf(stderr, "> ");
        }
*/
        int len = strlen(line);
        type = 0;
        memset(infile, 0, sizeof(infile));
        memset(outfile, 0, sizeof(outfile));
        char *token;
        char **tokens = malloc(len * sizeof(char*));
        token = strtok(line, "\n\t ");
        if(token == NULL) {
            free(tokens);
            ret_get = getline(&line, &lenn, stdin);
            continue;
        }
 /*       if(!strcmp(token, "exit")) {
            free(tokens);
            exit(0);
        }   */
        int counter = 0;

        while(token != NULL) {
            if(strlen(token) >= 2 && token[0] == '\'' && token[strlen(token) - 1] == '\'') {
                char* temp = token + 1;
                temp[strlen(temp) - 1] = '\0';
                tokens[counter] = temp;
            } else {
                tokens[counter] = token;
            }
            counter++;

            if(counter >= len) {
                len *= 2;
                tokens = realloc(tokens, len * sizeof(char*));
            }
            token = strtok(NULL, "\n\t ");

        }

        tokens[counter] = NULL;
        identify(counter, tokens);
        if(!strcmp(tokens[0], "")) {
            free(tokens);
            ret_get = getline(&line, &lenn, stdin);
            continue;
        }

        if(!strcmp(tokens[counter-1], "&")) {
            needwait = false;
            tokens[counter-1] = NULL;
            counter--;
        }


        if(!strcmp(tokens[0], "exit")) {
            if(counter > 1)
                write(STDERR_FILENO, error_message, strlen(error_message));
            free(tokens);
            exit(0);
        } else if(!strcmp(tokens[0], "wait")) {
            if(counter >1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(tokens);
                exit(0);
            }
            for(int i = 0; i < child_counter; i++) {
                waitpid(childproc[i], &retval, 0);
            }
            free(tokens);
            ret_get = getline(&line, &lenn, stdin);
            continue;
        } else if(!strcmp(tokens[0], "pwd")) {
            if(counter > 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(tokens);
                exit(0);
            }
            char path[1000];
            getcwd(path, 1000);
            write(STDOUT_FILENO, path, strlen(path));
            free(tokens);
            ret_get = getline(&line, &lenn, stdin);
            continue;
        } else if(!strcmp(tokens[0], "cd")) {
            if(counter > 2) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(tokens);
                exit(0);
            }
            chdir(tokens[1]);
            free(tokens);
            ret_get = getline(&line, &lenn, stdin);
            continue;
        } else if(!strcmp(tokens[0], "help")) {
            if(counter > 2) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(tokens);
                exit(0);
            }
            printf("cd\n");
            printf("pwd\n");
            printf("wait\n");
            printf("exit\n");
            printf("help\n");
            free(tokens);
            ret_get = getline(&line, &lenn, stdin);
            continue;
        }

        pid_t pid;
        pid = fork();

        if(pid < 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            //fprintf(stderr, "ERROR! pid negative\n");
            exit(-1);
        }
        if(pid == 0) {  // children
            if(type == 0) {
                if(execvp(tokens[0], tokens) == -1) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    //fprintf(stderr, "ERROR! execvp fail\n");
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
            childproc[child_counter] = pid;
            child_counter++;
            if(needwait)
                waitpid(pid, &retval, 0);
        }
        free(tokens);
        ret_get = getline(&line, &lenn, stdin);
    }

    return 0;
}
