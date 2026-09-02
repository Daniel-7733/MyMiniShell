#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"

static int execute_pipeline_text(const char *input)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];
    CommandLine command_line;

    int token_count = lex(
        input,
        storage,
        sizeof(storage),
        tokens
    );

    assert(token_count >= 0);

    int parse_result = parse_tokens(
        tokens,
        token_count,
        &command_line
    );

    assert(parse_result == 0);
    assert(command_line.command_count > 1);

    /*
     * execute_pipeline() finishes before this function
     * returns, so storage remains alive while argv and
     * filename pointers are being used.
     */
    return execute_pipeline(&command_line);
}

static void test_matching_pipeline(void)
{
    int result = execute_pipeline_text(
        "/bin/echo hello | "
        "/usr/bin/grep -q hello"
    );

    assert(result == 0);
}

static void test_nonmatching_pipeline(void)
{
    int result = execute_pipeline_text(
        "/bin/echo hello | "
        "/usr/bin/grep -q goodbye"
    );

    /*
     * grep returns 1 when it finds no matching line.
     */
    assert(result == 1);
}

static void test_multiple_pipes(void)
{
    int result = execute_pipeline_text(
        "/bin/echo hello | "
        "/usr/bin/tr a-z A-Z | "
        "/usr/bin/grep -q HELLO"
    );

    assert(result == 0);
}

static void test_pipeline_with_redirection(void)
{
    char input_filename[128];
    char output_filename[128];
    char command_text[INPUT_SIZE];

    snprintf(
        input_filename,
        sizeof(input_filename),
        "/tmp/minishell-pipeline-input-%ld.txt",
        (long)getpid()
    );

    snprintf(
        output_filename,
        sizeof(output_filename),
        "/tmp/minishell-pipeline-output-%ld.txt",
        (long)getpid()
    );

    FILE *input_file = fopen(input_filename, "w");
    assert(input_file != NULL);

    assert(
        fputs(
            "banana\napple\ncherry\napple\n",
            input_file
        ) >= 0
    );

    assert(fclose(input_file) == 0);

    int written = snprintf(
        command_text,
        sizeof(command_text),
        "/bin/cat<%s|"
        "/usr/bin/sort|"
        "/usr/bin/uniq>%s",
        input_filename,
        output_filename
    );

    assert(written >= 0);
    assert((size_t)written < sizeof(command_text));

    assert(execute_pipeline_text(command_text) == 0);

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
    test_matching_pipeline();
    test_nonmatching_pipeline();
    test_multiple_pipes();
    test_pipeline_with_redirection();

    printf("All pipeline tests passed.\n");
    return 0;
}
