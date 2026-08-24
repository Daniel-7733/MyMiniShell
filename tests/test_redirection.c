#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"

static void test_output_redirection(void)
{
    char filename[128];

    snprintf(
        filename,
        sizeof(filename),
        "/tmp/minishell-redirection-%ld.txt",
        (long)getpid()
    );

    char *tokens[] = {
        "/bin/echo",
        "hello",
        ">",
        filename,
        NULL
    };

    int status = execute_external(tokens);

    assert(status == 0);

    FILE *file = fopen(filename, "r");
    assert(file != NULL);

    char content[64];

    assert(fgets(content, sizeof(content), file) != NULL);
    assert(strcmp(content, "hello\n") == 0);

    assert(fclose(file) == 0);
    assert(unlink(filename) == 0);
}

static void test_append_redirection(void)
{
    char filename[128];

    snprintf(
        filename,
        sizeof(filename),
        "/tmp/minishell-append-%ld.txt",
        (long)getpid()
    );

    char *first_command[] = {
        "/bin/echo",
        "first",
        ">",
        filename,
        NULL
    };

    char *second_command[] = {
        "/bin/echo",
        "second",
        ">>",
        filename,
        NULL
    };

    assert(execute_external(first_command) == 0);
    assert(execute_external(second_command) == 0);

    FILE *file = fopen(filename, "r");
    assert(file != NULL);

    char first_line[64];
    char second_line[64];

    assert(fgets(first_line, sizeof(first_line), file) != NULL);
    assert(fgets(second_line, sizeof(second_line), file) != NULL);

    assert(strcmp(first_line, "first\n") == 0);
    assert(strcmp(second_line, "second\n") == 0);

    assert(fclose(file) == 0);
    assert(unlink(filename) == 0);
}

static void test_combined_redirection(void)
{
    char input_filename[128];
    char output_filename[128];

    snprintf(
        input_filename,
        sizeof(input_filename),
        "/tmp/minishell-input-%ld.txt",
        (long)getpid()
    );

    snprintf(
        output_filename,
        sizeof(output_filename),
        "/tmp/minishell-output-%ld.txt",
        (long)getpid()
    );

    FILE *input_file = fopen(input_filename, "w");
    assert(input_file != NULL);

    assert(fputs("banana\napple\ncherry\n", input_file) >= 0);
    assert(fclose(input_file) == 0);

    char *tokens[] = {
        "sort",
        "<",
        input_filename,
        ">",
        output_filename,
        NULL
    };

    assert(execute_external(tokens) == 0);

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
    test_output_redirection();
    test_append_redirection();
    test_combined_redirection();

    printf("All redirection tests passed.\n");
    return 0;
}

