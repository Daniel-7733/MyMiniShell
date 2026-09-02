#include <assert.h>
#include <stdio.h>

#include "shell.h"

static void test_successful_command(void)
{
    Command command = {
        .argv = {"/bin/true", NULL},
        .argc = 1
    };

    assert(execute_command(&command) == 0);
}

static void test_failed_command(void)
{
    Command command = {
        .argv = {"/bin/false", NULL},
        .argc = 1
    };

    assert(execute_command(&command) == 1);
}

static void test_missing_command(void)
{
    Command command = {
        .argv = {
            "command-that-does-not-exist",
            NULL
        },
        .argc = 1
    };

    assert(execute_command(&command) == 127);
}

int main(void)
{
    test_successful_command();
    test_failed_command();
    test_missing_command();

    printf("All external-command tests passed.\n");
    return 0;
}

