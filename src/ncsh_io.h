// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_io_h
#define ncsh_io_h

#include <stdint.h>
#include <linux/limits.h>

//input definitions
#define ESCAPE_CHARACTER 27
#define DOUBLE_QUOTE_KEY '\"'
#define CTRL_D '\004'
#define BACKSPACE_KEY 127
#define UP_ARROW 'A'
#define DOWN_ARROW 'B'
#define RIGHT_ARROW 'C'
#define LEFT_ARROW 'D'
#define DELETE_PREFIX_KEY 51
#define DELETE_KEY '~'
#define TAB_KEY '\t'

//terminal manipulations
#define MOVE_CURSOR_RIGHT "\033[1C"
#define MOVE_CURSOR_RIGHT_LENGTH 4
#define MOVE_CURSOR_LEFT "\033[1D"
#define MOVE_CURSOR_LEFT_LENGTH 4
#define BACKSPACE_STRING "\b \b"
#define BACKSPACE_STRING_LENGTH 3
#define DELETE_STRING " \b"
#define DELETE_STRING_LENGTH 2
#define GET_CURSOR_POSITION "\033[6n"
#define GET_CURSOR_POSITION_LENGTH 4
#define SAVE_CURSOR_POSITION "\033[s"
#define SAVE_CURSOR_POSITION_LENGTH 3
#define RESTORE_CURSOR_POSITION "\033[u"
#define RESTORE_CURSOR_POSITION_LENGTH 3
#define ERASE_CURRENT_LINE "\033[K"
#define ERASE_CURRENT_LINE_LENGTH 3

#define CLEAR_SCREEN "\033[2J"
#define CLEAR_SCREEN_LENGTH 4
#define MOVE_CURSOR_HOME "\033[H"
#define MOVE_CURSOR_HOME_LENGTH 3

struct ncsh_Directory {
	bool reprint_prompt;
	char* user;
	char path[PATH_MAX];
};

enum ncsh_Hotkey {
	NONE = 0,
	UP = 1,
	DOWN = 2,
	LEFT = 3,
	RIGHT = 4,
	DELETE_PREFIX = 5
};

#endif // !ncsh_io_h

