#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

static void test_pipeline_with_redirection(void)
{
    char input_filename[128];
    char output_filename[128];

    snprintf(
        input_filename,
        sizeof(input_filename),
        "/tmp/minishell-pipe-input-%ld.txt",
        (long)getpid()
    );

    snprintf(
        output_filename,
        sizeof(output_filename),
        "/tmp/minishell-pipe-output-%ld.txt",
        (long)getpid()
    );

    FILE *input_file = fopen(input_filename, "w");
    assert(input_file != NULL);

    assert(fputs(
        "banana\napple\ncherry\napple\n",
        input_file
    ) >= 0);

    assert(fclose(input_file) == 0);

    char *tokens[] = {
        "/bin/cat",
        "<",
        input_filename,
        "|",
        "/usr/bin/sort",
        "|",
        "/usr/bin/uniq",
        ">",
        output_filename,
        NULL
    };

    assert(execute_pipeline(tokens) == 0);

    FILE *output_file = fopen(output_filename, "r");
    assert(output_file != NULL);

    char content[128];

    size_t bytes_read = fread(
        content,
        1,
        sizeof(content) - 1,
        output_file
    );

    content[bytes_read] = '\0';

    assert(strcmp(
        content,
        "apple\nbanana\ncherry\n"
    ) == 0);

    assert(fclose(output_file) == 0);
    assert(unlink(input_filename) == 0);
    assert(unlink(output_filename) == 0);
}

int main(void)
{
    test_pipe_transfers_matching_data();
    test_pipe_transfers_nonmatching_data();
    test_missing_left_command();
    test_missing_right_command();
    test_multiple_pipes();
    test_pipeline_with_redirection();

    printf("All pipeline tests passed.\n");
    return 0;
}

