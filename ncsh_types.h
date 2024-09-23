#ifndef ncsh_types_h
#define ncsh_types_h

#include <linux/limits.h>
#include <stdint.h>

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

