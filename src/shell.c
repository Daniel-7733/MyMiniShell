#include <asm-generic/errno-base.h>
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
static int wait_for_child(pid_t child_pid);


/**
 * Splits the input string into individual tokens.
 *
 * Tokens are separated by spaces and tabs and stored in the
 * provided tokens array. The array is always NULL-terminated.
 *
 * @param input  The input command string to tokenize.
 * @param tokens Array where the resulting tokens are stored.
 * @return Number of tokens found.
 */
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


/**
 * Prints the list of built-in commands supported by the shell.
 *
 * This function displays the available built-in commands and
 * a short description of what each command does.
 */
void print_help(void)
{
    printf("MyMiniShell built-in commands:\n");
    printf("  help    Show this help message\n");
    printf("  exit    Exit the shell\n");
    printf("  pwd     Print the current working directory\n");
    printf("  cd      Change the current working directory\n");
}


/**
 * Checks whether a command is a shell built-in and executes it.
 *
 * Supported built-ins are:
 * - exit
 * - help
 * - pwd
 * - cd
 *
 * @param tokens     Array of command tokens.
 * @param token_count Number of tokens in the command.
 * @return BUILTIN_EXIT if the shell should exit,
 *         BUILTIN_HANDLED if a built-in was executed,
 *         BUILTIN_NOT_FOUND if the command is not a built-in.
 */
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


/**
 * Prints the current working directory.
 *
 * Uses getcwd() to obtain the current directory and prints it
 * to standard output. An error message is printed if the
 * current directory cannot be obtained.
 */
void print_working_directory(void)
{
    char cwd[INPUT_SIZE];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("minishell: pwd");
        return;
    }

    printf("%s\n", cwd);
}


/**
 * Changes the shell's current working directory.
 *
 * If no directory is provided, the HOME environment variable
 * is used as the destination. If a directory is provided,
 * that directory is used instead.
 *
 * @param tokens      Array containing the command and arguments.
 * @param token_count Number of tokens in the command.
 */
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


/**
 * Executes an external command in a child process.
 *
 * The function creates a child using fork(). The child handles
 * redirections and executes the command using execvp(), while
 * the parent waits for the child to finish.
 *
 * fork() < 0    failure
 * fork() == 0   child process
 * fork() > 0    parent process; value is child’s process ID
 *
 * @param tokens Array containing the command and its arguments.
 * @return The exit status of the child, or -1 if fork() fails.
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

    return wait_for_child(child_pid);
}



// =============================================
//           Recognize operators  
// =============================================



/**
 * Checks whether a token is a supported redirection operator.
 *
 * Supported operators are:
 * - <
 * - >
 * - >>
 *
 * @param token Token to check.
 * @return Non-zero if the token is a redirection operator,
 *         otherwise 0.
 */
static int is_redirection_operator(const char *token)
{
    return strcmp(token, "<") == 0 ||
           strcmp(token, ">") == 0 ||
           strcmp(token, ">>") == 0;
}


/**
 * Parses redirection operators from a command.
 *
 * Redirection operators and their filenames are removed from
 * the token array. The corresponding input/output filenames
 * are stored in the Redirection structure.
 *
 * @param tokens      Array containing command tokens.
 * @param redirection Structure used to store parsed redirections.
 * @return 0 on success, -1 if the redirection syntax is invalid.
 */
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


/**
 * Sets up input and output redirections for the current process.
 *
 * Input redirection uses STDIN_FILENO, while output redirection
 * uses STDOUT_FILENO. The >> operator appends to the output file,
 * while > truncates the file before writing.
 *
 * @param redirection Redirection information to apply.
 * @return 0 on success, -1 if opening, duplicating, or closing
 *         a file descriptor fails.
 */
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



// =============================================
//               The pipline
//
//       ls stdout ──► pipe ──► wc stdin 
// =============================================



/**
 * Searches for a pipe operator in the token array.
 *
 * @param tokens Array containing command tokens.
 * @return Index of the first "|" token, or -1 if no pipe exists.
 */
int find_pipe(char *tokens[])
{
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            return i;
        }
    }

    return -1;
}


/**
 * Splits a token array into separate commands for a pipeline.
 *
 * Each "|" token is replaced with NULL so that every command
 * becomes a separate NULL-terminated token array.
 *
 * @param tokens   Original token array containing the pipeline.
 * @param commands Array where pointers to individual commands
 *                 are stored.
 * @return Number of commands on success, or -1 if the pipeline
 *         syntax is invalid.
 */
