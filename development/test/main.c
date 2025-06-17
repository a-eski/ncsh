#include <stdio.h>
#include <unistd.h>

int main(void)
{
    puts("starting test");

    char character;
    while (character != 'z') {
        read(STDIN_FILENO, &character, 1);
        putchar(character);
    }

    return 0;
}
