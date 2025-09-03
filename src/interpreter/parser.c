/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.c: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../debug.h"
#include "expansions.h"
#include "lexemes.h"
#include "ops.h"
#include "parser.h"
#include "stmts.h"

typedef struct {
    int parser_errno;
    char* msg;
} Parser_Internal;

Parser_Output parser_internal_to_ouput(Parser_Internal* internal)
{
    return (Parser_Output){
        .parser_errno = internal->parser_errno,
        .output.msg = internal->msg
    };
}

bool parser_consume(Lexemes* restrict lexemes, size_t* restrict n, enum Ops expected)
{
    assert(*n < lexemes->count);
    if (*n < lexemes->count && lexemes->ops[*n] == expected) {
        ++*n;
        return true;
    }
    else {
        debugf("ncsh parser: consumption failed. expected %d, actual %d\n", expected, lexemes->ops[*n]);
        return false;
    }
}

[[nodiscard("Whole point of peeking is to check the next op!")]]
enum Ops parser_peek(Lexemes* restrict lexemes, size_t n)
{
    if (n >= lexemes->count) {
        return OP_NONE;
    }
    return lexemes->ops[n];
}

[[nodiscard]]
bool parser_is_valid_statement(enum Ops op)
{
    switch (op) {
    case OP_CONST:
    case OP_TRUE:
    case OP_FALSE:
    case OP_EQUALS:
    case OP_AND:
    case OP_OR:
    case OP_LESS_THAN:
    case OP_VARIABLE:
    case OP_GREATER_THAN: {
        return true;
    }
    default: {
        return false;
    }
    }
}

Parser_Internal parser_cmds_process(Parser_Data* data, size_t* restrict n)
{
    if (!parser_is_valid_statement(parser_peek(data->lexemes, *n))) {
        return (Parser_Internal){.parser_errno = PE_INVALID_STMT, .msg = "no valid statement where one was expected."};
    }

    Lexemes* lexemes = data->lexemes;

    do {
        bool end_of_statement = false;
        size_t len = lexemes->strs[*n].length;
        if (len > 2 && lexemes->strs[*n].value[len - 2] == ';') {
            lexemes->strs[*n].value[len - 2] = '\0';
            --lexemes->strs[*n].length;
            end_of_statement = true;
        }

        switch (lexemes->ops[*n]) {
        case OP_HOME_EXPANSION:
            expansion_home(data, *n);
            break;
        case OP_EQUALS:
        case OP_LESS_THAN:
        case OP_GREATER_THAN: {
            data->cur_cmds->prev_op = lexemes->ops[*n];
            break;
        }
        case OP_VARIABLE:
            expansion_variable(data, &lexemes->strs[*n]);
            ++*n;
            continue;
        case OP_GLOB_EXPANSION:
            expansion_glob(lexemes->strs[*n].value, data->cur_cmds, data->s);
            continue;
        case OP_PIPE:
            ++data->stmts->pipes_count;
            data->cur_cmds = cmd_next(data->cur_cmds, data->s);
            data->cur_cmds->prev_op = OP_PIPE;
            continue;
        case OP_ASSIGNMENT:
            expansion_assignment(lexemes, *n, data->sh);
            if (*n + 1 < lexemes->count && (lexemes->ops[*n + 1] == OP_AND || lexemes->ops[*n + 1] == OP_OR)) {
                ++*n; // skip || or && on assignment, assigment not included in commands
            }
            continue;
        case OP_AND:
        case OP_OR: {
            data->cur_cmds = cmd_next(data->cur_cmds, data->s);
            data->cur_cmds->prev_op = lexemes->ops[*n];
            ++*n;
            continue;
        }
        }

        data->cur_cmds->strs[data->cur_cmds->pos] = lexemes->strs[*n];
        data->cur_cmds->ops[data->cur_cmds->pos] = lexemes->ops[*n];
        ++data->cur_cmds->pos;
        ++*n;

        if (end_of_statement) {
            data->cur_cmds = cmd_next(data->cur_cmds, data->s);
            debug("end of statement");
        }
    } while (parser_is_valid_statement(parser_peek(lexemes, *n)));

    debug("commands processed");
    return (Parser_Internal){.parser_errno = 0};
}

