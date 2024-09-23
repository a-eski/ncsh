#include <stdlib.h>

#include "ncsh_args.h"

bool ncsh_args_is_valid(struct ncsh_Args args) {
	if (args.count == 0)
		return false;
	else if (args.values == NULL)
		return false;
	else if (args.values[0] == NULL)
		return false;
	else
		return true;
}

void ncsh_args_free(struct ncsh_Args args) {
	for (uint_fast32_t i = 0; i < args.count; i++)
		free(args.values[i]);

	free(args.values);
}
