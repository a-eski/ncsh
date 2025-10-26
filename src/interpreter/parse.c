/* Copyright ncsh (C) by Alex Eski 2025 */
/* parse.c: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../debug.h"
#include "lex.h"
#include "parse.h"
#include "parse_errors.h"

static size_t parser_state;

typedef struct {
    int parser_errno;
    char* msg;
} Parser_Internal;

/* enum Parser_State
 * Flags used by the lexer to keep track of state, like whether the current stream of tokens being processed are inside quotes or are a mathematical expression.
 */
// clang-format off
enum Parser_State: size_t {
    PS_NONE =                    0,
    IN_SINGLE_QUOTES =           1 << 0,
    IN_DOUBLE_QUOTES =           1 << 1,
    IN_BACKTICK_QUOTES =         1 << 2,
    IN_FOR_C_STYLE =             1 << 3,
};
// clang-format on

#define STMT_DEFAULT_N 25
#define STMT_MAX_N 100

void cmd_realloc_exact(Commands* restrict cmds, Arena* restrict scratch, size_t new_cap)
{
    size_t c = cmds->cap;
    cmds->cap = new_cap;
    cmds->strs =
        arena_realloc(scratch, new_cap, Str, cmds->strs, c);
    cmds->keys =
        arena_realloc(scratch, new_cap, Str, cmds->keys, c);
    cmds->ops =
        arena_realloc(scratch, new_cap, enum Ops, cmds->ops, c);
}

static Commands* cmds_alloc(Arena* restrict scratch)
{
    Commands* c = arena_malloc(scratch, 1, Commands);
    c->count = 0;
    c->cap = STMT_DEFAULT_N;
    c->strs = arena_malloc(scratch, STMT_DEFAULT_N, Str);
    c->keys = arena_malloc(scratch, STMT_DEFAULT_N, Str);
    c->ops = arena_malloc(scratch, STMT_DEFAULT_N, enum Ops);
    c->next = NULL;
    c->op = OP_NONE;
    c->prev_op = OP_NONE;
    return c;
}

static void cmd_realloc(Commands* restrict cmds, Arena* restrict scratch)
{
    size_t c = cmds->cap;
    size_t new_cap = c *= 2;
    cmds->cap = new_cap;
    cmds->strs =
        arena_realloc(scratch, new_cap, Str, cmds->strs, c);
    cmds->keys =
        arena_realloc(scratch, new_cap, Str, cmds->keys, c);
    cmds->ops =
        arena_realloc(scratch, new_cap, enum Ops, cmds->ops, c);
}

static Commands* cmd_next(Commands* restrict cmds, Arena* restrict scratch)
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

static Statement* stmt_alloc(Arena* restrict scratch)
{
    Statement* stmt = arena_malloc(scratch, 1, Statement);
    stmt->type = LT_NORMAL;
    stmt->commands = cmds_alloc(scratch);
    return stmt;
}

static int stmt_next(Parser_Data* restrict data, enum Logic_Type type)
{
    if (!data->stmts->head) {
        assert(data->cur_stmt);
        data->cur_stmt->type = type;
        data->stmts->head = data->cur_stmt;
        data->cur_stmt = stmt_alloc(data->s);
        data->prev_stmt = data->stmts->head;
        return EXIT_SUCCESS;
    }

    switch (type) {
    case LT_ELIF_CONDITIONS:
    case LT_ELSE:
        goto left;
    default:
        goto right;
    }

    unreachable();
    return EXIT_FAILURE_CONTINUE;

right:
    data->cur_stmt->type = type;
    data->cur_stmt->prev = data->prev_stmt;
    data->cur_stmt->prev->right = data->cur_stmt;
    data->prev_stmt = data->cur_stmt;
    data->cur_stmt = stmt_alloc(data->s);
    return EXIT_SUCCESS;

left:
    data->cur_stmt->type = type;
    data->cur_stmt->prev = data->prev_stmt->prev;
    data->cur_stmt->prev->left = data->cur_stmt;
    data->prev_stmt = data->cur_stmt;
    data->cur_stmt = stmt_alloc(data->s);
    return EXIT_SUCCESS;
}

