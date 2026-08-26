#ifndef SHELL_H
#define SHELL_H

#define INPUT_SIZE 1024
#define MAX_ARGS 64

typedef enum {
    BUILTIN_NOT_FOUND,
    BUILTIN_HANDLED,
    BUILTIN_EXIT
} BuiltinResult;

typedef struct {
    char *input_file;
    char *output_file;
    int append;
} Redirection;

int tokenize(char *input, char *tokens[]);
BuiltinResult execute_builtin(char *tokens[], int token_count);
int execute_external(char *tokens[]);
void print_help(void);
void print_working_directory(void);
void change_directory(char *tokens[], int token_count);
int find_pipe(char *tokens[]);
int execute_pipeline(char *tokens[]);

#endif
