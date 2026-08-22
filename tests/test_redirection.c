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

int main(void)
{
    test_output_redirection();
    test_append_redirection();

    printf("All redirection tests passed.\n");
    return 0;
}

