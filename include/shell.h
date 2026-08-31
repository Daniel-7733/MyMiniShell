#include <stddef.h>

#ifndef SHELL_H
#define SHELL_H

#define INPUT_SIZE 1024
#define MAX_ARGS 64

// ============================ 
//           enums
// ============================ 
typedef enum {
    BUILTIN_NOT_FOUND,
    BUILTIN_HANDLED,
    BUILTIN_EXIT
} BuiltinResult;

typedef enum {
    QUOTE_NONE,
    QUOTE_SINGLE,
    QUOTE_DOUBLE
} QuoteState;

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_INPUT,
    TOKEN_OUTPUT,
    TOKEN_APPEND
} TokenType;

// ============================ 
//          structs 
// ============================ 
typedef struct {
    TokenType type;
    char *text;
} Token;

typedef struct {
    char *input_file;
    char *output_file;
    int append;
} Redirection;

// typedef struct {
//     const char *read_cursor;
//
//     char *storage;
//     char *write_cursor;
//     size_t storage_size;
//
//     char **tokens;
//     int token_count;
//     int inside_word;
//
//     QuoteState quote_state;
// } Lexer;

// ============================ 
//          declerations 
// ============================ 
int tokenize(const char *input, char *storage, size_t storage_size, char *tokens[]);
int lex(const char *input, char *storage, size_t storage_size, Token tokens[]);
BuiltinResult execute_builtin(char *tokens[], int token_count);
int execute_external(char *tokens[]);
void print_help(void);
void print_working_directory(void);
void change_directory(char *tokens[], int token_count);
int find_pipe(char *tokens[]);
int execute_pipeline(char *tokens[]);

#endif