static void cmd_stmt_next(Parser_Data* restrict data, enum Logic_Type type)
{
    data->cur_cmds->count = data->cur_cmds->pos == 0 ? 1 : data->cur_cmds->pos; // update last commands count

    stmt_next(data, type);

    data->cur_cmds = data->cur_stmt->commands;
}

static Parser_Output internal_to_output(Parser_Internal* internal)
{
    return (Parser_Output){
        .parser_errno = internal->parser_errno,
        .output.msg = internal->msg
    };
}

static bool consume(Lexemes* restrict lexemes, size_t* restrict n, enum Token expected)
{
    assert(*n < lexemes->count);
    if (*n < lexemes->count && lexemes->ops[*n] == expected) {
        debugf("consuming %s\n", lexemes->strs[*n].value);
        ++*n;
        return true;
    }
    else {
        debugf("ncsh parser: consumption failed. expected %d, actual %d\n", expected, lexemes->ops[*n]);
        return false;
    }
}

[[nodiscard("Whole point of peeking is to check the next op!")]]
enum Token peek(Lexemes* restrict lexemes, size_t n)
{
    if (n >= lexemes->count) {
        return T_NONE;
    }
    return lexemes->ops[n];
}

static void data_cmd_update(Parser_Data* restrict data, Str s, enum Ops op) {
    data->cur_cmds->strs[data->cur_cmds->pos] = s;
    data->cur_cmds->ops[data->cur_cmds->pos] = op;
    ++data->cur_cmds->pos;
}

static inline bool is_in_quotes()
{
    return parser_state & IN_SINGLE_QUOTES || parser_state & IN_DOUBLE_QUOTES || parser_state & IN_BACKTICK_QUOTES;
}

static void handle_quotes(Parser_Data* restrict data, enum Parser_State in_state, Str_Builder* restrict sb)
{
    if (!(parser_state & in_state)) {
        parser_state |= in_state;
    } else {
        parser_state &= ~in_state;

        data->cur_cmds->strs[data->cur_cmds->pos] = *sb_to_joined_str(sb, ' ', data->s);
        data->sb->n = 0; // zero out the string builder
        data->cur_cmds->ops[data->cur_cmds->pos] = OP_CONST;
        ++data->cur_cmds->pos;
    }
}

static bool is_end_of_stmt(enum Token op)
{
    switch (op) {
    case T_SEMIC:
    case T_ELIF:
    case T_ELSE:
    case T_C_BRACK: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static Parser_Internal parse_token(Parser_Data* restrict data, Lexemes* restrict lexemes, size_t* restrict i);

[[nodiscard]]
static Parser_Internal parse_cmds(Parser_Data* restrict data, size_t* restrict n)
{
    Parser_Internal rv;

    do {
        rv = parse_token(data, data->lexemes, n);
        if (rv.parser_errno)
            return rv;
    } while (!is_end_of_stmt(data->lexemes->ops[++*n]) && *n < data->lexemes->count - 1);

    if (*n > 0 && data->lexemes->ops[*n - 1] == T_C_BRACK)
        --*n;

    debug("commands processed");
    return (Parser_Internal){};
}

[[nodiscard]]
static Parser_Internal parse_conditions(Parser_Data* restrict data, size_t* restrict n, enum Logic_Type type)
{
    if (!consume(data->lexemes, n, T_O_BRACK)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing condition start '['."};
    }

    // support [ or [[ (bash uses [[)
    int double_bracks = 0;
    if (peek(data->lexemes, *n) == T_O_BRACK) {
        consume(data->lexemes, n, T_O_BRACK);
        ++double_bracks;
    }

    debug("processing conditions");
    Parser_Internal rv = parse_cmds(data, n);
    if (rv.parser_errno)
        return rv;

    consume(data->lexemes, n, T_SEMIC);

    cmd_stmt_next(data, type);
    if (!consume(data->lexemes, n, T_C_BRACK)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "found condition start '[', missing condition end ']'."};
    }

    // support ] or ]] (bash uses ]])
    if (peek(data->lexemes, *n) == T_C_BRACK) {
        consume(data->lexemes, n, T_C_BRACK);
        ++double_bracks;
    }

    if (double_bracks == 1) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "found mismatching brackets, i.e. '[[' for start condition and ']' for end."};
    }

    consume(data->lexemes, n, T_SEMIC);

    return (Parser_Internal){};
}

