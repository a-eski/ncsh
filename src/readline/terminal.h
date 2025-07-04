/* Copyright ncsh (C) by Alex Eski 2024 */
/* terminal.h: deal with the underlying tty */

#pragma once

// input definitions
#define ESCAPE_CHARACTER 27 // "\033" or "^["
#define DOUBLE_QUOTE_KEY '\"'
#define BACKSPACE_KEY 127
#define TAB_KEY '\t'
// #define CTRL_CHARACTER '^'

// ctrl key combinations
#define CTRL_A 1
#define CTRL_B 2
#define CTRL_C 3 // '\003'
#define CTRL_D 4 // '\004'
#define CTRL_E 5
#define CTRL_F 6
#define CTRL_G 7
#define CTRL_H 8
#define CTRL_I 9
#define CTRL_J 10
#define CTRL_K 11
#define CTRL_L 12
#define CTRL_M 13
#define CTRL_N 14
#define CTRL_O 15
#define CTRL_P 16
#define CTRL_Q 17
#define CTRL_R 18
#define CTRL_S 19
#define CTRL_T 20
#define CTRL_U 21
#define CTRL_V 22
#define CTRL_W 23
#define CTRL_X 24
#define CTRL_Y 25
#define CTRL_Z 26

//  input definitions for multiple character inputs
//  definition only includes the final character.
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
#define ERASE_CURRENT_LINE_TO_LEFT "\033[1K"
#define ERASE_CURRENT_LINE_ALL "\033[2K"

#define ERASE_BELOW "\033[J"
#define ERASE_ABOVE "\033[1J"
#define ERASE_ALL "\033[2J"

#define CLEAR_SCREEN "\033[2J"

// terminal manipulations: cursor
#define MOVE_CURSOR_RIGHT "\033[1C"
#define MOVE_CURSOR_LEFT "\033[1D"
#define MOVE_CURSOR_UP "\033[1A"
#define MOVE_CURSOR_DOWN "\033[1B"

#define MOVE_CURSOR_START_OF_LINE_CHAR '\r'
#define MOVE_CURSOR_START_OF_LINE "\r"
#define MOVE_CURSOR_NEXT_LINE "\033[1E"
#define MOVE_CURSOR_PREVIOUS_LINE "\033[1F"

#define GET_CURSOR_POSITION "\033[6n"
#define SAVE_CURSOR_POSITION "\0337"
#define RESTORE_CURSOR_POSITION "\0338"

#define MOVE_CURSOR_HOME "\033[H"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

typedef struct {
    int x;
    int y;
} Coordinates;

Coordinates terminal_size(void);

/* terminal_init
 * Put the terminal in the proper modes for ncsh and save the previous state of the terminal in memory.
 * ncsh puts the shell in noncanonical mode, so input can be processed one character at a time.
 * Returns the current size of the terminal.
 */
Coordinates terminal_init(void);

/* terminal_reset
 * Put the terminal in the previous state of the terminal that is stored in memory.
 * Resets the terminal to the settings it had before ncsh was started.
 */
void terminal_reset(void);

// void terminal_move(int x, int y);
// void terminal_move_absolute(int x);
void terminal_move_up(int i);
void terminal_move_down(int i);
void terminal_move_right(int i);
void terminal_move_left(int i);
void terminal_move_to_end_of_previous_line();
void terminal_move_to_start_of_next_line();

// void terminal_line_insert(char* line);
// void terminal_line_delete(int line);
// void terminal_characters_delete(int i);
