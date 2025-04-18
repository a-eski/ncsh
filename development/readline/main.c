/* experiment with using GNU readline */

#include <stdio.h>
// #include <readline/ncsh_arena.h>
// #include <readline/ncsh_autocompletions.h>
// #include <readline/ncsh_string.h>
#include <readline/history.h>
#include <readline/ncsh_readline.h>
#include <stdlib.h>

int main(void)
{
    readline_input input = {0};
    input.prompt = "> ";

    while (1) {
        char* line = ncsh_readline(&input);
        if (line) {
            // Use the input line
            printf("You entered: %s\n", line);
            // Add the line to history
            add_history(line);
            // Free the allocated memory
            free(line);
        }
        else {
            // Handle EOF (e.g., Ctrl+D)
            printf("\n");
        }
    }
}
