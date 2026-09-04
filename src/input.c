#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "input.h"

static InputResult discard_remaining_line(FILE *stream)
{
    int character;

    while ((character = fgetc(stream)) != '\n' && character != EOF) {
        /*
         * Discard every remaining character.
         */
    }

    if (character == EOF && ferror(stream)) {
        return INPUT_ERROR;
    }

    return INPUT_TOO_LONG;
}

InputResult read_command_line(FILE *stream, char buffer[], size_t buffer_size)
{
    if (stream == NULL || buffer == NULL || buffer_size < 2 || buffer_size > INT_MAX) {
        return INPUT_ERROR;
    }

    if (fgets(buffer, (int)buffer_size, stream) == NULL) {
        if (feof(stream)) {
            return INPUT_END;
        }

        return INPUT_ERROR;
    }

    size_t length = strlen(buffer);

    /*
     * Normal case: fgets stored the newline.
     */
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
        return INPUT_OK;
    }

    /*
     * No newline was stored. Read one extra character
     * to distinguish an exactly-full valid line from
     * an overlong line.
     */
    int next_character = fgetc(stream);

    if (next_character == '\n') {
        return INPUT_OK;
    }

    if (next_character == EOF) {
        if (ferror(stream)) {
            return INPUT_ERROR;
        }

        /*
         * The final line ended at EOF without a newline.
         */
        return INPUT_OK;
    }

    /*
     * There is more command text. Discard that character
     * and everything through the next newline.
     */
    buffer[0] = '\0';

    InputResult discard_result = discard_remaining_line(stream);

    if (discard_result == INPUT_ERROR) {
        return INPUT_ERROR;
    }

    return INPUT_TOO_LONG;
}
