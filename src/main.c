#include <unistd.h>

#include "ncsh.h"
#include "ncsh_noninteractive.h"

int main(int argc, char **argv)
{
    if (argc > 1 || !isatty(STDIN_FILENO))
        return ncsh_noninteractive(argc, argv);
    else
        return ncsh();
}
