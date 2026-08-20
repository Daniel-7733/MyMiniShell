#include <assert.h>
#include <stdio.h>

#include "shell.h"

static void test_successful_command(void)
{
    char *tokens[] = {"/bin/true", NULL};

    int status = execute_external(tokens);

    assert(status == 0);
}

static void test_failed_command(void)
{
    char *tokens[] = {"/bin/false", NULL};

    int status = execute_external(tokens);

    assert(status == 1);
}

static void test_missing_command(void)
{
    char *tokens[] = {"command-that-does-not-exist", NULL};

    int status = execute_external(tokens);

    assert(status == 127);
}

int main(void)
{
    test_successful_command();
    test_failed_command();
    test_missing_command();

    printf("All external-command tests passed.\n");
    return 0;
}

