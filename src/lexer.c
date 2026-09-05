#include <ctype.h>

#include "shell.h"

typedef enum {
    UNQUOTED,
    SINGLE_QUOTED,
    DOUBLE_QUOTED
} LexerState;


typedef struct {
    // Group 1: All 8-byte members (Total: 40 bytes)
    const char *cursor; // 8 bytes
    char *storage;      // 8 bytes
    size_t capacity;    // 8 bytes
    size_t used;        // 8 bytes
    Token *tokens;      // 8 bytes

    // Group 2: All 4-byte members packed together cleanly (Total: 12 bytes)
    int token_count;    // 4 bytes
    int inside_word;    // 4 bytes
    LexerState state;   // 4 bytes (Enum)

    // -- Only 4 bytes of trailing padding added here to reach a multiple of 8 --
} Lexer;

// ==============================================
//                Decleration
// ==============================================
static int consume_escape(Lexer *lexer);


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
//              Helper function
// ==============================================
static int consume_escape(Lexer *lexer)
{
    /*
     * cursor currently points at '\'.
     */
    if (lexer->cursor[1] == '\0') {
        return -1;
    }

    if (start_word(lexer) == -1) {
        return -1;
    }

    /*
     * Skip the backslash.
     */
    lexer->cursor++;

    /*
     * Copy the escaped character without interpreting it.
     */
    if (store_character(lexer, *lexer->cursor) == -1) {
        return -1;
    }

    /*
     * Move beyond the escaped character.
     */
    lexer->cursor++;

    return 0;
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
         * Single-quoted state.
         */
        if (lexer.state == SINGLE_QUOTED) {
            if (character == '\'') {
                lexer.state = UNQUOTED;
            } else {
                if (
                    store_character(
                        &lexer,
                        character
                    ) == -1
                ) {
                    return -1;
                }
            }

            lexer.cursor++;
            continue;
        }

        /*
         * Double-quoted state.
         */
        if (lexer.state == DOUBLE_QUOTED) {
            if (character == '"') {
                lexer.state = UNQUOTED;
                lexer.cursor++;
                continue;
            }

            if (character == '\\') {
                char next_character = lexer.cursor[1];

                if (next_character == '\0') {
                    return -1;
                }

                /*
                 * \" becomes "
                 * \\ becomes \
                 */
                if (
                    next_character == '"' ||
                    next_character == '\\'
                ) {
                    if (
                        store_character(
                            &lexer,
                            next_character
                        ) == -1
                    ) {
                        return -1;
                    }

                    lexer.cursor += 2;
                    continue;
                }

                /*
                 * Preserve the backslash for sequences such as \n.
                 * The next character will be processed on the next
                 * loop iteration.
                 */
                if (store_character(&lexer, '\\') == -1) {
                    return -1;
                }

                lexer.cursor++;
                continue;
            }

            /*
             * Ordinary character inside double quotes.
             */
            if (store_character(&lexer, character) == -1) {
                return -1;
            }

            lexer.cursor++;
            continue;
        }

        /*
         * From this point onward, state must be UNQUOTED.
         * Handle escaping while unquoted
         */
        if (character == '\\') {
            if (consume_escape(&lexer) == -1) {
                return -1;
            }

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