static Parser_Internal parse_if_stmts(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing if statements");
    Parser_Internal rv = parse_cmds(data, n);
    if (rv.parser_errno)
        return rv;

    consume(data->lexemes, n, T_SEMIC);

    cmd_stmt_next(data, LT_IF);
    return rv;
}

static Parser_Internal parse_else_stmts(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing else");

    if (!consume(data->lexemes, n, T_ELSE)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "expected 'else' but it was not found."};
    }

    Parser_Internal rv = parse_cmds(data, n);
    if (rv.parser_errno)
        return rv;

    consume(data->lexemes, n, T_SEMIC);

    cmd_stmt_next(data, LT_ELSE);
    return rv;
}

static Parser_Internal parse_elif_stmts(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing elif");

    if (!consume(data->lexemes, n, T_ELIF)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "expected 'elif' but it was not found."};
    }

    Parser_Internal rv = parse_conditions(data, n, LT_ELIF_CONDITIONS);
    if (rv.parser_errno) {
        Str* expanded_msg = estrcat(&Str_Get(rv.msg), &Str_Lit(" Failed parsing elif conditions."), data->s);
        rv.msg = expanded_msg && expanded_msg->value ? expanded_msg->value : rv.msg;
        return rv;
    }
    if (!consume(data->lexemes, n, T_THEN)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing 'then' after a condition."};
    }

    rv = parse_cmds(data, n);
    if (rv.parser_errno) {
        Str* expanded_msg = estrcat(&Str_Get(rv.msg), &Str_Lit(" Failed parsing elif commands."), data->s);
        rv.msg = expanded_msg && expanded_msg->value ? expanded_msg->value : rv.msg;
        return rv;
    }

    consume(data->lexemes, n, T_SEMIC);

    cmd_stmt_next(data, LT_ELIF);

    if (peek(data->lexemes, *n) == T_ELIF) {
        debug("found another elif to process");
        rv = parse_elif_stmts(data, n);
        if (rv.parser_errno)
            return rv;
    }

    return rv;
}

[[nodiscard]]
static Parser_Internal parse_if(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing if");

    if (!consume(data->lexemes, n, T_IF)) {
        debug("no OP_IF");
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, "didn't find 'if' where expected."};
    }

    Parser_Internal rv = parse_conditions(data, n, LT_IF_CONDITIONS);
    if (rv.parser_errno)
        return rv;
    if (!consume(data->lexemes, n, T_THEN)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing 'then' after a condition."};
    }

    rv = parse_if_stmts(data, n);
    if (rv.parser_errno) {
        Str* expanded_msg = estrcat(&Str_Get(rv.msg), &Str_Lit(" Couldn't process if statements."), data->s);
        rv.msg = expanded_msg && expanded_msg->value ? expanded_msg->value : rv.msg;
        return rv;
    }

    consume(data->lexemes, n, T_SEMIC);

    enum Token peeked = peek(data->lexemes, *n);
    if (peeked == T_FI) {
        consume(data->lexemes, n, T_FI);
        debug("returning success, OP_FI found, no else");
        data->stmts->type = ST_IF;
        return (Parser_Internal){};
    }

    if (peeked != T_ELSE && peeked != T_ELIF) {
        debug("no OP_ELSE or OP_ELIF");
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing, 'fi', 'else', or 'elif' where expected."};
    }

    if (peeked == T_ELSE) {
        data->stmts->type = ST_IF_ELSE;
        rv = parse_else_stmts(data, n);
    }
    else if (peeked == T_ELIF) {
        rv = parse_elif_stmts(data, n);
        if (rv.parser_errno)
            return rv;

        peeked = peek(data->lexemes, *n);
        if (peeked == T_ELSE) {
            data->stmts->type = ST_IF_ELIF_ELSE;
            rv = parse_else_stmts(data, n);
        }
        else {
            data->stmts->type = ST_IF_ELIF;
        }
    }

    if (rv.parser_errno)
        return rv;

    if (!consume(data->lexemes, n, T_FI)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing 'fi', no closing 'fi'."};
    }

    return rv;
}

[[nodiscard]]
static Parser_Internal parse_while_stmts(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing while statements");
    Parser_Internal rv = parse_cmds(data, n);
    if (rv.parser_errno)
        return rv;

    consume(data->lexemes, n, T_SEMIC);

    cmd_stmt_next(data, LT_WHILE);
    return rv;
}

