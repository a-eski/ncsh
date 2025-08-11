#include "../../src/arena.h"
#include "../../src/env.h"
#include "../../src/types.h"

static inline void shell_init_no_vars(Shell* restrict shell, Arena* scratch, char** envp)
{
    shell->arena = *scratch;
    shell->scratch_arena = *scratch;
    env_new(shell, envp, &shell->arena);
}

static inline void shell_init(Shell* restrict shell, Arena* scratch, char** envp)
{
    shell_init_no_vars(shell, scratch, envp);
    vars_malloc(&shell->arena, &shell->vars);
}
