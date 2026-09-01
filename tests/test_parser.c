#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "shell.h"

static int lex_and_parse(
    const char *input,
    char storage[],
    Token tokens[],
    CommandLine *command_line
)
{
    int token_count = lex(
        input,
        storage,
        INPUT_SIZE,
        tokens
    );

    if (token_count == -1) {
        return -1;
    }

    return parse_tokens(
        tokens,
        token_count,
        command_line
    );
}

static void test_quoted_operators_are_arguments(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];
    CommandLine line;

    assert(
        lex_and_parse(
            "echo \"|\" \">\"",
            storage,
            tokens,
            &line
        ) == 0
    );

    assert(line.command_count == 1);

    Command *command = &line.commands[0];

    assert(command->argc == 3);
    assert(strcmp(command->argv[0], "echo") == 0);
    assert(strcmp(command->argv[1], "|") == 0);
    assert(strcmp(command->argv[2], ">") == 0);
    assert(command->argv[3] == NULL);
    assert(command->output_file == NULL);
}

static void test_pipeline_and_redirections(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];
    CommandLine line;

    assert(
        lex_and_parse(
            "cat<input.txt|sort>>output.txt",
            storage,
            tokens,
            &line
        ) == 0
    );

    assert(line.command_count == 2);

    assert(line.commands[0].argc == 1);
    assert(strcmp(
        line.commands[0].argv[0],
        "cat"
    ) == 0);
    assert(strcmp(
        line.commands[0].input_file,
        "input.txt"
    ) == 0);

    assert(line.commands[1].argc == 1);
    assert(strcmp(
        line.commands[1].argv[0],
        "sort"
    ) == 0);
    assert(strcmp(
        line.commands[1].output_file,
        "output.txt"
    ) == 0);
    assert(line.commands[1].append_output == 1);
}

static void test_quoted_filename(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];
    CommandLine line;

    assert(
        lex_and_parse(
            "echo hello>\"output file.txt\"",
            storage,
            tokens,
            &line
        ) == 0
    );

    assert(line.command_count == 1);
    assert(strcmp(
        line.commands[0].output_file,
        "output file.txt"
    ) == 0);
}

static void test_invalid_pipeline(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];
    CommandLine line;

    assert(
        lex_and_parse(
            "echo hello||wc",
            storage,
            tokens,
            &line
        ) == -1
    );

    assert(
        lex_and_parse(
            "|wc",
            storage,
            tokens,
            &line
        ) == -1
    );
}

static void test_invalid_redirection(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];
    CommandLine line;

    assert(
        lex_and_parse(
            "echo hello>",
            storage,
            tokens,
            &line
        ) == -1
    );

    assert(
        lex_and_parse(
            "cat<input-one<input-two",
            storage,
            tokens,
            &line
        ) == -1
    );
}

int main(void)
{
    test_quoted_operators_are_arguments();
    test_pipeline_and_redirections();
    test_quoted_filename();
    test_invalid_pipeline();
    test_invalid_redirection();

    printf("All parser tests passed.\n");
    return 0;
}