static int split_pipeline(char *tokens[], char **commands[])
{
    int command_count = 1;

    commands[0] = tokens;

    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "|") != 0) {
            continue;
        }

        if (i == 0) {
            fprintf(stderr, "minishell: missing command before '|'\n");
            return -1;
        }

        if (tokens[i + 1] == NULL || strcmp(tokens[i + 1], "|") == 0) {
            fprintf(stderr, "minishell: missing command after '|'\n");
            return -1;
        }

        tokens[i] = NULL;
        commands[command_count] = &tokens[i + 1];
        command_count++;
    }

    return command_count;
}


/**
 * Executes a pipeline of external commands.
 *
 * Creates pipes and child processes so that the standard output
 * of each command is connected to the standard input of the next
 * command. Redirections are also applied to each command.
 *
 * @param tokens Array containing the complete pipeline.
 * @return Exit status of the last command in the pipeline, or
 *         an error status if pipeline setup fails.
 */
int execute_pipeline(char *tokens[])
{
    char **commands[MAX_ARGS];
    pid_t child_pids[MAX_ARGS];

    int command_count = split_pipeline(tokens, commands);

    if (command_count == -1) {
        return 2;
    }

    int previous_read_fd = -1;
    int children_created = 0;

    for (int i = 0; i < command_count; i++) {
        int pipe_fds[2] = {-1, -1};
        int has_next_command = i < command_count - 1;

        if (has_next_command && pipe(pipe_fds) == -1) {
            perror("minishell: pipe");

            if (previous_read_fd != -1) {
                close(previous_read_fd);
            }

            for (int j = 0; j < children_created; j++) {
                wait_for_child(child_pids[j]);
            }

            return 1;
        }

        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("minishell: fork");

            if (previous_read_fd != -1) {
                close(previous_read_fd);
            }

            if (has_next_command) {
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }

            for (int j = 0; j < children_created; j++) {
                wait_for_child(child_pids[j]);
            }

            return 1;
        }

        if (child_pid == 0) {
            if (previous_read_fd != -1) {
                if (
                    dup2(
                        previous_read_fd,
                        STDIN_FILENO
                    ) == -1
                ) {
                    perror("minishell: dup2 input");
                    _exit(1);
                }
            }

            if (has_next_command) {
                if (
                    dup2(
                        pipe_fds[1],
                        STDOUT_FILENO
                    ) == -1
                ) {
                    perror("minishell: dup2 output");
                    _exit(1);
                }
            }

            Redirection redirection;

            if (parse_redirections(commands[i], &redirection) == -1) {
                _exit(2);
            }

            if (setup_redirections(&redirection) == -1) {
                _exit(1);
            }

            if (previous_read_fd != -1) {
                close(previous_read_fd);
            }

            if (has_next_command) {
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }

            execvp(commands[i][0], commands[i]);

            perror(commands[i][0]);
            _exit(127);
        }

        child_pids[children_created] = child_pid;
        children_created++;

        if (previous_read_fd != -1) {
            close(previous_read_fd);
        }

        if (has_next_command) {
            close(pipe_fds[1]);
            previous_read_fd = pipe_fds[0];
        } else {
            previous_read_fd = -1;
        }
    }

    int pipeline_result = 0;

    for (int i = 0; i < children_created; i++) {
        int child_result =
            wait_for_child(child_pids[i]);

        if (i == children_created - 1) {
            if (child_result == -1) {
                pipeline_result = 1;
            } else {
                pipeline_result = child_result;
            }
        }
    }

    return pipeline_result;
}



// =============================================
//             Helper function 
// =============================================



/**
 * Waits for a child process to finish and converts its status
 * into a shell-compatible exit status.
 *
 * If the child exits normally, its exit status is returned.
 * If the child is terminated by a signal, 128 plus the signal
 * number is returned.
 *
 * @param child_pid Process ID of the child to wait for.
 * @return Child exit status, or 1 if waitpid() fails or the
 *         child status cannot be interpreted.
 */
static int wait_for_child(pid_t child_pid)
{
    int status; // This veriable contains encoded information about how the child ended.

    while (waitpid(child_pid, &status, 0) == -1) {
        if (errno == EINTR) {
            continue;
        }

        perror("minishell: waitpid");
        return 1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return 1;
}


