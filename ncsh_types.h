#ifndef ncsh_types_h
#define ncsh_types_h

#include <linux/limits.h>
#include <stdint.h>

#define ESCAPE_CHARACTER 27
#define DOUBLE_QUOTE_KEY '\"'
#define CTRL_D '\004'
#define BACKSPACE_KEY 127
#define UP_ARROW 'A'
#define DOWN_ARROW 'B'
#define RIGHT_ARROW 'C'
#define LEFT_ARROW 'D'
#define DELETE_KEY '~'

#define MOVE_CURSOR_RIGHT "\033[1C"
#define MOVE_CURSOR_RIGHT_LENGTH 4
#define MOVE_CURSOR_LEFT "\033[1D"
#define MOVE_CURSOR_LEFT_LENGTH 4
#define BACKSPACE_STRING "\b \b"
#define BACKSPACE_STRING_LENGTH 3
#define BACKSPACE_AND_SAVE_POSITION_STRING "\b\0337"
#define BACKSPACE_AND_SAVE_POSITION_STRING_LENGTH 3
#define RESTORE_SAVED_POSITION_STRING "\0338"
#define RESTORE_SAVED_POSITION_STRING_LENGTH 2
#define ERASE_CURRENT_LINE "\033[K"
#define ERASE_CURRENT_LINE_LENGTH 3

struct ncsh_Directory {
	char* user;
	char path[PATH_MAX];
};

enum ncsh_Hotkey {
	NONE = 0,
	UP = 1,
	DOWN = 2,
	LEFT = 3,
	RIGHT = 4,
	DELETE = 5
};

#endif // !ncsh_types_h

