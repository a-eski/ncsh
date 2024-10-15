/* Copyright ncsh by Alex Eski 2024 */

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

//terminal manipulations
#define MOVE_CURSOR_RIGHT "\033[1C"
#define MOVE_CURSOR_RIGHT_LENGTH 4
#define MOVE_CURSOR_LEFT "\033[1D"
#define MOVE_CURSOR_LEFT_LENGTH 4
#define BACKSPACE_STRING "\b \b"
#define BACKSPACE_STRING_LENGTH 3
#define DELETE_STRING " \b"
#define DELETE_STRING_LENGTH 2
#define SAVE_CURSOR_POSITION "\033[s"
#define SAVE_CURSOR_POSITION_LENGTH 3
#define RESTORE_CURSOR_POSITION "\033[u"
#define RESTORE_CURSOR_POSITION_LENGTH 3
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
	DELETE_PREFIX = 5
};

enum ncsh_Hotkey ncsh_get_key(char character);

void ncsh_write(char* string, uint_fast32_t length);

void ncsh_print_prompt(struct ncsh_Directory prompt_info);

#endif // !ncsh_io_h

