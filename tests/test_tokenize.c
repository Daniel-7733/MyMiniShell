#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "shell.h"

static void test_single_command(void)
{
    char input[] = "pwd";
    char *tokens[MAX_ARGS];

    int count = tokenize(input, tokens);

    assert(count == 1);
    assert(strcmp(tokens[0], "pwd") == 0);
    assert(tokens[1] == NULL);
}

static void test_command_with_arguments(void)
{
    char input[] = "cd /home/ghost";
    char *tokens[MAX_ARGS];

    int count = tokenize(input, tokens);

    assert(count == 2);
    assert(strcmp(tokens[0], "cd") == 0);
    assert(strcmp(tokens[1], "/home/ghost") == 0);
    assert(tokens[2] == NULL);
}

static void test_extra_whitespace(void)
{
    char input[] = "   help    ";
    char *tokens[MAX_ARGS];

    int count = tokenize(input, tokens);

    assert(count == 1);
    assert(strcmp(tokens[0], "help") == 0);
}

static void test_empty_input(void)
{
    char input[] = "";
    char *tokens[MAX_ARGS];

    int count = tokenize(input, tokens);

    assert(count == 0);
    assert(tokens[0] == NULL);
}

int main(void)
{
    test_single_command();
    test_command_with_arguments();
    test_extra_whitespace();
    test_empty_input();

    printf("All tokenizer tests passed.\n");
    return 0;
}
