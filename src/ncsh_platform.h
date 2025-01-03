#ifndef ncsh_platform_h
#define ncsh_platform_h

#if defined(linux) || defined(__unix__)

#include <linux/limits.h>
#include <sys/wait.h>
#include <glob.h>

#else

#undef PATH_MAX
#define PATH_MAX 4096

#undef MAX_INPUT
#define MAX_INPUT 255

#define pipe(input) 0
#define fork() 0
#define kill(pid, sig)
#define waitpid(pid, status, flag)
#define WIFEXITED(status) true
#define WIFSIGNALED(status) true

#endif // linux

#endif /* endif ncsh_platform_h */
