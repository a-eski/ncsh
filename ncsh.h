#ifndef ncsh_h
#define ncsh_h

// loop variables in a struct optimized for memory layout, just an idea
// struct ncsh_Loop {
// 	bool reprint_prompt;
// 	uint_fast8_t command_result;
// 	enum ncsh_Hotkey key;
// 	uint_fast32_t history_position;
// 	char character;
// 	char buffer[MAX_INPUT];
// 	struct ncsh_Directory prompt_info;
// 	struct ncsh_Args args;
// 	struct ncsh_String history;
// };

int ncsh(void);

#endif // !ncsh_h

