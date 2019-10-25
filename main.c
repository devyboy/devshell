#include "main.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>

#define MAXLINE 128

int main(int argc, char **argv, char **envp) {

    char *prevDir = malloc(MAXLINE);
    char *currDir = malloc(MAXLINE);
    getwd(currDir);
    getwd(prevDir);
    int status;

    while (1) {
        char *input = malloc(MAXLINE);

        printf("[%s]>", currDir); // prompt
        fgets(input, MAXLINE, stdin); // get user input
        char **cmd = parseInput(input); // parse user input

        if (strcmp(cmd[0], "exit") == 0) { // compare with built in commands
            exit(0);
        } else if (strcmp(cmd[0], "pwd") == 0) {
            printf("Current directory: %s\n", currDir);
        } else if (strcmp(cmd[0], "cd") == 0) {
            if (!cmd[1]) {
                chdir("~");
            }
            if (strcmp(cmd[1], "-") == 0) { // go to the last directory
                chdir(prevDir);
            } else {
                getwd(prevDir); // When changing directory, store the last one in prevDir
                chdir(cmd[1]);
            }
            getwd(currDir); // set the current directory again
        }
        else if (strcmp(cmd[0], "pid") == 0) {
            printf("Shell PID: %d\n", getpid());
        }
        else if (strcmp(cmd[0], "list") == 0) {
            DIR *d;
            struct dirent *dir;
            if (!cmd[1]) {
                d = opendir(".");
            }
            else {
                d = opendir(cmd[1]);
            }
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                    printf("%s\n", dir->d_name);
                }
                closedir(d);
            }
        }
        else if (strcmp(cmd[0], "kill") == 0) {
            printf("Executing built-in %s\n", cmd[0]);
            const char first = cmd[1][0]; // put the first character of the second arg in a variable
            if (strcmp(&first, "-") == 0) { // if the first character of the second arg is a dash
                kill((pid_t) cmd[2], (int) cmd[1]++); // kill the process and also remove the dash from the signal
            }
            else {
                kill((pid_t) cmd[1], SIGTERM);
            }
        }
        else if (strcmp(cmd[0], "printenv") == 0) {
            printf("Executing built-in %s\n", cmd[0]);
            if (cmd[2]) {
                fprintf(stderr, "%s: too many arguments", cmd[0]);
            }
            if (!cmd[1]) {
                for (char **env = envp; *env != 0; env++) {
                    char *thisEnv = *env;
                    printf("%s\n", thisEnv);
                }
            }
            else {
                char *yeet = getenv(cmd[1]);
                printf("%s", yeet);
            }
        }
        else if (strcmp(cmd[0], "setenv") == 0) {
            printf("Executing built-in %s\n", cmd[0]);
            if (cmd[3]) {
                fprintf(stderr, "%s: too many arguments", cmd[0]);
            }
            if (!cmd[1]) {
                for (char **env = envp; *env != 0; env++) {
                    char *thisEnv = *env;
                    printf("%s\n", thisEnv);
                }
            }
            else {
                setenv(cmd[1], cmd[2] , 1);
            }
        }
//        else if (strcmp((const char *) cmd[0][0], "/") == 0) {  crashes for some reason
//            if (access(cmd[0], X_OK) == 0) {
//                execv(cmd[0], cmd);
//            }
//        }
        else {
            pid_t cpid = fork(); // if not built in, fork process

            if (cpid < 0) {
                perror("Problem calling fork");
                exit(1);
            }
            if (cpid == 0) {
                if(strcmp(cmd[0], "where") == 0) {
                    if (execvp("whereis", cmd) < 0) { // if child, exec the command with its arguments
                        perror(cmd[0]); // also check if it fails
                        exit(1);
                    }
                }
                if (execvp(cmd[0], cmd) < 0) { // if child, exec the command with its arguments
                    perror(cmd[0]); // also check if it fails
                    exit(1);
                }
            } else {
                waitpid(cpid, &status, WNOHANG); // if parent, wait for child to finish
                printf("Return status: %d:", status);
            }
            free(input); //free allocated memory
            free(cmd);
        }
    }
}

char **parseInput(char *input) { // function to parse user input and return an array of args
    char **cmd = malloc(MAXLINE);
    char *curr;
    int i = 0;

    if (cmd == NULL) {
        perror("Problem with malloc");
        exit(1);
    }
    if (input[strlen(input) - 1] == '\n') {
        input[strlen(input) - 1] = 0; /* replace newline with null */
    }
    curr = strtok(input, " ");
    while (curr != NULL) { // loop through delimited strings and add them to args array
        cmd[i] = curr;
        i++;
        curr = strtok(NULL, " ");
    }
    cmd[i] = NULL; // make the last arg null
    return cmd;
}