[[nodiscard]]
static Parser_Internal parse_while(Parser_Data* restrict data, size_t* restrict n)
{
    if (data->cur_cmds->pos > 0) {
        cmd_stmt_next(data, LT_NORMAL);
    }
    data->stmts->type = ST_WHILE;
    consume(data->lexemes, n, T_WHILE);

    Parser_Internal rv = parse_conditions(data, n, LT_WHILE_CONDITIONS);
    if (rv.parser_errno) {
        Str* expanded_msg = estrcat(&Str_Get(rv.msg), &Str_Lit(" Failed parsing while conditions."), data->s);
        rv.msg = expanded_msg && expanded_msg->value ? expanded_msg->value : rv.msg;
        return rv;
    }

    assert(data->prev_stmt->type == LT_WHILE_CONDITIONS);
    Statement* conds = data->prev_stmt;

    consume(data->lexemes, n, T_DO);

    do {
        rv = parse_while_stmts(data, n);
        if (rv.parser_errno) {
            Str* expanded_msg = estrcat(&Str_Get(rv.msg), &Str_Lit(" Failed parsing while statements."), data->s);
            rv.msg = expanded_msg && expanded_msg->value ? expanded_msg->value : rv.msg;
            return rv;
        }
    } while (*n < data->lexemes->count && peek(data->lexemes, *n) != T_DONE);

    data_cmd_update(data, Str_Lit("JUMP"), OP_JUMP);
    cmd_stmt_next(data, LT_WHILE_CONDITIONS);
    data->cur_stmt = conds;
    cmd_stmt_next(data, LT_WHILE_CONDITIONS);

    if (!consume(data->lexemes, n, T_DONE))
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing 'done', no closing 'done' in while loop."};

    return (Parser_Internal){};
}

[[nodiscard]]
static Parser_Internal parse_for_stmts(Parser_Data* restrict data, size_t* restrict n, enum Logic_Type lt)
{
    debug("processing for statements");
    Parser_Internal rv;
    do {
        rv = parse_token(data, data->lexemes, n);
        if (rv.parser_errno)
            return rv;
    } while (data->lexemes->ops[++*n] != T_SEMIC && *n < data->lexemes->count - 1 && data->lexemes->ops[*n] != T_C_PARAN);

    if (rv.parser_errno)
        return rv;

    consume(data->lexemes, n, T_SEMIC);

    cmd_stmt_next(data, lt);
    return rv;
}

[[nodiscard]]
static Parser_Internal parse_for_increment(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing for statements");
    Parser_Internal rv;
    do {
        rv = parse_token(data, data->lexemes, n);
        if (rv.parser_errno)
            return rv;
    } while (data->lexemes->ops[++*n] != T_SEMIC && *n < data->lexemes->count - 1 && data->lexemes->ops[*n] != T_C_PARAN);

    if (rv.parser_errno)
        return rv;

    consume(data->lexemes, n, T_SEMIC);

    data->cur_cmds->count = data->cur_cmds->pos == 0 ? 1 : data->cur_cmds->pos; // update last commands count

    data->cur_stmt->type = LT_FOR_INCREMENT;
    data->for_data->increment = data->cur_stmt;
    data->cur_stmt = stmt_alloc(data->s);

    data->cur_cmds = data->cur_stmt->commands;
    return rv;
}

