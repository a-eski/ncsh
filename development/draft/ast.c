#include <stdint.h>
#include <stdlib.h>

#include "../eskilib/str.h"
#include "lexemes.h"
#include "../types.h"

#define STMT_DEFAULT_N 10
#define STMT_MAX_N 40

enum Ops : uint8_t {
    // Default value, indicative of an issue parsing when found during execution
    OP_NONE = 0,
    // Cmdsant value
    OP_CONST = 1,
    // Shell operators
    OP_PIPE = 2,                                  // |
    OP_STDOUT_REDIRECTION = 3,                    // >
    OP_STDOUT_REDIRECTION_APPEND = 4,             // >>
    OP_STDIN_REDIRECTION = 5,                     // <
    OP_STDIN_REDIRECTION_APPEND = 6,              // <<
    OP_STDERR_REDIRECTION = 7,                    // 2>
    OP_STDERR_REDIRECTION_APPEND = 8,             // 2>>
    OP_STDOUT_AND_STDERR_REDIRECTION = 9,         // &>
    OP_STDOUT_AND_STDERR_REDIRECTION_APPEND = 10, // &>>
    OP_BACKGROUND_JOB = 11,                       // &
    OP_AND = 12,                                  // &&
    OP_OR = 13,                                   // ||
    // Math (only implemented in parser)
    OP_ADD = 14,                   // +
    OP_SUBTRACT = 15,              // -
    OP_MULTIPLY = 16,              // *
    OP_DIVIDE = 17,                // /
    OP_MODULO = 18,                // %
    OP_EXPONENTIATION = 19,        // **
    OP_MATH_EXPRESSION_START = 20, // $(
    OP_MATH_EXPRESSION_END = 21,   // )
    // Variables
    OP_VARIABLE = 22,   // a variable value (starting with $, $VAR)
    OP_ASSIGNMENT = 23, // assigning a variable (var=val)
    // Boolean
    OP_TRUE = 24,  // true
    OP_FALSE = 25, // false
    // Expansion
    OP_HOME_EXPANSION = 26, // ~
    OP_GLOB_EXPANSION = 27, // * or ?
    // Control flow structures
    OP_CONDITION_START = 28,
    OP_CONDITION_END = 29,
    OP_IF = 30,
    OP_ELSE = 31,
    OP_ELIF = 32,
    OP_THEN = 33,
    OP_FI = 34,
    // Equality comparisons
    OP_EQUALS = 35,
    OP_LESS_THAN = 36,
    OP_GREATER_THAN = 37,
};

typedef struct Cmds_ {
    size_t pos;
    size_t count;
    size_t cap;
    enum Ops prev_op;

    enum Ops* ops;
    Str* strs;

    struct Cmds_* next;
} Cmds;


Cmds* cmds_alloc(Arena* restrict scratch)
{
    Cmds* c = arena_malloc(scratch, 1, Cmds);
    c->count = 0;
    c->cap = STMT_DEFAULT_N;
    c->strs = arena_malloc(scratch, STMT_DEFAULT_N, Str);
    c->ops = arena_malloc(scratch, STMT_DEFAULT_N, enum Ops);
    c->next = NULL;
    c->prev_op = OP_NONE;
    return c;
}

void cmd_realloc(Cmds* restrict cmds, Arena* restrict scratch)
{
    size_t c = cmds->cap;
    size_t new_cap = c *= 2;
    cmds->cap = new_cap;
    cmds->strs =
        arena_realloc(scratch, new_cap, Str, cmds->strs, c);
    cmds->ops =
        arena_realloc(scratch, new_cap, enum Ops, cmds->ops, c);
}

void cmds_realloc(Cmds* restrict cmds, Arena* restrict scratch)
{
    size_t c = cmds->cap;
    size_t new_cap = c *= 2;
    cmds->cap = new_cap;
    cmds->strs =
        arena_realloc(scratch, new_cap, Str, cmds->strs, c);
    cmds->ops =
        arena_realloc(scratch, new_cap, enum Ops, cmds->ops, c);
}

Cmds* cmd_next(Cmds* restrict cmds, Arena* restrict scratch)
{
    if (!cmds->pos) {
        cmds->count = 1;
        cmds->strs[1].value = NULL;
    }
    else {
        cmds->count = cmds->pos;
    }

    cmds->next = cmds_alloc(scratch);
    cmds->pos = 0;

    cmds = cmds->next;
    cmds->pos = 0;
    return cmds;
}

