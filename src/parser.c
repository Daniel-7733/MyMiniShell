#include <stdio.h>
#include <string.h>

#include "shell.h"


// ====================================
//     Add a helper for arguments
// ====================================
static int add_argument(Command *command, char *argument)
{
    if (command->argc >= MAX_ARGS - 1) {
        fprintf(stderr, "minishell: too many command arguments\n");
        return -1;
    }

    command->argv[command->argc] = argument;
    command->argc++;
    command->argv[command->argc] = NULL;

    return 0;
}


// ====================================
//     Add a redirection parser:
// ====================================
static int parse_redirection(const Token tokens[], int token_count, int *index, Command *command)
{
    TokenType type = tokens[*index].type;

    if (*index + 1 >= token_count || tokens[*index + 1].type != TOKEN_WORD) {
        fprintf(stderr, "minishell: missing redirection filename\n");
        return -1;
    }

    char *filename = tokens[*index + 1].text;

    if (type == TOKEN_INPUT) {
        if (command->input_file != NULL) {
            fprintf(stderr, "minishell: multiple input redirections\n");
            return -1;
        }

        command->input_file = filename;
    } else {
        if (command->output_file != NULL) {
            fprintf(stderr, "minishell: multiple output redirections\n");
            return -1;
        }

        command->output_file = filename;
        command->append_output = type == TOKEN_APPEND; // Output will be 1 (true) or 0 (false)
    }

    /*
     * Skip the filename. The for loop will then advance
     * beyond it.
     */
    (*index)++;

    return 0;
}


// ====================================
//          main function
// ====================================
int parse_tokens(const Token tokens[], int token_count, CommandLine *command_line)
{
    memset(command_line, 0, sizeof(*command_line));

    if (token_count == 0) {
        command_line->command_count = 0;
        return 0;
    }

    command_line->command_count = 1;

    Command *current = &command_line->commands[0];

    for (int i = 0; i < token_count; i++) {
        Token token = tokens[i];

        if (token.type == TOKEN_WORD) {

            if (add_argument(current, token.text) == -1) {
                return -1;
            }

            continue;
        }

        if (token.type == TOKEN_PIPE) {
            
            if (current->argc == 0) {
                fprintf(stderr, "minishell: missing command before '|'\n");
                return -1;
            }

            if (i + 1 >= token_count) {
                fprintf(stderr, "minishell: missing command after '|'\n");
                return -1;
            }

            if (command_line->command_count >= MAX_ARGS) {
                fprintf(stderr, "minishell: too many pipeline commands\n");
                return -1;
            }

            current = &command_line->commands[command_line->command_count];
            command_line->command_count++;
            continue;
        }

        if (parse_redirection(tokens, token_count, &i, current) == -1) {
            return -1;
        }
    }

    if (current->argc == 0) {
        fprintf(stderr, "minishell: missing command after '|'\n");
        return -1;
    }

    return 0;
}