// for ((i = 1; i <= 5; i++)); do echo $i done
[[nodiscard]]
static Parser_Internal parse_for_c_style(Parser_Data* restrict data, size_t* restrict n)
{
    data->for_data = arena_malloc(data->s, 1, For_Data);

    consume(data->lexemes, n, T_O_PARAN);
    consume(data->lexemes, n, T_O_PARAN);

    Parser_Internal rv = parse_for_stmts(data, n, LT_FOR_INIT);
    if (rv.parser_errno)
        return rv;

    parser_state |= IN_FOR_C_STYLE;
    rv = parse_for_stmts(data, n, LT_FOR_CONDITIONS);
    if (rv.parser_errno)
        return rv;

    assert(data->prev_stmt->type == LT_FOR_CONDITIONS);
    Statement* conds = data->prev_stmt;

    rv = parse_for_increment(data, n);
    if (rv.parser_errno)
        return rv;

    parser_state &= ~IN_FOR_C_STYLE;

    consume(data->lexemes, n, T_C_PARAN);
    consume(data->lexemes, n, T_C_PARAN);
    consume(data->lexemes, n, T_SEMIC);
    consume(data->lexemes, n, T_DO);

    do {
        rv = parse_token(data, data->lexemes, n);
        if (rv.parser_errno)
            return rv;
    } while (data->lexemes->ops[++*n] != T_DONE && *n < data->lexemes->count - 1);
    cmd_stmt_next(data, LT_FOR);

    // set increment after the LT_FOR commands
    data->cur_stmt = data->for_data->increment;
    cmd_stmt_next(data, LT_FOR_INCREMENT);

    // then set to jump back to conditions, skipping init
    data_cmd_update(data, Str_Lit("JUMP"), OP_JUMP);
    cmd_stmt_next(data, LT_FOR_CONDITIONS);
    data->cur_stmt = conds;
    cmd_stmt_next(data, LT_FOR_CONDITIONS);

    if (!consume(data->lexemes, n, T_DONE))
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing 'done', no closing 'done' in for loop."};

    return (Parser_Internal){};
}

// for fruit in apple banana orange; do echo $fruit done
// for file in *; do echo $file done
[[nodiscard]]
static Parser_Internal parse_for_in(Parser_Data* restrict data, size_t* restrict n)
{
    data_cmd_update(data, data->lexemes->strs[*n++], OP_VARIABLE);

    if (!consume(data->lexemes, n, T_IN))
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing 'in', no joining 'in' in for loop."};

    Parser_Internal rv = parse_for_stmts(data, n, LT_FOR_VALUES);
    if (rv.parser_errno)
        return rv;

    consume(data->lexemes, n, T_DO);

    rv = parse_for_stmts(data, n, LT_FOR);
    if (rv.parser_errno)
        return rv;

    consume(data->lexemes, n, T_DONE);

    return (Parser_Internal){};
}

[[nodiscard]]
static Parser_Internal parse_for(Parser_Data* restrict data, size_t* restrict n)
{
    if (data->cur_cmds->pos > 0) {
        cmd_stmt_next(data, LT_NORMAL);
    }
    data->stmts->type = ST_FOR;
    consume(data->lexemes, n, T_FOR);

    Parser_Internal rv;
    if (peek(data->lexemes, *n) == T_O_PARAN)
        rv = parse_for_c_style(data, n);
    else
        rv = parse_for_in(data, n);

    return rv;
}

static Parser_Internal parse_pipe(Parser_Data* restrict data, Lexemes* restrict lexemes, size_t* restrict i)
{
    if (peek(lexemes, *i + 1) == T_PIPE) {
        ++*i;

        if (!(*i - 1))
            return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_OR_IN_FIRST_ARG};
        if (*i == lexemes->count - 1)
            return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_OR_IN_LAST_ARG};

        data->cur_cmds = cmd_next(data->cur_cmds, data->s);
        data->cur_cmds->prev_op = OP_OR;
        return (Parser_Internal){};
    }

    if (!*i)
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_PIPE_FIRST_ARG};
    if (*i == lexemes->count - 1)
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_PIPE_LAST_ARG};

    ++data->stmts->pipes_count;
    data->cur_cmds = cmd_next(data->cur_cmds, data->s);
    data->cur_cmds->prev_op = OP_PIPE;
    return (Parser_Internal){};
}

