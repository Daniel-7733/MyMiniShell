#ifndef SHELL_H
#define SHELL_H

#define INPUT_SIZE 1024
#define MAX_ARGS 64

int tokenize(char *input, char *tokens[]);
int execute_builtin(char *tokens[], int token_count);
void print_help(void);
void print_working_directory(void);
void change_directory(char *tokens[], int token_count);

#endif
