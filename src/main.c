#include <stdio.h>
#include <string.h>

#include "shell.h"

int main(void)
{
    char input[INPUT_SIZE];
    char token_storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    while (1) {
        printf("minishell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            putchar('\n');
            break;
        }

        input[strcspn(input, "\n")] = '\0';

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