static Parser_Internal parse_amp(Parser_Data* restrict data, Lexemes* restrict lexemes, size_t* restrict i)
{
    enum Token peeked = peek(lexemes, *i + 1);
    size_t start_i = *i;

    if (peeked == T_GT) {
        ++*i;
        if (peek(lexemes, *i + 1) == T_GT) {
            data->stmts->redirect_type = RT_OUT_ERR_APPEND;
            ++*i;
        } else {
            data->stmts->redirect_type = RT_OUT_ERR;
        }

        if (!start_i) {
            char* msg = data->stmts->redirect_type == RT_OUT_ERR
                    ? INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG
                    : INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG;
            return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = msg};
        }
        if (*i == lexemes->count - 1) {
            char* msg = data->stmts->redirect_type == RT_OUT_ERR
                    ? INVALID_SYNTAX_STDERR_REDIR_LAST_ARG
                    : INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG;
            return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = msg};
        }

        data->stmts->redirect_filename = lexemes->strs[*i + 1].value;
        ++*i; // skip filename and redirect type, not needed in commands
        return (Parser_Internal){};
    }

    if (peeked == T_AMP) {
        ++*i;
        if (!start_i)
            return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_AND_IN_FIRST_ARG};
        if (*i == lexemes->count - 1)
            return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_AND_IN_LAST_ARG};

        data->cur_cmds = cmd_next(data->cur_cmds, data->s);
        data->cur_cmds->prev_op = OP_AND;
        return (Parser_Internal){};
    }

    if (!*i)
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG};
    if (*i != lexemes->count - 1)
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG};

    data->stmts->is_bg_job = true;
    return (Parser_Internal){};
}

static Parser_Internal parse_gt(Parser_Data* restrict data, Lexemes* restrict lexemes, size_t* restrict i)
{
    enum Token peeked = peek(lexemes, *i + 1);
    if (peeked == T_EQ) {
        consume(lexemes, i, T_LT);
        data->cur_cmds->op = OP_GE;
        data_cmd_update(data, Str_Lit(">="), OP_GE);
        return (Parser_Internal){};
    }
    if (parser_state & IN_FOR_C_STYLE) {
        data->cur_cmds->op = OP_GT;
        data_cmd_update(data, Str_Lit("<"), OP_GT);
        return (Parser_Internal){};
    }

    if (peeked == T_GT) {
        data->stmts->redirect_type = RT_OUT_APPEND;
        ++*i;
    } else {
        data->stmts->redirect_type = RT_OUT;
    }

    size_t start_i = *i;
    if (!start_i) {
        char* msg = data->stmts->redirect_type == RT_OUT
                ? INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG
                : INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG;
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = msg};
    }
    if (*i == lexemes->count - 1) {
        char* msg = data->stmts->redirect_type == RT_OUT
                ? INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG
                : INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG;
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = msg};
    }

    data->stmts->redirect_filename = lexemes->strs[*i + 1].value;
    ++*i; // skip filename and redirect type, not needed in commands
    return (Parser_Internal){};
}

static Parser_Internal parse_lt(Parser_Data* restrict data, Lexemes* restrict lexemes, size_t* restrict i)
{
    enum Token peeked = peek(lexemes, *i + 1);
    if (peeked == T_EQ) {
        consume(lexemes, i, T_LT);
        data->cur_cmds->op = OP_LE;
        data_cmd_update(data, Str_Lit("<="), OP_LE);
        return (Parser_Internal){};
    }
    if (parser_state & IN_FOR_C_STYLE) {
        data->cur_cmds->op = OP_LT;
        data_cmd_update(data, Str_Lit("<"), OP_LT);
        return (Parser_Internal){};
    }

    size_t start_i = *i;
    if (peeked == T_LT) {
        // TODO: switch this to the "here document operator", there is not actually stdin append
        data->stmts->redirect_type = RT_IN_APPEND;
        ++*i;
    } else {
        data->stmts->redirect_type = RT_IN;
    }

    if (!start_i)
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG};
    if (*i == lexemes->count - 1)
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = INVALID_SYNTAX_STDIN_REDIR_LAST_ARG};

    data->stmts->redirect_filename = lexemes->strs[*i + 1].value;
    ++*i; // skip filename and redirect type, not needed in commands
    return (Parser_Internal){};
}

