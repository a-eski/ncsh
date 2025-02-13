#ifndef ncsh_platform_h
#define ncsh_platform_h

#include "ncsh_terminal.h"
#include "ncsh_parser.h"
#include "ncsh_defines.h"
#include <unistd.h>

#if defined(linux) || defined(__unix__)

#include <linux/limits.h>
#include <sys/wait.h>

#else

#ifdef PATH_MAX
#undef PATH_MAX
#endif // PATH_MAX

#define PATH_MAX 4096

#ifdef MAX_INPUT
#undef MAX_INPUT
#endif // MAX_INPUT
#define MAX_INPUT 255

#endif // linux


int ncsh_platform_mkdir(const char* dir);

int ncsh_platform_kill(pid_t pid);

void ncsh_platform_glob(const char* buffer, struct ncsh_Args* args);

void ncsh_platform_terminal_init();
void ncsh_platform_terminal_reset();
struct ncsh_Coordinates ncsh_platform_terminal_size_get();

#endif /* endif ncsh_platform_h */
