#include "../../src/arena.h"
#include "../../src/shell.h"

static inline void shell_init(Shell* rst shell, Arena* scratch)
{
    shell->arena = *scratch;
    shell->scratch_arena = *scratch;
    vars_malloc(&shell->arena, &shell->vars);
}
