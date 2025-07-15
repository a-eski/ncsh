/* Copyright ncsh (C) by Alex Eski 2025 */
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
