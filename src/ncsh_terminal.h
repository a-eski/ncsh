// Copyright (c) ncsh by Alex Eski 2024

#ifndef NCSH_TERMINAL_H_
#define NCSH_TERMINAL_H_

#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

// input definitions
#define ESCAPE_CHARACTER 27 // "\033" or "^["
#define DOUBLE_QUOTE_KEY '\"'
#define CTRL_C 3 // '\003'
#define CTRL_D 4 // '\004'
#define CTRL_U 21
#define CTRL_W 23
#define BACKSPACE_KEY 127
#define TAB_KEY '\t'

// input definitions for multiple character inputs
#define UP_ARROW 'A'         // "\033[[A" or "^[[A"
#define DOWN_ARROW 'B'       // "\033[[B" or "^[[B"
#define RIGHT_ARROW 'C'      // "\033[[C" or "^[[C"
#define LEFT_ARROW 'D'       // "\033[[D" or "^[[D"
#define DELETE_PREFIX_KEY 51 // 3
#define DELETE_KEY '~'       // "\033[[3~" or "^[[3~"
#define HOME_KEY 'H'         // "\033[[H" or "^[[H"
#define END_KEY 'F'          // "\033[[F" or "^[[F"

// terminal manipulations: io
#define BACKSPACE_STRING "\b \b"
#define DELETE_STRING " \b"

#define ERASE_CURRENT_LINE "\033[K"

#define CLEAR_SCREEN "\033[2J"

// terminal manipulations: cursor position
#define MOVE_CURSOR_RIGHT "\033[1C"
#define MOVE_CURSOR_LEFT "\033[1D"
#define MOVE_CURSOR_UP "\033[1A"
#define MOVE_CURSOR_DOWN "\033[1B"
#define GET_CURSOR_POSITION "\033[6n"
#define SAVE_CURSOR_POSITION "\033[s"
#define RESTORE_CURSOR_POSITION "\033[u"
#define MOVE_CURSOR_HOME "\033[H"

struct ncsh_Coordinates {
    int x;
    int y;
};

// Put the terminal in the previous state of the terminal that is stored in memory.
void ncsh_terminal_reset(void);

// Put the terminal in the proper modes for ncsh and save the previous state of the terminal in memory.
void ncsh_terminal_init(void);

void ncsh_terminal_move(int x, int y);
void ncsh_terminal_move_up(int i);
void ncsh_terminal_move_down(int i);
void ncsh_terminal_move_right(int i);
void ncsh_terminal_move_left(int i);

void ncsh_terminal_size_set(void);
struct ncsh_Coordinates ncsh_terminal_size_get(void);
int ncsh_terminal_size_x(void);
int ncsh_terminal_size_y(void);

// Get the length of the current prompt.
size_t ncsh_terminal_prompt_size(size_t user_len, size_t dir_len);

// can cause crashes when you paste in entries and it tries to get cursor position.
// need to figure out an alternative.
struct ncsh_Coordinates ncsh_terminal_position(void);

#endif // !NCSH_TERMINAL_H_
