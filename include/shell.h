#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>

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


typedef enum {
    UNQUOTED,
    SINGLE_QUOTED,
    DOUBLE_QUOTED
} LexerState;


// ============================ 
//          structs 
// ============================ 
typedef struct {
    char *text;      // 8 bytes (Pointer)
    TokenType type;  // 4 bytes (Enum)
    // 4 bytes of trailing padding added here to make total size a multiple of 8
} Token;


typedef struct {
    char *input_file;   // 8 bytes (Pointer)
    char *output_file;  // 8 bytes (Pointer)
    int append;         // 4 bytes (Int)
} Redirection;


typedef struct {
    char *argv[MAX_ARGS]; // 8 bytes each
    char *input_file;     // 8 bytes
    char *output_file;    // 8 bytes
    
    int argc;             // 4 bytes
    int append_output;    // 4 bytes
    // 0 bytes of middle padding!
} Command;


typedef struct {
    Command commands[MAX_ARGS]; // Array of 8-byte aligned structs
    int command_count;          // 4 bytes
    // 4 bytes of trailing padding (unavoidable)
} CommandLine;


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
int parse_tokens(const Token tokens[], int token_count, CommandLine *commands_line);
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
