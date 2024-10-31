// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_terminal_h
#define ncsh_terminal_h

struct ncsh_Coordinates {
	int x;
	int y;
};

void ncsh_terminal_reset(void);

void ncsh_terminal_init(void);

void ncsh_terminal_move(int x, int y);

struct ncsh_Coordinates ncsh_terminal_size();

struct ncsh_Coordinates ncsh_terminal_position();

#endif // !ncsh_terminal_h

