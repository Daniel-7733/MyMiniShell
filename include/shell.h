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
    char *text;      // 8 bytes (Pointer)
    TokenType type;  // 4 bytes (Enum)
    // 4 bytes of trailing padding added here to make total size a multiple of 8
} Token;


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


// ============================ 
//          declerations 
// ============================ 
int lex(const char *input, char *storage, size_t storage_size, Token tokens[]);
int parse_tokens(const Token tokens[], int token_count, CommandLine *command_line);
BuiltinResult execute_builtin(char *arguments[], int argument_count);
void print_help(void);
void print_working_directory(void);
void change_directory(char *arguments[], int argument_count);
int execute_command(const Command *command);
int execute_pipeline(const CommandLine *command_line);

#endif
