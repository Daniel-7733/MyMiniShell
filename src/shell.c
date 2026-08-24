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


// =============================================
//           private declarations  
// =============================================
static int is_redirection_operator(const char *token);
static int parse_redirections(char *tokens[], Redirection *redirection);
static int setup_redirections(const Redirection *redirection);


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
        Redirection redirection;

        if (parse_redirections(tokens, &redirection) == -1) {
            _exit(2);
        }

        if (setup_redirections(&redirection) == -1) {
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


// =============================================
//           Recognize operators  
// =============================================
static int is_redirection_operator(const char *token)
{
    return strcmp(token, "<") == 0 ||
           strcmp(token, ">") == 0 ||
           strcmp(token, ">>") == 0;
}

static int parse_redirections(char *tokens[], Redirection *redirection)
{
    redirection->input_file = NULL;
    redirection->output_file = NULL;
    redirection->append = 0;

    int read_index = 0;
    int write_index = 0;

    while (tokens[read_index] != NULL) {
        char *token = tokens[read_index];

        if (!is_redirection_operator(token)) {
            tokens[write_index] = token;
            write_index++;
            read_index++;
            continue;
        }

        char *filename = tokens[read_index + 1];

        if (filename == NULL || is_redirection_operator(filename)) {
            fprintf(stderr, "minishell: missing filename after '%s'\n", token);
            return -1;
        }

        if (strcmp(token, "<") == 0) {
            if (redirection->input_file != NULL) {
                fprintf(stderr, "minishell: multiple input redirections\n");
                return -1;
            }

            redirection->input_file = filename;
        } else {
            if (redirection->output_file != NULL) {
                fprintf(stderr, "minishell: multiple output redirections\n");
                return -1;
            }

            redirection->output_file = filename;
            redirection->append = strcmp(token, ">>") == 0;
        }

        read_index += 2;
    }

    tokens[write_index] = NULL;

    if (write_index == 0) {
        fprintf(stderr, "minishell: missing command\n");
        return -1;
    }

    return 0;
}


static int setup_redirections(const Redirection *redirection)
{
    if (redirection->input_file != NULL) {
        int input_fd = open(
            redirection->input_file,
            O_RDONLY
        );

        if (input_fd == -1) {
            perror("minishell: open input");
            return -1;
        }

        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("minishell: dup2 input");
            close(input_fd);
            return -1;
        }

        if (close(input_fd) == -1) {
            perror("minishell: close input");
            return -1;
        }
    }

    if (redirection->output_file != NULL) {
        int flags = O_WRONLY | O_CREAT;

        if (redirection->append) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }

        int output_fd = open(
            redirection->output_file,
            flags,
            0644
        );

        if (output_fd == -1) {
            perror("minishell: open output");
            return -1;
        }

        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("minishell: dup2 output");
            close(output_fd);
            return -1;
        }

        if (close(output_fd) == -1) {
            perror("minishell: close output");
            return -1;
        }
    }

    return 0;
}
