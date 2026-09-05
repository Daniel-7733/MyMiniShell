#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shell.h"
#include "signals.h"

static int wait_for_child(pid_t child_pid);
static int setup_command_redirections(const Command *command);

static int wait_for_child(pid_t child_pid)
{
    int status;
    pid_t result;

    do {
        result = waitpid(child_pid, &status, 0);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
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

static int setup_command_redirections(const Command *command)
{
    if (command->input_file != NULL) {
        int input_fd = open(command->input_file, O_RDONLY);

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

    if (command->output_file != NULL) {
        int flags = O_WRONLY | O_CREAT;

        if (command->append_output) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }

        int output_fd = open(command->output_file, flags, 0644);

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

int execute_command(const Command *command)
{
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("minishell: fork");
        return -1;
    }

    if (child_pid == 0) {
        restore_child_signal_handlers();

        if (setup_command_redirections(command) == -1) {
            _exit(1);
        }

        execvp(command->argv[0], command->argv);
        perror(command->argv[0]);
        _exit(127);
    }

    return wait_for_child(child_pid);
}


int execute_pipeline(const CommandLine *command_line)
{
    pid_t child_pids[MAX_ARGS];
    int previous_read_fd = -1;
    int children_created = 0;

    /*
     * First loop:
     * create pipes and fork every command.
     */
    for (
        int command_index = 0;
        command_index < command_line->command_count;
        command_index++
    ) {
        const Command *command = &command_line->commands[command_index];

        int has_next_command =
            command_index < command_line->command_count - 1;

        int pipe_fds[2] = {-1, -1};

        if (has_next_command) {
            if (pipe(pipe_fds) == -1) {
                perror("minishell: pipe");

                if (previous_read_fd != -1) {
                    close(previous_read_fd);
                }

                for (int i = 0; i < children_created; i++) {
                    wait_for_child(child_pids[i]);
                }

                return 1;
            }
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

            for (int i = 0; i < children_created; i++) {
                wait_for_child(child_pids[i]);
            }

            return 1;
        }

        if (child_pid == 0) {
            restore_child_signal_handlers();

            /*
             * Read from the preceding pipe.
             */
            if (previous_read_fd != -1) {
                if (dup2(previous_read_fd, STDIN_FILENO) == -1) {
                    perror("minishell: dup2 input");
                    _exit(1);
                }
            }

            /*
             * Write into the following pipe.
             */
            if (has_next_command) {
                if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
                    perror("minishell: dup2 output");
                    _exit(1);
                }
            }

            /*
             * Explicit redirection is applied after pipe
             * wiring, so it can override the pipe endpoint.
             */
            if (setup_command_redirections(command) == -1) {
                _exit(1);
            }

            if (previous_read_fd != -1) {
                close(previous_read_fd);
            }

            if (has_next_command) {
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }

            execvp(command->argv[0], command->argv);

            perror(command->argv[0]);
            _exit(127);
        }

        child_pids[children_created] = child_pid;
        children_created++;

        /*
         * The parent no longer needs the preceding pipe.
         */
        if (previous_read_fd != -1) {
            close(previous_read_fd);
        }

        /*
         * The parent keeps only the read end needed by
         * the next command.
         */
        if (has_next_command) {
            close(pipe_fds[1]);
            previous_read_fd = pipe_fds[0];
        } else {
            previous_read_fd = -1;
        }
    }

    /*
     * The parent should no longer keep this pipe open.
     */
    if (previous_read_fd != -1) {
        close(previous_read_fd);
    }

    /*
     * Second loop:
     * wait for every child.
     */
    int final_status = 0;

    for (int i = 0; i < children_created; i++) {
        int child_status = wait_for_child(child_pids[i]);

        /*
         * Pipeline status is the final command's status.
         */
        if (i == children_created - 1) {
            final_status = child_status;
        }
    }

    return final_status;
}

