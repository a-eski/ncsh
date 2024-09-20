#ifndef shl_debug_h
#define shl_debug_h

#include <stdio.h>

#include "shl_commands.h"

void shl_debug_line(struct shl_Line line);
void shl_debug_args(struct shl_Args args);
void shl_debug_launch_process(struct shl_Args args);

#endif // !shl_debug_h

