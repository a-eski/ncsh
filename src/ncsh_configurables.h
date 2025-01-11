/* NCSH_DEBUG: enable debug mode for the shell */
// #define NCSH_DEBUG

/* NCSH_SHORT_DIRETCTORY: only show up to 2 directories in prompt line (defined) or show current path entirely (not
 * defined) */
#ifdef NCSH_SHORT_DIRECTORY
#undef NCSH_SHORT_DIRECTORY
#endif
#define NCSH_SHORT_DIRECTORY

/* NCSH_CLEAR_SCREEN_ON_STARTUP: clear screen on startup (defined) or don't clear screen on startup (not defined) */
#ifdef NCSH_CLEAR_SCREEN_ON_STARTUP
#undef NCSH_CLEAR_SCREEN_ON_STARTUP
#endif // NCSH_CLEAR_SCREEN_ON_STARTUP
// #define NCSH_CLEAR_SCREEN_ON_STARTUP

/* NCSH_START_TIME: display the amount of milliseconds it took to startup ncsh */
#ifdef NCSH_START_TIME
#undef NCSH_START_TIME
#endif
#define NCSH_START_TIME
