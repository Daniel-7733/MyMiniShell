#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shell.h"

// =======================
//   mental model
// =======================
//
//
// minishell process
//        |
//        | fork()
//        |
//    ┌───┴────────┐
//    │            │
//  parent        child
//    │            │
//  waitpid()    execvp("ls", tokens)
//    │            │
//    │           ls program
//    │            │
//    └──── waits ─┘
//           |
//     show next prompt
//
// ---------------------------



int tokenize(char *input, char *tokens[])
{
    int token_count = 0;
    char *token = strtok(input, " \t");

    while (token != NULL && token_count < MAX_ARGS - 1) {
        tokens[token_count] = token;
        token_count++;

        token = strtok(NULL, " \t");
    }

    tokens[token_count] = NULL;

    return token_count;
}

void print_help(void)
{
    printf("MyMiniShell built-in commands:\n");
    printf("  help    Show this help message\n");
    printf("  exit    Exit the shell\n");
    printf("  pwd     Print the current working directory\n");
    printf("  cd      Change the current working directory\n");
}

BuiltinResult execute_builtin(char *tokens[], int token_count)
{
    if (strcmp(tokens[0], "exit") == 0) {
        if (token_count == 1) {
            return BUILTIN_EXIT;
        }

        printf("minishell: exit: too many arguments\n");
        return BUILTIN_HANDLED;
    }

    if (strcmp(tokens[0], "help") == 0) {
        if (token_count == 1) {
            print_help();
        } else {
            printf("minishell: help: too many arguments\n");
        }

        return BUILTIN_HANDLED;
    }

    if (strcmp(tokens[0], "pwd") == 0) {
        if (token_count == 1) {
            print_working_directory();
        } else {
            printf("minishell: pwd: too many arguments\n");
        }

        return BUILTIN_HANDLED;
    }

    if (strcmp(tokens[0], "cd") == 0) {
        change_directory(tokens, token_count);
        return BUILTIN_HANDLED;
    }

    return BUILTIN_NOT_FOUND;
}

void print_working_directory(void)
{
    char cwd[INPUT_SIZE];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("minishell: pwd");
        return;
    }

    printf("%s\n", cwd);
}


void change_directory(char *tokens[], int token_count)
{
    char *destination;

    if (token_count > 2) {
        printf("minishell: cd: too many arguments\n");
        return;
    }

    if (token_count == 1) {
        destination = getenv("HOME");

        if (destination == NULL) {
            printf("minishell: cd: HOME is not set\n");
            return;
        }

    } else {
        destination = tokens[1];
    }

    if (chdir(destination) == -1) {
        perror("minishell: cd");
    }
}

/*
 * fork() < 0    failure
 * fork() == 0   child process
 * fork() > 0    parent process; value is child’s process ID
*/
void execute_external(char *tokens[])
{
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("minishell: fork");
        return;
    }

    if (child_pid == 0) {
        execvp(tokens[0], tokens);

        perror(tokens[0]);
        _exit(127);
    }

    if (waitpid(child_pid, NULL, 0) == -1) {
        perror("minishell: waitpid");
    }
}

