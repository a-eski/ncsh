#include "../../src/arena.h"
#include "../../src/env.h"
#include "../../src/types.h"

static inline void shell_init(Shell* restrict shell, Arena* scratch, char** envp)
{
    shell->arena = *scratch;
    shell->scratch_arena = *scratch;
    env_new(shell, envp, &shell->arena);
}
