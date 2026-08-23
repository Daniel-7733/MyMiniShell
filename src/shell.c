#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

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


static int setup_output_redirection(char *tokens[]);
static int find_output_redirection(char *tokens[]);
static int find_input_redirection(char *tokens[]);
static int setup_input_redirection(char *tokens[]);


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
int execute_external(char *tokens[])
{
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("minishell: fork");
        return -1;
    }

    if (child_pid == 0) {
        if (setup_input_redirection(tokens) == -1) {
            _exit(1);
        }

        if (setup_output_redirection(tokens) == -1) {
            _exit(1);
        }

        execvp(tokens[0], tokens);

        perror(tokens[0]);
        _exit(127);
    }

    int status; // This veriable contains encoded information about how the child ended.

    while (waitpid(child_pid, &status, 0) == -1) {
        if (errno == EINTR) {
            continue;
        }

        perror("minishell: waitpid");
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return -1;
}

static int setup_output_redirection(char *tokens[]) // This function only use in shell.c
{
    int operator_index = find_output_redirection(tokens);

    if (operator_index == -1) {
        return 0;
    }

    if (operator_index == 0) {
        fprintf(
            stderr,
            "minishell: missing command before '%s'\n",
            tokens[operator_index]
        );
        return -1;
    }

    if (tokens[operator_index + 1] == NULL) {
        fprintf(
            stderr,
            "minishell: missing filename after '%s'\n",
            tokens[operator_index]
        );
        return -1;
    }

    if (tokens[operator_index + 2] != NULL) {
        fprintf(
            stderr,
            "minishell: only one output file is supported\n"
        );
        return -1;
    }

    // --------------Part 2 -------------- //
    char *operator = tokens[operator_index];
    char *filename = tokens[operator_index + 1];

    // ------- Concept 1 -------
    // O_WRONLY   open for writing
    // O_CREAT    create the file if it does not exist
    // O_TRUNC    empty the existing file before writing
    int flags = O_WRONLY | O_CREAT;

    if (strcmp(operator, ">") == 0) {
        flags |= O_TRUNC;
    } else {
        flags |= O_APPEND;
    }


    // ------- Concept 2 ------- 
    // owner: read + write
    // group: read
    // others: read
    int output_fd = open(filename, flags, 0644);

    if (output_fd == -1) {
        perror("minishell: open");
        return -1;
    }

    if (dup2(output_fd, STDOUT_FILENO) == -1) {
        perror("minishell: dup2");
        close(output_fd);
        return -1;
    }

    if (close(output_fd) == -1) {
        perror("minishell: close");
        return -1;
    }

    tokens[operator_index] = NULL;

    return 0;
}

/*
 * let say input is echo hello > output.txt
 * function return 2 because sign is on 2nd index.
 * However, if the out put is echo hello
 * then function return -1 because the input doesn't have the sign
 *
 * -1 = operator not found
 *  0+ = index where operator was found
 */
static int find_output_redirection(char *tokens[])
{
    for (int i = 0; tokens[i] != NULL; i++) {
        if (
            strcmp(tokens[i], ">") == 0 ||
            strcmp(tokens[i], ">>") == 0
        ) {
            return i;
        }
    }

    return -1;
}

// ---------------- Input part ---------------- 
static int find_input_redirection(char *tokens[])
{
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            return i;
        }
    }

    return -1;
}

static int setup_input_redirection(char *tokens[])
{
    int operator_index = find_input_redirection(tokens);

    if (operator_index == -1) {
        return 0;
    }

    if (operator_index == 0) {
        fprintf(stderr, "minishell: missing command before '<'\n");
        return -1;
    }

    if (tokens[operator_index + 1] == NULL) {
        fprintf(stderr, "minishell: missing filename after '<'\n");
        return -1;
    }

    if (tokens[operator_index + 2] != NULL) {
        fprintf(
            stderr,
            "minishell: only one input file is supported\n"
        );
        return -1;
    }

    char *filename = tokens[operator_index + 1];

    int input_fd = open(filename, O_RDONLY);

    if (input_fd == -1) {
        perror("minishell: open");
        return -1;
    }

    if (dup2(input_fd, STDIN_FILENO) == -1) {
        perror("minishell: dup2");
        close(input_fd);
        return -1;
    }

    if (close(input_fd) == -1) {
        perror("minishell: close");
        return -1;
    }

    tokens[operator_index] = NULL;

    return 0;
}
