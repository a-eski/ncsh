// Copyright (c) ncsh by Alex Eski 2025

/* Debug Settings */
/* NCSH_DEBUG: enable debug mode for the shell */
#ifndef NCSH_DEBUG
// #define NCSH_DEBUG
#endif // !NCSH_DEBUG



/* Prompt Settings */
/* NCSH_PROMPT_DIRECTORY section: do you want to show the entire cwd in the prompt line? no directory?
 * a shortened version? use this configurable. */
/* NCSH_DIRECTORY_{OPTION}: options for NCSH_PROMPT_DIRECTORY */
#define NCSH_DIRECTORY_NORMAL 0  // show the current working directory in the prompt line
#define NCSH_DIRECTORY_SHORT  1  // show up to 2 of the parent directories in the prompt line
#define NCSH_DIRECTORY_NONE   2  // do not show the current working directory in the prompt line

#ifndef NCSH_PROMPT_DIRECTORY
/* put the option from above that you want after NCSH_PROMPT_DIRECTORY */
#define NCSH_PROMPT_DIRECTORY NCSH_DIRECTORY_SHORT
#endif // !NCSH_PROMPT_DIRECTORY

/* NCSH_PROMPT_ENDING_STRING: the string at the end of the prompt line, traditionally "> " or "$ ".
 * Allows you to control spacing as well, if you want "{user} {directory}> ", you would put "> ".*/
#ifndef NCSH_PROMPT_ENDING_STRING
/* NCSH_PROMPT_ENDING_DEBUG: useful for defining when ncsh is your shell but you want to run a debug build and tell the difference. */
#ifdef NCSH_PROMPT_ENDING_DEBUG
#define NCSH_PROMPT_ENDING_STRING " (d) \u2771 "
#else
#define NCSH_PROMPT_ENDING_STRING " \u2771 "
#endif /* NCSH_PROMPT_ENDING_DEBUG  */
#endif // !NCSH_PROMPT_ENDING_STRING


/* NCSH_PROMPT_SHOW_USER: whether or not to show the current user at the start of the prompt. */
/* NCSH_USER_{OPTION}: options for NCSH_PROMPT_SHOW_USER */
#define NCSH_SHOW_USER_NORMAL 0
#define NCSH_SHOW_USER_NONE 1

#ifndef NCSH_PROMPT_SHOW_USER
#define NCSH_PROMPT_SHOW_USER NCSH_SHOW_USER_NORMAL
#endif // !NCSH_PROMPT_SHOW_USER



/* Startup Settings */
/* NCSH_CLEAR_SCREEN_ON_STARTUP: clear screen on startup (defined) or don't clear screen on startup (not defined).
 * Useful for when you are doing debugging/dev and don't want to clear the screen on startup. */
#ifndef NCSH_CLEAR_SCREEN_ON_STARTUP
// #define NCSH_CLEAR_SCREEN_ON_STARTUP
#endif // !NCSH_CLEAR_SCREEN_ON_STARTUP

/* NCSH_START_TIME: display the amount of milliseconds it took to startup ncsh when defined. */
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
