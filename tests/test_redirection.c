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

int main(void)
{
    test_output_redirection();

    printf("All redirection tests passed.\n");
    return 0;
}
