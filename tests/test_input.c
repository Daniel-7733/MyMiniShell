#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "input.h"

static FILE *create_test_stream(const char *content)
{
    FILE *stream = tmpfile();
    assert(stream != NULL);

    assert(fputs(content, stream) >= 0);
    rewind(stream);

    return stream;
}

static void test_normal_lines(void)
{
    FILE *stream = create_test_stream(
        "echo hello\nexit\n"
    );

    char buffer[32];

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_OK
    );

    assert(strcmp(buffer, "echo hello") == 0);

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_OK
    );

    assert(strcmp(buffer, "exit") == 0);

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_END
    );

    assert(fclose(stream) == 0);
}

static void test_exactly_full_line(void)
{
    FILE *stream = create_test_stream("abcd\n");
    char buffer[5];

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_OK
    );

    assert(strcmp(buffer, "abcd") == 0);
    assert(fclose(stream) == 0);
}

static void test_overlong_line_is_discarded(void)
{
    FILE *stream = create_test_stream(
        "abcdef\nnext\n"
    );

    char buffer[5];

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_TOO_LONG
    );

    assert(buffer[0] == '\0');

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_OK
    );

    assert(strcmp(buffer, "next") == 0);
    assert(fclose(stream) == 0);
}

static void test_final_line_without_newline(void)
{
    FILE *stream = create_test_stream("exit");
    char buffer[16];

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_OK
    );

    assert(strcmp(buffer, "exit") == 0);

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_END
    );

    assert(fclose(stream) == 0);
}

static void test_empty_line(void)
{
    FILE *stream = create_test_stream("\n");
    char buffer[16];

    assert(
        read_command_line(
            stream,
            buffer,
            sizeof(buffer)
        ) == INPUT_OK
    );

    assert(strcmp(buffer, "") == 0);
    assert(fclose(stream) == 0);
}

int main(void)
{
    test_normal_lines();
    test_exactly_full_line();
    test_overlong_line_is_discarded();
    test_final_line_without_newline();
    test_empty_line();

    printf("All input tests passed.\n");
    return 0;
}
