#ifndef shl_types_h
#define shl_types_h

#include <linux/limits.h>
#include <stdint.h>

struct shl_Args
{
	uint_fast32_t count;
	uint_fast32_t maxLineSize;
	char** lines;
};

struct shl_Directory
{
	char* user;
	char path[PATH_MAX];
};

enum shl_Hotkey
{
	NONE = 0,
	UP = 1,
	DOWN = 2,
	LEFT = 3,
	RIGHT = 4,
	DELETE = 5
};

#endif // !shl_types_h

