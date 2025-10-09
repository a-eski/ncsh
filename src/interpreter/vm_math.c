/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_math.c: math processing */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../defines.h"
#include "../signals.h"
#include "../eskilib/str.h"
#include "../ttyio/ttyio.h"
#include "stmts.h"
#include "vm_types.h"

/* vm_math_condition
 * Processes math conditions and sets vm->status with result of operation
 */
void vm_math_condition(Vm_Data* restrict vm)
{
    if (!vm->cmds || !vm->cmds->strs || !vm->cmds->strs[0].value || !vm->cmds->strs[2].value) {
        vm->status = EXIT_FAILURE_CONTINUE;
        return;
    }

    Str s1 = vm->cmds->strs[0];
    enum Ops op = vm->cmds->ops[1];
    Str s2 = vm->cmds->strs[2];
    assert(s1.value); assert(s2.value);

    bool result;
    switch (op) {
    case OP_EQUALS: {
        result = numeq(estrtonum(s1), estrtonum(s2));
        break;
    }
    case OP_LESS_THAN: {
        result = numlt(estrtonum(s1), estrtonum(s2));
        break;
    }
    case OP_LESS_THAN_OR_EQUALS: {
        result = numle(estrtonum(s1), estrtonum(s2));
        break;
    }
    case OP_GREATER_THAN: {
        result = numgt(estrtonum(s1), estrtonum(s2));
        break;
    }
    case OP_GREATER_THAN_OR_EQUALS: {
        result = numge(estrtonum(s1), estrtonum(s2));
        break;
    }
    default: {
        tty_fprintln(stderr, "ncsh: while trying to process 'if' logic, found unsupported operation '%d'.", vm->op_current);
        result = false;
        break;
    }
    }

    vm->status = result ? EXIT_SUCCESS : EXIT_FAILURE;
}

typedef struct {
    size_t n;
    enum {
        M_OP,
        M_NUM
    } type;
    union {
        enum Ops op;
        Num num;
    } val;
} Expr;

/* vm_math_expr
 * Processes math operations in place in an array of Expr.
 * Example:
    PARAN, EXP, MULT, DIV, ADD, SUB
    1 + 1 - 1 * 1 / 1 % 1
    1 + 1 - 1 / 1 % 1
    1 + 1 - 1 % 1
    1 + 1 - 1
    2 - 1
    1
 */
Str vm_math_expr(Vm_Data* restrict vm)
{
#ifdef NCSH_DEBUG
    char** buf = vm->buffer;
    while (*buf && printf("%s\n", *buf) && ++buf);
#endif

    Commands* cmds = vm->cmds;
    constexpr size_t start = 1;
    if (cmds->ops[0] != OP_MATH_EXPR_START || cmds->ops[1] != OP_NUM) {
        if (!cmds->next || cmds->next->ops[0] != OP_MATH_EXPR_START || cmds->next->ops[1] != OP_NUM) {
            tty_fputs("ncsh: unable to process math expression.", stderr);
            return Str_Empty;
        }
        cmds = cmds->next;
    }

    // Setup
    Expr* exs = arena_malloc(vm->s, cmds->count, Expr);
    exs->n = cmds->count;
    for (size_t i = start; i < exs->n; ++i) {
        if (cmds->ops[i] == OP_NUM) {
            exs[i] = (Expr){.type = M_NUM, .val.num = estrtonum(cmds->strs[i])};
            continue;
        }
        if (cmds->ops[i] == OP_MATH_EXPR_END)
            break;
        exs[i] = (Expr){.type = M_OP, .val.op = cmds->ops[i]};
    }

    // Operations
    // TODO: parans support

    for (size_t i = start; i < exs->n; ++i) {
        if (exs[i].type == M_OP && exs[i].val.op == OP_EXP) {
            int rv = numpowi(exs[i - 1].val.num, exs[i + 1].val.num);
            // int rv = exs[i - 1].val.num.value.i * exs[i - 1].val.num.value.i * exs[i + 1].val.num.value.i;
            exs[i - 1] = (Expr){.type = M_NUM, .val.num = (Num){.type = N_INT, .value.i = rv}};
            if (exs->n > 3) {
                memmove(exs + i, exs + i + 2, sizeof(Expr) * (exs->n - i + 1));
            }
            exs->n -= 2;
            i = start;
        }
    }

    if (exs->n == 1)
        return *numtostr((Num){.type = N_INT, .value.i = exs[1].val.num.value.i}, vm->s);

    for (size_t i = start; i < exs->n; ++i) {
        if (exs[i].type == M_OP && exs[i].val.op == OP_MUL) {
            int rv = exs[i - 1].val.num.value.i * exs[i + 1].val.num.value.i;
            exs[i - 1] = (Expr){.type = M_NUM, .val.num = (Num){.type = N_INT, .value.i = rv}};
            if (exs->n > 3) {
                memmove(exs + i, exs + i + 2, sizeof(Expr) * (exs->n - i + 1));
            }
            exs->n -= 2;
            i = start;
        }
    }

    if (exs->n == 1)
        return *numtostr((Num){.type = N_INT, .value.i = exs[1].val.num.value.i}, vm->s);

    for (size_t i = start; i < exs->n; ++i) {
        if (exs[i].type == M_OP && exs[i].val.op == OP_DIV) {
            if (exs[i + 1].val.num.value.i == 0)
                return Str_Empty;

            int rv = exs[i - 1].val.num.value.i / exs[i + 1].val.num.value.i;
            exs[i - 1] = (Expr){.type = M_NUM, .val.num = (Num){.type = N_INT, .value.i = rv}};
            if (exs->n > 3) {
                memmove(exs + i, exs + i + 2, sizeof(Expr) * (exs->n - i + 1));
            }
            exs->n -= 2;
            i = start;
        }
    }

    if (exs->n == 1)
        return *numtostr((Num){.type = N_INT, .value.i = exs[1].val.num.value.i}, vm->s);

    for (size_t i = start; i < exs->n; ++i) {
        if (exs[i].type == M_OP && exs[i].val.op == OP_ADD) {
            int rv = exs[i - 1].val.num.value.i + exs[i + 1].val.num.value.i;
            exs[i - 1] = (Expr){.type = M_NUM, .val.num = (Num){.type = N_INT, .value.i = rv}};
            if (exs->n > 3) {
                memmove(exs + i, exs + i + 2, sizeof(Expr) * (exs->n - i + 1));
            }
            exs->n -= 2;
            i = start;
        }
    }

    if (exs->n == 1)
        return *numtostr((Num){.type = N_INT, .value.i = exs[1].val.num.value.i}, vm->s);

    for (size_t i = start; i < exs->n; ++i) {
        if (exs[i].type == M_OP && exs[i].val.op == OP_SUB) {
            int rv = exs[i - 1].val.num.value.i - exs[i + 1].val.num.value.i;
            exs[i - 1] = (Expr){.type = M_NUM, .val.num = (Num){.type = N_INT, .value.i = rv}};
            if (exs->n > 3) {
                memmove(exs + i, exs + i + 2, sizeof(Expr) * (exs->n - i + 1));
            }
            exs->n -= 2;
            i = start;
        }
    }

    return *numtostr((Num){.type = N_INT, .value.i = exs[1].val.num.value.i}, vm->s);
}