Parser_Internal parser_conditions(Parser_Data* data, size_t* restrict n, enum Logic_Type type)
{
    if (!parser_consume(data->lexemes, n, OP_CONDITION_START)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing condition start '['."};
    }

    debug("processing conditions");
    Parser_Internal rv = parser_cmds_process(data, n);
    if (rv.parser_errno) {
        return rv;
    }

    cmd_stmt_next(data, type);
    if (!parser_consume(data->lexemes, n, OP_CONDITION_END)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "found condition start '[', missing condition end ']'."};
    }

    if (!parser_consume(data->lexemes, n, OP_THEN)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing 'then' after a condition."};
    }

    return (Parser_Internal){.parser_errno = 0};
}

Parser_Internal parser_if_statements(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing if statements");
    Parser_Internal rv = parser_cmds_process(data, n);
    if (rv.parser_errno) {
        return rv;
    }

    cmd_stmt_next(data, LT_IF);
    return rv;
}

Parser_Internal parser_else_statements(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing else");

    if (!parser_consume(data->lexemes, n, OP_ELSE)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "expected 'else' but it was not found."};
    }

    Parser_Internal rv = parser_cmds_process(data, n);
    if (rv.parser_errno) {
        return rv;
    }

    cmd_stmt_next(data, LT_ELSE);
    return rv;
}

Parser_Internal parser_elif_statements(Parser_Data* restrict data, size_t* restrict n)
{
    debug("processing elif");

    if (!parser_consume(data->lexemes, n, OP_ELIF)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "expected 'elif' but it was not found."};
    }

    Parser_Internal rv = parser_conditions(data, n, LT_ELIF_CONDITIONS);
    if (rv.parser_errno) {
        Str* expanded_msg = estrcat(&Str_Get(rv.msg), &Str_New_Literal(" Failed parsing elif conditions."), data->s);
        rv.msg = expanded_msg && expanded_msg->value ? expanded_msg->value : rv.msg;
        return rv;
    }

    rv = parser_cmds_process(data, n);
    if (rv.parser_errno) {
        Str* expanded_msg = estrcat(&Str_Get(rv.msg), &Str_New_Literal(" Failed parsing elif commands."), data->s);
        rv.msg = expanded_msg && expanded_msg->value ? expanded_msg->value : rv.msg;
        return rv;
    }
    cmd_stmt_next(data, LT_ELIF);

    if (parser_peek(data->lexemes, *n) == OP_ELIF) {
        debug("found another elif to process");
        rv = parser_elif_statements(data, n);
        if (rv.parser_errno) {
            return rv;
        }
    }

    return rv;
}

[[nodiscard]]
Parser_Internal parser_if(Parser_Data* data, size_t* restrict n)
{
    debug("processing if");

    if (!parser_consume(data->lexemes, n, OP_IF)) {
        debug("no OP_IF");
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, "didn't find 'if' where expected."};
    }

    Parser_Internal rv = parser_conditions(data, n, LT_IF_CONDITIONS);
    if (rv.parser_errno) {
        return rv;
    }

    rv = parser_if_statements(data, n);
    if (rv.parser_errno) {
        Str* expanded_msg = estrcat(&Str_Get(rv.msg), &Str_New_Literal(" Couldn't process if statements."), data->s);
        rv.msg = expanded_msg && expanded_msg->value ? expanded_msg->value : rv.msg;
        return rv;
    }

    if (parser_peek(data->lexemes, *n) == OP_FI) {
        parser_consume(data->lexemes, n, OP_FI);
        debug("returning success, OP_FI found, no else");
        data->stmts->type = ST_IF;
        return (Parser_Internal){.parser_errno = 0};
    }

    enum Ops peeked = parser_peek(data->lexemes, *n);
    if (peeked != OP_ELSE && peeked != OP_ELIF) {
        debug("no OP_ELSE or OP_ELIF");
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing, 'fi', 'else', or 'elif' where expected."};
    }

    if (peeked == OP_ELSE) {
        data->stmts->type = ST_IF_ELSE;
        rv = parser_else_statements(data, n);
    }
    else if (peeked == OP_ELIF) {
        rv = parser_elif_statements(data, n);
        if (rv.parser_errno) {
            return rv;
        }

        peeked = parser_peek(data->lexemes, *n);
        if (peeked == OP_ELSE) {
            data->stmts->type = ST_IF_ELIF_ELSE;
            rv = parser_else_statements(data, n);
        }
        else {
            data->stmts->type = ST_IF_ELIF;
        }
    }

    if (rv.parser_errno) {
        return rv;
    }

    if (!parser_consume(data->lexemes, n, OP_FI)) {
        return (Parser_Internal){.parser_errno = PE_MISSING_TOK, .msg = "missing 'fi', no closing 'fi'."};
    }

    return rv;
}

