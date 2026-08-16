#include <stdio.h>
#include <string.h>

#define INPUT_SIZE 1024

int main(void)
{
    char input[INPUT_SIZE];

    while (1) {
        printf("minishell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            putchar('\n');
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            break;
        }
    }

    return 0;
}