static Parser_Internal parse_token(Parser_Data* restrict data, Lexemes* restrict lexemes, size_t* restrict i)
{
    enum Token peeked;
    enum Ops const_op = OP_CONST;

    switch (lexemes->ops[*i]) {
    case T_PIPE: {
        if (is_in_quotes())
            goto quoted;

        return parse_pipe(data, lexemes, i);
    }

    case T_AMP: {
        return parse_amp(data, lexemes, i);
    }

    case T_GT: {
        return parse_gt(data, lexemes, i);
    }

    case T_LT: {
        return parse_lt(data, lexemes, i);
    }

    case T_NUM: {
        const_op = OP_NUM;
        if (is_in_quotes())
            goto quoted;
        if (lexemes->strs[*i].length > 2 || *lexemes->strs[*i].value != '2')
            break;

        size_t start_i = *i;
        if (peek(lexemes, *i + 1) == T_GT) {
            ++*i;
            if (peek(lexemes, *i + 1) == T_GT) {
                data->stmts->redirect_type = RT_ERR_APPEND;
                ++*i;
            } else {
                data->stmts->redirect_type = RT_ERR;
            }
        } else {
            break;
        }

        if (!start_i) {
            char* msg = data->stmts->redirect_type == RT_ERR
                    ? INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG
                    : INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG;
            return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = msg};
        }
        if (*i == lexemes->count - 1) {
            char* msg = data->stmts->redirect_type == RT_ERR
                    ? INVALID_SYNTAX_STDERR_REDIR_LAST_ARG
                    : INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG;
            return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = msg};
        }

        data->stmts->redirect_filename = lexemes->strs[*i + 1].value;
        ++*i; // skip filename and redirect type, not needed in commands
        return (Parser_Internal){};
    }

    case T_EQ: {
        if (*i > 0 && lexemes->ops[*i - 1] == T_CONST) {
            peeked = peek(lexemes, *i + 1);
            if (peeked == T_CONST || peeked == T_NUM || peeked == T_QUOTE || peeked == T_D_QUOTE || peeked == T_BACKTICK || peeked == T_DOLLAR) {
                data->cur_cmds->op = OP_ASSIGNMENT;
                data_cmd_update(data, lexemes->strs[*i], OP_ASSIGNMENT);
                return (Parser_Internal){};
            }
        }
        break;
    }

    case T_D_QUOTE: {
        handle_quotes(data, IN_DOUBLE_QUOTES, data->sb);
        return (Parser_Internal){};
    }
    case T_QUOTE: {
        handle_quotes(data, IN_SINGLE_QUOTES, data->sb);
        return (Parser_Internal){};
    }
    case T_BACKTICK: {
        handle_quotes(data, IN_BACKTICK_QUOTES, data->sb);
        return (Parser_Internal){};
    }

    case T_DOLLAR: {
        peeked = peek(lexemes, *i + 1);
        if (peeked == T_CONST) {
            ++*i;
            data_cmd_update(data, lexemes->strs[*i], OP_VARIABLE);
            return (Parser_Internal){};
        }
        if (peeked == T_O_PARAN) {
            consume(lexemes, i, T_O_PARAN);
            ++*i;
            if (data->cur_cmds->pos > 0)
                data->cur_cmds = cmd_next(data->cur_cmds, data->s);
            data_cmd_update(data, Str_Lit("$("), OP_MATH_EXPR_START);
            return (Parser_Internal){};
        }

        break;
    }

    case T_HOME: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_HOME_EXPANSION);
        return (Parser_Internal){};
    }

    case T_GLOB: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_GLOB_EXPANSION);
        return (Parser_Internal){};
    }

    case T_TRUE: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_TRUE);
        return (Parser_Internal){};
    }

    case T_FALSE: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_FALSE);
        return (Parser_Internal){};
    }

    case T_IF: {
        if (is_in_quotes())
            goto quoted;

        Parser_Internal rv = parse_if(data, i);
        if (rv.parser_errno)
            return rv;

        return (Parser_Internal){};
    }

    case T_WHILE: {
        if (is_in_quotes())
            goto quoted;

        Parser_Internal rv = parse_while(data, i);
        if (rv.parser_errno)
            return rv;

        return (Parser_Internal){};
    }

    case T_FOR: {
        if (is_in_quotes())
            goto quoted;

        Parser_Internal rv = parse_for(data, i);
        if (rv.parser_errno)
            return rv;

        return (Parser_Internal){};
    }
    case T_IN: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_IN);
        return (Parser_Internal){};
    }

    case T_EQ_A: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_EQ_A);
        data->cur_cmds->prev_op = OP_EQ_A;
        return (Parser_Internal){};
    }
    case T_LT_A: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_LT_A);
        data->cur_cmds->prev_op = OP_LT_A;
        return (Parser_Internal){};
    }
    case T_LE_A: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_LE_A);
        data->cur_cmds->prev_op = OP_LE_A;
        return (Parser_Internal){};
    }
    case T_GT_A: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_GT_A);
        data->cur_cmds->prev_op = OP_GT_A;
        return (Parser_Internal){};
    }
    case T_GE_A: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_GE_A);
        data->cur_cmds->prev_op = OP_GE_A;
        return (Parser_Internal){};
    }

    case T_PLUS: {
        if (is_in_quotes())
            goto quoted;

        if (peek(lexemes, *i + 1) == T_PLUS) {
            ++*i;
            data->cur_cmds->op = OP_INCREMENT;
            data_cmd_update(data, Str_Lit("++"), OP_INCREMENT);
            return (Parser_Internal){};
        }

        data_cmd_update(data, lexemes->strs[*i], OP_ADD);
        data->cur_cmds->prev_op = OP_ADD;
        return (Parser_Internal){};
    }
    case T_MINUS: {
        if (is_in_quotes())
            goto quoted;

        if (peek(lexemes, *i + 1) == T_MINUS) {
            ++*i;
            data->cur_cmds->op = OP_DECREMENT;
            data_cmd_update(data, Str_Lit("--"), OP_DECREMENT);
            return (Parser_Internal){};
        }

        data_cmd_update(data, lexemes->strs[*i], OP_SUB);
        data->cur_cmds->prev_op = OP_SUB;
        return (Parser_Internal){};
    }
    case T_MOD: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_MOD);
        data->cur_cmds->prev_op = OP_MOD;
        return (Parser_Internal){};
    }
    case T_FSLASH: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_DIV);
        data->cur_cmds->prev_op = OP_DIV;
        return (Parser_Internal){};
    }
    case T_STAR: {
        if (is_in_quotes())
            goto quoted;

        if (peek(lexemes, *i + 1) == T_STAR) {
            consume(lexemes, i, T_STAR);
            data_cmd_update(data, Str_Lit("**"), OP_EXP);
            data->cur_cmds->prev_op = OP_EXP;
            return (Parser_Internal){};
        }

        data_cmd_update(data, lexemes->strs[*i], OP_MUL);
        data->cur_cmds->prev_op = OP_MUL;
        return (Parser_Internal){};
    }
    case T_C_PARAN: {
        if (is_in_quotes())
            goto quoted;

        data_cmd_update(data, lexemes->strs[*i], OP_MATH_EXPR_END);
        return (Parser_Internal){};
    }

    default: {
    quoted:
        if (is_in_quotes()) {
            sb_add(&lexemes->strs[*i], data->sb, data->s);
            return (Parser_Internal){};
        }
        break;
    }
    }

    if (const_op == OP_CONST && parser_state & IN_FOR_C_STYLE) {
        data_cmd_update(data, lexemes->strs[*i], OP_VARIABLE);
        return (Parser_Internal){};
    }

    data_cmd_update(data, lexemes->strs[*i], const_op);
    return (Parser_Internal){};
}

