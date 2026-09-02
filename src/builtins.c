#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"

void print_help(void)
{
    printf("MyMiniShell built-in commands:\n");
    printf("  help    Show this help message\n");
    printf("  exit    Exit the shell\n");
    printf("  pwd     Print the current working directory\n");
    printf("  cd      Change the current working directory\n");
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

void change_directory(char *arguments[], int argument_count)
{
    char *destination;

    if (argument_count > 2) {
        fprintf(stderr, "minishell: cd: too many arguments\n");
        return;
    }

    if (argument_count == 1) {
        destination = getenv("HOME");

        if (destination == NULL) {
            fprintf(stderr, "minishell: cd: HOME is not set\n");
            return;
        }
    } else {
        destination = arguments[1];
    }

    if (chdir(destination) == -1) {
        perror("minishell: cd");
    }
}

BuiltinResult execute_builtin(char *arguments[], int argument_count)
{
    if (strcmp(arguments[0], "exit") == 0) {
        if (argument_count == 1) {
            return BUILTIN_EXIT;
        }

        fprintf(
            stderr,
            "minishell: exit: too many arguments\n"
        );
        return BUILTIN_HANDLED;
    }

    if (strcmp(arguments[0], "help") == 0) {
        if (argument_count == 1) {
            print_help();
        } else {
            fprintf(stderr, "minishell: help: too many arguments\n");
        }

        return BUILTIN_HANDLED;
    }

    if (strcmp(arguments[0], "pwd") == 0) {
        if (argument_count == 1) {
            print_working_directory();
        } else {
            fprintf(stderr, "minishell: pwd: too many arguments\n");
        }

        return BUILTIN_HANDLED;
    }

    if (strcmp(arguments[0], "cd") == 0) {
        change_directory(arguments, argument_count);
        return BUILTIN_HANDLED;
    }

    return BUILTIN_NOT_FOUND;
}

