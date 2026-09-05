#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "shell.h"

static void test_quoted_operators_are_words(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    int count = lex(
        "echo \"|\" '>'",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 3);

    assert(tokens[0].type == TOKEN_WORD);
    assert(strcmp(tokens[0].text, "echo") == 0);

    assert(tokens[1].type == TOKEN_WORD);
    assert(strcmp(tokens[1].text, "|") == 0);

    assert(tokens[2].type == TOKEN_WORD);
    assert(strcmp(tokens[2].text, ">") == 0);
}

static void test_attached_operators(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    int count = lex(
        "cat<in.txt|sort>>out.txt",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 7);

    assert(tokens[0].type == TOKEN_WORD);
    assert(strcmp(tokens[0].text, "cat") == 0);

    assert(tokens[1].type == TOKEN_INPUT);

    assert(tokens[2].type == TOKEN_WORD);
    assert(strcmp(tokens[2].text, "in.txt") == 0);

    assert(tokens[3].type == TOKEN_PIPE);

    assert(tokens[4].type == TOKEN_WORD);
    assert(strcmp(tokens[4].text, "sort") == 0);

    assert(tokens[5].type == TOKEN_APPEND);

    assert(tokens[6].type == TOKEN_WORD);
    assert(strcmp(tokens[6].text, "out.txt") == 0);
}

static void test_empty_and_joined_words(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    int count = lex(
        "echo \"\" ab\"cd ef\"gh",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 3);

    assert(tokens[1].type == TOKEN_WORD);
    assert(strcmp(tokens[1].text, "") == 0);

    assert(tokens[2].type == TOKEN_WORD);
    assert(strcmp(tokens[2].text, "abcd efgh") == 0);
}

static void test_invalid_input(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    assert(
        lex(
            "echo \"unfinished",
            storage,
            sizeof(storage),
            tokens
        ) == -1
    );
}

static void test_small_storage(void)
{
    char storage[1];
    Token tokens[MAX_ARGS];

    /*
     * "a" needs two bytes: 'a' followed by '\0'.
     */
    assert(
        lex("a", storage, sizeof(storage), tokens) == -1
    );

    /*
     * An empty quoted word needs only its '\0'.
     */
    assert(
        lex("\"\"", storage, sizeof(storage), tokens) == 1
    );

    assert(strcmp(tokens[0].text, "") == 0);

    assert(
        lex("\"\"", storage, 0, tokens) == -1
    );
}

static void test_escaped_whitespace(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    int count = lex(
        "echo hello\\ world",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 2);
    assert(tokens[0].type == TOKEN_WORD);
    assert(strcmp(tokens[0].text, "echo") == 0);
    assert(tokens[1].type == TOKEN_WORD);
    assert(strcmp(tokens[1].text, "hello world") == 0);
}

static void test_escaped_operators_are_words(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    int count = lex(
        "echo \\| \\> \\<",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 4);

    assert(tokens[1].type == TOKEN_WORD);
    assert(strcmp(tokens[1].text, "|") == 0);

    assert(tokens[2].type == TOKEN_WORD);
    assert(strcmp(tokens[2].text, ">") == 0);

    assert(tokens[3].type == TOKEN_WORD);
    assert(strcmp(tokens[3].text, "<") == 0);
}

static void test_escape_inside_double_quotes(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    int count = lex(
        "echo \"hello \\\"world\\\"\"",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 2);
    assert(tokens[1].type == TOKEN_WORD);
    assert(strcmp(
        tokens[1].text,
        "hello \"world\""
    ) == 0);
}

static void test_single_quotes_preserve_backslash(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    int count = lex(
        "printf '\\n'",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 2);
    assert(strcmp(tokens[1].text, "\\n") == 0);
}

static void test_trailing_escape(void)
{
    char storage[INPUT_SIZE];
    Token tokens[MAX_ARGS];

    int count = lex(
        "echo hello\\",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == -1);
}

static void test_backslash_preserved_inside_double_quotes(void)
{
    char storage[128];
    Token tokens[MAX_ARGS];

    int count = lex(
        "printf \"banana\\napple\\n\"",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 2);
    assert(tokens[0].type == TOKEN_WORD);
    assert(strcmp(tokens[0].text, "printf") == 0);
    assert(tokens[1].type == TOKEN_WORD);
    assert(strcmp(tokens[1].text, "banana\\napple\\n") == 0);
}

static void test_backslashes_inside_double_quotes(void)
{
    char storage[128];
    Token tokens[MAX_ARGS];

    int count = lex(
        "printf \"banana\\napple\\nbanana\\n\"",
        storage,
        sizeof(storage),
        tokens
    );

    assert(count == 2);

    assert(tokens[0].type == TOKEN_WORD);
    assert(strcmp(tokens[0].text, "printf") == 0);

    assert(tokens[1].type == TOKEN_WORD);
    assert(
        strcmp(
            tokens[1].text,
            "banana\\napple\\nbanana\\n"
        ) == 0
    );
}

int main(void)
{
    test_quoted_operators_are_words();
    test_attached_operators();
    test_empty_and_joined_words();
    test_invalid_input();
    test_small_storage();
    test_escaped_whitespace();
    test_escaped_operators_are_words();
    test_escape_inside_double_quotes();
    test_single_quotes_preserve_backslash();
    test_trailing_escape();
    test_backslash_preserved_inside_double_quotes();
    test_backslashes_inside_double_quotes();


    printf("All typed-lexer tests passed.\n");
    return 0;
}