typedef struct Ast Ast;

struct Ast {
    enum Ops type;

    union {
        struct Ast_Err { int errno; Str* msg; } Err;
        struct Ast_None { int n; } None;
        struct Ast_Const { Str* str; Ast* right; } Const;
        struct Ast_Redir { Str* file; } Redir;

        struct Ast_Number { int n; } Number;
        struct Ast_Eql { Ast* left; Ast* right; } Eql;
        struct Ast_Lt { Ast* left; Ast* right; } Lt;
        struct Ast_Gt { Ast* left; Ast* right; } Gt;
        struct Ast_Add { Ast* left; Ast* right; } Add;
        struct Ast_Sub { Ast* left; Ast* right; } Sub;
        struct Ast_Mul { Ast* left; Ast* right; } Mul;
        struct Ast_Div { Ast* left; Ast* right; } Div;
        struct Ast_Mod { Ast* left; Ast* right; } Mod;
        struct Ast_Exp { Ast* left; Ast* right; } Exp;

        struct Ast_If { Cmds* if_conds; Ast* elif_conds; Ast* if_body; } If;
        struct Ast_If_Body { Cmds* cmds; } If_Body;
        struct Ast_Elif { Cmds* conds; Ast* else_body; Ast* elif_body; } Elif;
        struct Ast_Elif_Body { Cmds* cmds; } Elif_Body;
        struct Ast_Else { Cmds* conds; } Else;
    } data;
};

typedef struct Ast_Err Ast_Err; typedef struct Ast_None Ast_None; typedef struct Ast_Const Ast_Const;
typedef struct Ast_Redir Ast_Redir; typedef struct Ast_Number Ast_Number; typedef struct Ast_Eql Ast_Eql;
typedef struct Ast_Lt Ast_Lt; typedef struct Ast_Gt Ast_Gt; typedef struct Ast_Add Ast_Add;
typedef struct Ast_Sub Ast_Sub; typedef struct Ast_Mul Ast_Mul; typedef struct Ast_Div Ast_Div;
typedef struct Ast_Mod Ast_Mod; typedef struct Ast_Exp Ast_Exp; typedef struct Ast_If Ast_If;
typedef struct Ast_Elif Ast_Elif; typedef struct Ast_Else Ast_Else;

typedef struct {
    size_t pos;   // pos
    size_t n;   // number/count
    size_t cap;   // cap
    Ast* a;     // data
} Asts;

static inline Asts* asts_alloc(Arena* restrict scratch)
{
    Asts* asts = arena_malloc(scratch, 1, Asts);
    constexpr size_t c = 10;
    asts->cap = c;
    asts->a = arena_malloc(scratch, c, Ast);
    return asts;
}

static inline Asts* asts_realloc(Asts* restrict asts, Arena* restrict scratch)
{
    size_t c = asts->cap;
    size_t new_cap = c *= 2;
    asts->cap = new_cap;
    asts->a =
        arena_realloc(scratch, new_cap, Ast, asts->a, c);
    return asts;
}

static void parse_const(Ast* restrict a, Asts* restrict asts, Lexemes* restrict lexemes, size_t* n, Arena* restrict scratch)
{
    if (*n >= asts->cap - 1)
        asts = asts_realloc(asts, scratch);


    a = !a ? arena_malloc(scratch, 1, Ast) : a;
    a->type = OP_CONST;
    a->data.Const = (Ast_Const){.str = &lexemes->strs[*n]};
    if (*n < lexemes->count && lexemes->ops[++*n] == OP_CONST)
        parse_const(a->data.Const.right, asts, lexemes, n, scratch);
}

Asts* parser_parse(Lexemes* restrict lexemes, [[maybe_unused]] Shell* restrict shell, Arena* restrict scratch)
{
    Asts* asts = asts_alloc(scratch);

    /*if (lexemes->ops[0] == OP_CONSTANT) {
        // TODO: check aliases in certain other conditions, like after && or ||.
        expansion_alias(lexemes, 0, scratch);
    }*/

    Ast* a = arena_malloc(scratch, 1, Ast);
    for (size_t i = 0; i < lexemes->count; ++i) {
        switch (lexemes->ops[i]) {
            default: {
                parse_const(a, asts, lexemes, &i, scratch);
                break;
            }
        }
    }

    return asts;
}
