#include <assert.h>
#include <stdio.h>

#include "shell.h"

static void test_pipe_transfers_matching_data(void)
{
    char *tokens[] = {
        "/bin/echo",
        "hello",
        "|",
        "/usr/bin/grep",
        "-q",
        "hello",
        NULL
    };

    int pipe_index = find_pipe(tokens);

    assert(pipe_index == 2);
    assert(execute_pipeline(tokens) == 0);
}

static void test_pipe_transfers_nonmatching_data(void)
{
    char *tokens[] = {
        "/bin/echo",
        "hello",
        "|",
        "/usr/bin/grep",
        "-q",
        "goodbye",
        NULL
    };

    int pipe_index = find_pipe(tokens);

    assert(pipe_index == 2);
    assert(execute_pipeline(tokens) == 1);
}

static void test_missing_left_command(void)
{
    char *tokens[] = {
        "|",
        "/usr/bin/wc",
        "-l",
        NULL
    };

    int pipe_index = find_pipe(tokens);

    assert(pipe_index == 0);
    assert(execute_pipeline(tokens) == 2);
}

static void test_missing_right_command(void)
{
    char *tokens[] = {
        "/bin/echo",
        "hello",
        "|",
        NULL
    };

    int pipe_index = find_pipe(tokens);

    assert(pipe_index == 2);
    assert(execute_pipeline(tokens) == 2);
}

static void test_multiple_pipes(void)
{
    char *tokens[] = {
        "/bin/echo",
        "hello",
        "|",
        "/usr/bin/tr",
        "a-z",
        "A-Z",
        "|",
        "/usr/bin/grep",
        "-q",
        "HELLO",
        NULL
    };

    assert(find_pipe(tokens) == 2);
    assert(execute_pipeline(tokens) == 0);
}

int main(void)
{
    test_pipe_transfers_matching_data();
    test_pipe_transfers_nonmatching_data();
    test_missing_left_command();
    test_missing_right_command();
    test_multiple_pipes();

    printf("All pipeline tests passed.\n");
    return 0;
}
