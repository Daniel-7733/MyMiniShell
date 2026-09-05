#include <stdio.h>
#include <errno.h>

#include "input.h"
#include "shell.h"
#include "signals.h"

int main(void)
{
    install_shell_signal_handlers();

    char input[INPUT_SIZE];
    char token_storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    while (1) {
        printf("minishell> ");
        fflush(stdout);

        InputResult input_result = read_command_line(stdin, input, sizeof(input));

        /*
         * Ctrl+D means end-of-input.
         */
        if (input_result == INPUT_END) {
            putchar('\n');
            break;
        }

        /*
         * Ctrl+C may interrupt the input operation.
         */
        if (input_result == INPUT_ERROR) {
            if (errno == EINTR) {
                clearerr(stdin);
                continue;
            }

            perror("minishell: input");
            break;
        }

        if (input_result == INPUT_TOO_LONG) {
            fprintf(stderr, "minishell: command line is too long\n");
            continue;
        }

        int token_count = lex(input, token_storage, sizeof(token_storage), tokens);

        if (token_count == -1) {
            fprintf(stderr, "minishell: invalid command syntax\n");
            continue;
        }

        if (token_count == 0) {
            continue;
        }

        CommandLine command_line;

        if (parse_tokens(tokens, token_count, &command_line) == -1) {
            continue;
        }

        if (command_line.command_count > 1) {
            execute_pipeline(&command_line);
            continue;
        }

        Command *command = &command_line.commands[0];

        BuiltinResult result = execute_builtin(command->argv, command->argc);

        if (result == BUILTIN_EXIT) {
            break;
        }

        if (result == BUILTIN_NOT_FOUND) {
            execute_command(command);
        }
    }

    return 0;
}


