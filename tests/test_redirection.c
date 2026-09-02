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

    Command command = {
        .argv = {"/bin/echo", "hello", NULL},
        .argc = 2,
        .output_file = filename,
        .append_output = 0
    };

    assert(execute_command(&command) == 0);

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

    Command first = {
        .argv = {"/bin/echo", "first", NULL},
        .argc = 2,
        .output_file = filename,
        .append_output = 0
    };

    Command second = {
        .argv = {"/bin/echo", "second", NULL},
        .argc = 2,
        .output_file = filename,
        .append_output = 1
    };

    assert(execute_command(&first) == 0);
    assert(execute_command(&second) == 0);

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

    assert(
        fputs(
            "banana\napple\ncherry\n",
            input_file
        ) >= 0
    );

    assert(fclose(input_file) == 0);

    Command command = {
        .argv = {"/usr/bin/sort", NULL},
        .argc = 1,
        .input_file = input_filename,
        .output_file = output_filename,
        .append_output = 0
    };

    assert(execute_command(&command) == 0);

    FILE *output_file = fopen(
        output_filename,
        "r"
    );

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

