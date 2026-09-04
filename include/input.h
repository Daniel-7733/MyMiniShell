#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>
#include <stdio.h>

typedef enum {
    INPUT_OK,
    INPUT_END,
    INPUT_TOO_LONG,
    INPUT_ERROR
} InputResult;

InputResult read_command_line(FILE *stream, char buffer[], size_t buffer_size);

#endif