[[nodiscard]]
Parser_Output parse(Lexemes* restrict lexemes, Arena* restrict scratch)
{
    assert(lexemes); assert(scratch);
    if (!lexemes->count) {
        return (Parser_Output){
            .parser_errno = PE_NOTHING
        };
    }

    Parser_Data data = {
        .lexemes = lexemes,
        .stmts = arena_malloc(scratch, 1, Statements),
        .cur_stmt = stmt_alloc(scratch),
        .s = scratch,
        .sb = sb_new(scratch),
        .cur_cmds = data.cur_stmt->commands
    };
    parser_state = 0;
    Parser_Internal rv;

    for (size_t i = 0; i < lexemes->count; ++i) {
        if (data.cur_cmds->pos >= data.cur_cmds->cap - 1) {
            cmd_realloc(data.cur_cmds, scratch);
        }

        rv = parse_token(&data, lexemes, &i);
        if (rv.parser_errno)
            return internal_to_output(&rv);
    }

    if (data.cur_cmds->pos > 0) {
        data.cur_cmds->count = data.cur_cmds->pos;
    }

    if (!data.stmts->head) {
        data.stmts->head = data.cur_stmt;
    }

    // no branch is fine, this value not used unless pipes are present.
    ++data.stmts->pipes_count;

    return (Parser_Output){.output.stmts = data.stmts};
}
