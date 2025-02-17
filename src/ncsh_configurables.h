// Copyright (c) ncsh by Alex Eski 2025

/* Debug Settings */
/* NCSH_DEBUG: enable debug mode for the shell */
#ifndef NCSH_DEBUG
// #define NCSH_DEBUG
#endif // !NCSH_DEBUG



/* Prompt Settings */
/* NCSH_SHORT_DIRECTORY: only show up to 2 directories in prompt line (defined) or show current path entirely (not
 * defined) */
#ifndef NCSH_SHORT_DIRECTORY
#define NCSH_SHORT_DIRECTORY
#endif // !NCSH_SHORT_DIRECTORY

/* NCSH_PROMPT_ENDING_STRING: the string at the end of the prompt line, traditionally "> " or "$ ".
 * Allows you to control spacing as well, if you want "{user} {directory}> ", you would put "> ".*/
#ifndef NCSH_PROMPT_ENDING_STRING
#define NCSH_PROMPT_ENDING_STRING " \u2771 "
#endif // !NCSH_PROMPT_ENDING_STRING



/* Startup Settings */
/* NCSH_CLEAR_SCREEN_ON_STARTUP: clear screen on startup (defined) or don't clear screen on startup (not defined) */
#ifndef NCSH_CLEAR_SCREEN_ON_STARTUP
#define NCSH_CLEAR_SCREEN_ON_STARTUP
#endif // !NCSH_CLEAR_SCREEN_ON_STARTUP

/* NCSH_START_TIME: display the amount of milliseconds it took to startup ncsh */
#ifndef NCSH_START_TIME
#define NCSH_START_TIME
#endif // !NCSH_START_TIME



/* History Settings */
/* NCSH_MAX_HISTORY_FILE: the maximum number of history entries to save to the history file */
#ifndef NCSH_MAX_HISTORY_FILE
#define NCSH_MAX_HISTORY_FILE 2000
#endif // !NCSH_MAX_HISTORY_FILE

/* NCSH_MAX_HISTORY_FILE: the maximum number of history entries to be able to hold in memory while the program is running. */
#ifndef NCSH_MAX_HISTORY_IN_MEMORY
#define NCSH_MAX_HISTORY_IN_MEMORY 2400
#endif // !NCSH_MAX_HISTORY_IN_MEMORY
