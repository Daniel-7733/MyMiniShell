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

static void test_operator_without_spaces(void)
{
    char input[] = "echo hello>output.txt";
    char *tokens[MAX_ARGS];

    int count = tokenize(input, tokens);

    assert(count == 4);
    assert(strcmp(tokens[0], "echo") == 0);
    assert(strcmp(tokens[1], "hello") == 0);
    assert(strcmp(tokens[2], ">") == 0);
    assert(strcmp(tokens[3], "output.txt") == 0);
    assert(tokens[4] == NULL);
}

static void test_multiple_attached_operators(void)
{
    char input[] = "cat<input.txt|sort>output.txt";
    char *tokens[MAX_ARGS];

    int count = tokenize(input, tokens);

    assert(count == 7);
    assert(strcmp(tokens[0], "cat") == 0);
    assert(strcmp(tokens[1], "<") == 0);
    assert(strcmp(tokens[2], "input.txt") == 0);
    assert(strcmp(tokens[3], "|") == 0);
    assert(strcmp(tokens[4], "sort") == 0);
    assert(strcmp(tokens[5], ">") == 0);
    assert(strcmp(tokens[6], "output.txt") == 0);
    assert(tokens[7] == NULL);
}

static void test_attached_append_operator(void)
{
    char input[] = "echo hello>>output.txt";
    char *tokens[MAX_ARGS];

    int count = tokenize(input, tokens);

    assert(count == 4);
    assert(strcmp(tokens[0], "echo") == 0);
    assert(strcmp(tokens[1], "hello") == 0);
    assert(strcmp(tokens[2], ">>") == 0);
    assert(strcmp(tokens[3], "output.txt") == 0);
    assert(tokens[4] == NULL);
}

int main(void)
{
    test_single_command();
    test_command_with_arguments();
    test_extra_whitespace();
    test_empty_input();
    test_operator_without_spaces();
    test_multiple_attached_operators();
    test_attached_append_operator();

    printf("All tokenizer tests passed.\n");
    return 0;
}
