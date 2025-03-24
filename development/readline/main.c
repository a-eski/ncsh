/* experiment with using GNU readline */

#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>

int main(void)
{
    while (1) {
	char* line = readline("prompt> ");
        if (line) {
            // Use the input line
            printf("You entered: %s\n", line);
            // Add the line to history
            add_history(line);
            // Free the allocated memory
            free(line);
        } else {
            // Handle EOF (e.g., Ctrl+D)
            printf("\n");
        }
    }
}
