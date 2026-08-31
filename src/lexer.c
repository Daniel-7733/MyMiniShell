#include <ctype.h>

#include "shell.h"

typedef enum {
    UNQUOTED,
    SINGLE_QUOTED,
    DOUBLE_QUOTED
} LexerState;

typedef struct {
    const char *cursor;

    char *storage;
    size_t capacity;
    size_t used;

    Token *tokens;
    int token_count;
    int inside_word;

    LexerState state;
} Lexer;

// ==============================================
//     Helper: emit one typed token
// ==============================================
static int emit_token(Lexer *lexer, TokenType type, char *text)
{
    /*
     * Reserve one slot for the future argv NULL terminator.
     */
    if (lexer->token_count >= MAX_ARGS - 1) {
        return -1;
    }

    lexer->tokens[lexer->token_count].type = type;
    lexer->tokens[lexer->token_count].text = text;
    lexer->token_count++;

    return 0;
}

// ==============================================
//     Helper: write one byte safely
// ==============================================
/*
 * This checks capacity for every byte, including string terminators.
 * It also avoids subtracting one from a possibly zero-sized buffer.
 */
static int store_character(Lexer *lexer, char character)
{
    if (lexer->used >= lexer->capacity) {
        return -1;
    }

    lexer->storage[lexer->used] = character;
    lexer->used++;

    return 0;
}

// ==============================================
//     Helpers: start and finish words
// ==============================================
static int start_word(Lexer *lexer)
{
    if (lexer->inside_word) {
        return 0;
    }

    if (lexer->used >= lexer->capacity) {
        return -1;
    }

    if (emit_token(lexer, TOKEN_WORD, &lexer->storage[lexer->used]) == -1) {
        return -1;
    }

    lexer->inside_word = 1;
    return 0;
}

static int finish_word(Lexer *lexer)
{
    if (!lexer->inside_word) {
        return 0;
    }

    if (store_character(lexer, '\0') == -1) {
        return -1;
    }

    lexer->inside_word = 0;
    return 0;
}

// ==============================================
//     Helpers: start and finish words
// ==============================================
static int emit_operator(Lexer *lexer)
{
    char character = *lexer->cursor;

    if (character == '>' && lexer->cursor[1] == '>') {
        lexer->cursor += 2;
        return emit_token(lexer, TOKEN_APPEND, ">>");
    }

    lexer->cursor++;

    if (character == '>') {
        return emit_token(lexer, TOKEN_OUTPUT, ">");
    }

    if (character == '<') {
        return emit_token(lexer, TOKEN_INPUT, "<");
    }

    return emit_token(lexer, TOKEN_PIPE, "|");
}

// ==============================================
//          Complete function
// ==============================================
int lex(const char *input, char *storage, size_t storage_size, Token tokens[])
{
    Lexer lexer = {
        .cursor = input,
        .storage = storage,
        .capacity = storage_size,
        .used = 0,
        .tokens = tokens,
        .token_count = 0,
        .inside_word = 0,
        .state = UNQUOTED
    };

    while (*lexer.cursor != '\0') {
        char character = *lexer.cursor;

        /*
         * While quoted, only the matching closing quote
         * has special meaning.
         */
        if (lexer.state != UNQUOTED) {
            char closing_quote = lexer.state == SINGLE_QUOTED ? '\'' : '"';

            if (character == closing_quote) {
                lexer.state = UNQUOTED;
            } else {
                if (store_character(&lexer, character) == -1) {
                    return -1;
                }
            }

            lexer.cursor++;
            continue;
        }

        /*
         * Unquoted whitespace separates words.
         */
        if (isspace((unsigned char)character)) {
            if (finish_word(&lexer) == -1) {
                return -1;
            }

            lexer.cursor++;
            continue;
        }

        /*
         * Opening quotes start or continue a word.
         */
        if (character == '\'' || character == '"') {
            if (start_word(&lexer) == -1) {
                return -1;
            }

            lexer.state = character == '\'' ? SINGLE_QUOTED : DOUBLE_QUOTED;

            lexer.cursor++;
            continue;
        }

        /*
         * Only unquoted operators receive operator types.
         */
        if (character == '|' || character == '<' || character == '>') {
            if (finish_word(&lexer) == -1) {
                return -1;
            }

            if (emit_operator(&lexer) == -1) {
                return -1;
            }

            continue;
        }

        /*
         * An ordinary character belongs to a word.
         */
        if (start_word(&lexer) == -1) {
            return -1;
        }

        if (store_character(&lexer, character) == -1) {
            return -1;
        }

        lexer.cursor++;
    }

    if (lexer.state != UNQUOTED) {
        return -1;
    }

    if (finish_word(&lexer) == -1) {
        return -1;
    }

    return lexer.token_count;
}