[[nodiscard]]
Parser_Output parser_parse(Lexemes* restrict lexemes, Shell* restrict shell, Arena* restrict scratch)
{
    assert(lexemes); assert(scratch);
    if (!lexemes->count) {
        return (Parser_Output){
            .parser_errno = PE_NOTHING,
            .output.msg = "Nothing to process"
        };
    }

    Parser_Data data = {
        .lexemes = lexemes,
        .stmts = arena_malloc(scratch, 1, Statements),
        .cur_stmt = stmt_alloc(scratch),
        .s = scratch,
        .sh = shell
    };
    data.cur_cmds = data.cur_stmt->commands;

    Parser_Internal rv;

    if (lexemes->ops[0] == OP_CONST) {
        // TODO: check aliases in certain other conditions, like after && or ||.
        expansion_alias(lexemes, 0, scratch);
    }

    for (size_t i = 0; i < lexemes->count; ++i) {
        if (data.cur_cmds->pos >= data.cur_cmds->cap - 1) {
            cmds_realloc(&data, scratch);
        }

        switch (lexemes->ops[i]) {
        case OP_HOME_EXPANSION: {
            expansion_home(&data, i);
            break;
        }
        case OP_EQUALS:
        case OP_LESS_THAN:
        case OP_GREATER_THAN: {
            data.cur_cmds->prev_op = lexemes->ops[i];
            break;
        }
        case OP_IF: {
            rv = parser_if(&data, &i);
            if (rv.parser_errno) {
                return parser_internal_to_ouput(&rv);
            }
            continue;
        }
        case OP_VARIABLE: {
            expansion_variable(&data, &lexemes->strs[i]);
            continue;
        }
        case OP_GLOB_EXPANSION: {
            expansion_glob(lexemes->strs[i].value, data.cur_cmds, scratch);
            continue;
        }
        case OP_PIPE: {
            ++data.stmts->pipes_count;
            data.cur_cmds = cmd_next(data.cur_cmds, scratch);
            data.cur_cmds->prev_op = OP_PIPE;
            continue;
        }
        case OP_BACKGROUND_JOB: {
            data.stmts->is_bg_job = true;
            continue;
        }
        case OP_ASSIGNMENT: {
            if (data.cur_cmds->pos != 0) {
                break;
            }
            expansion_assignment(lexemes, i, shell);
            if (i + 1 < lexemes->count && (lexemes->ops[i + 1] == OP_AND || lexemes->ops[i + 1] == OP_OR)) {
                ++i; // skip || or && on assignment, assigment not included in commands
            }
            continue;
        }
        case OP_AND:
        case OP_OR: {
            data.cur_cmds = cmd_next(data.cur_cmds, scratch);
            data.cur_cmds->prev_op = lexemes->ops[i];
            continue;
        }
        case OP_STDOUT_REDIRECTION:
        case OP_STDOUT_REDIRECTION_APPEND:
        case OP_STDIN_REDIRECTION:
        case OP_STDIN_REDIRECTION_APPEND:
        case OP_STDERR_REDIRECTION:
        case OP_STDERR_REDIRECTION_APPEND:
        case OP_STDOUT_AND_STDERR_REDIRECTION:
        case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
            data.stmts->redirect_type = lexemes->ops[i];
            data.stmts->redirect_filename = lexemes->strs[i + 1].value;
            ++i; // skip filename and redirect type, not needed in commands
            continue;
        }
        }

        data.cur_cmds->strs[data.cur_cmds->pos] = lexemes->strs[i];
        data.cur_cmds->ops[data.cur_cmds->pos] = lexemes->ops[i];
        ++data.cur_cmds->pos;
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
