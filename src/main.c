#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define INPUT_SIZE 1024
#define MAX_ARGS 64

int tokenize(char *input, char *tokens[]);
void print_help(void);
int execute_builtin(char *tokens[], int token_count);
void print_working_directory(void);

int main(void)
{
    char input[INPUT_SIZE];
    char *tokens[MAX_ARGS];

    while (1) {
        printf("minishell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            putchar('\n');
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        int token_count = tokenize(input, tokens);

        if (token_count == 0) {
            continue;
        }

        int should_exit = execute_builtin(tokens, token_count);

        if (should_exit) {
            break;
        }
    }

    return 0;
}

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
}

int execute_builtin(char *tokens[], int token_count)
{
    if (strcmp(tokens[0], "exit") == 0) {
        if (token_count == 1) {
            return 1;
        }

        printf("minishell: exit: too many arguments\n");
        return 0;
    }

    if (strcmp(tokens[0], "help") == 0) {
        if (token_count == 1) {
            print_help();
        } else {
            printf("minishell: help: too many arguments\n");
        }

        return 0;
    }

    if (strcmp(tokens[0], "pwd") == 0) {
        if (token_count == 1) {
            print_working_directory();
        } else {
            printf("minishell: pwd: too many arguments\n");
        }

        return 0;
    }

    printf("minishell: unknown command: %s\n", tokens[0]);
    return 0;
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

