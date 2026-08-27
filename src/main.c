#include <stdio.h>
#include <string.h>

#include "shell.h"

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

        if (token_count == -1) {
            fprintf(stderr, "minishell: too many command tokens\n");
            continue;
        }

        if (token_count == 0) {
            continue;
        }

        int pipe_index = find_pipe(tokens);

        if (pipe_index != -1) {
            execute_pipeline(tokens);
            continue;
        }

        BuiltinResult result = execute_builtin(tokens, token_count);

        if (result == BUILTIN_EXIT) {
            break;
        }

        if (result == BUILTIN_NOT_FOUND) {
            execute_external(tokens);
        }
    }

    return 0;
}


