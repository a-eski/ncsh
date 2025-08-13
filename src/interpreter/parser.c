/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.c: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../debug.h"
#include "../ttyio/ttyio.h"
#include "expansions.h"
#include "lexemes.h"
#include "ops.h"
#include "parser.h"
#include "statements.h"

static Commands* commands;

bool parser_consume(Lexemes* restrict lexemes, size_t* restrict n, enum Ops expected)
{
    assert(*n < lexemes->count);
    if (*n < lexemes->count && lexemes->ops[*n] == expected) {
        ++*n;
        return true;
    }
    else {
        debugf("parser_consume false. expected %d, actual %d\n", expected, lexemes->ops[*n]);
        return false;
    }
}

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
    case OP_CONSTANT:
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

int parser_commands_process(Shell* shell, Lexemes* restrict lexemes, size_t* restrict n, Statements* restrict stmts, Arena* restrict scratch)
{
    if (!parser_is_valid_statement(parser_peek(lexemes, *n))) {
        tty_puts("ncsh parser: no valid statement.");
        return EXIT_FAILURE;
    }

    do {
        bool end_of_statement = false;
        size_t len = lexemes->lens[*n];
        if (len > 2 && lexemes->vals[*n][len - 2] == ';') {
            debugf("replacing ; val %s, char %c\n", lexemes->vals[*n], lexemes->vals[*n][len - 2]);
            lexemes->vals[*n][len - 2] = '\0';
            --lexemes->lens[*n];
            end_of_statement = true;
        }

        switch (lexemes->ops[*n]) {
        case OP_HOME_EXPANSION:
            expansion_home(shell, lexemes, *n, scratch);
            break;
        case OP_EQUALS:
        case OP_LESS_THAN:
        case OP_GREATER_THAN: {
            commands->current_op = lexemes->ops[*n];
            commands->prev_op = lexemes->ops[*n];
            break;
        }
        case OP_VARIABLE:
            expansion_variable(lexemes->vals[*n], lexemes->lens[*n], commands, /*stmts,*/ shell, scratch);
            ++*n;
            continue;
        case OP_GLOB_EXPANSION:
            expansion_glob(lexemes->vals[*n], commands, scratch);
            continue;
        case OP_PIPE:
            ++stmts->pipes_count;
            commands[commands->pos - 1].current_op = OP_PIPE;
            commands = command_next(commands, scratch);
            commands->prev_op = OP_PIPE;
            continue;
        case OP_ASSIGNMENT:
            expansion_assignment(lexemes, *n, shell);
            if (*n + 1 < lexemes->count && (lexemes->ops[*n + 1] == OP_AND || lexemes->ops[*n + 1] == OP_OR)) {
                ++*n; // skip || or && on assignment, assigment not included in commands
            }
            continue;
        case OP_AND:
        case OP_OR: {
            commands->current_op = lexemes->ops[*n];
            commands = command_next(commands, scratch);
            commands->prev_op = lexemes->ops[*n];
            ++*n;
            continue;
        }
        }

        commands->vals[commands->pos] = lexemes->vals[*n];
        commands->lens[commands->pos] = lexemes->lens[*n];
        commands->ops[commands->pos] = lexemes->ops[*n];
        ++commands->pos;
        ++*n;

        // enum Ops peeked = parser_peek(lexemes, *n);
        if (end_of_statement) {
            commands = command_next(commands, scratch);
            debug("end of statement");
        }
        /*else if (peeked == OP_AND || peeked == OP_OR) {
            command_next_set_prev_op(lexemes->ops[*n], scratch);
            debug("end of statement, && or || found");
        }*/
    } while (parser_is_valid_statement(parser_peek(lexemes, *n)));

    debug("commands processed");
    return EXIT_SUCCESS;
}

int parser_conditions(Shell* shell, Lexemes* restrict lexemes, size_t* restrict n, enum Logic_Type type, Statements* restrict stmts, Arena* restrict scratch)
{
    if (!parser_consume(lexemes, n, OP_CONDITION_START)) {
        debug("no OP_CONDITION_START");
        return EXIT_FAILURE;
    }

    debug("processing conditions");
    int res = parser_commands_process(shell, lexemes, n, stmts, scratch);
    if (res != EXIT_SUCCESS) {
        return res;
    }

    commands = command_statement_next(stmts, commands, type, scratch);
    if (!parser_consume(lexemes, n, OP_CONDITION_END)) {
        debug("no OP_CONDITION_END");
        return EXIT_FAILURE;
    }

    if (!parser_consume(lexemes, n, OP_THEN)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int parser_if_statements(Shell* shell, Lexemes* restrict lexemes, size_t* restrict n, Statements* restrict stmts, Arena* restrict scratch)
{
    debug("processing if statements");
    int res = parser_commands_process(shell, lexemes, n, stmts, scratch);
    if (res != EXIT_SUCCESS) {
        return res;
    }
    commands = command_statement_next(stmts, commands, LT_IF, scratch);
    return res;
}

int parser_else_statements(Shell* shell, Lexemes* restrict lexemes, size_t* restrict n, Statements* restrict stmts, Arena* restrict scratch)
{
    debug("processing else");
    if (!parser_consume(lexemes, n, OP_ELSE)) {
        return EXIT_FAILURE;
    }

    int res = parser_commands_process(shell, lexemes, n, stmts, scratch);
    if (res != EXIT_SUCCESS) {
        return res;
    }

    commands = command_statement_next(stmts, commands, LT_ELSE, scratch);
    return EXIT_SUCCESS;
}

int parser_elif_statements(Shell* shell, Lexemes* restrict lexemes, size_t* restrict n, Statements* restrict stmts, Arena* restrict scratch)
{
    debug("processing elif");
    if (!parser_consume(lexemes, n, OP_ELIF)) {
        return EXIT_FAILURE;
    }

    int res = parser_conditions(shell, lexemes, n, LT_ELIF_CONDTIONS, stmts, scratch);
    if (res != EXIT_SUCCESS) {
        debug("failed parsing elif conditions");
        return res;
    }

    res = parser_commands_process(shell, lexemes, n, stmts, scratch);
    if (res != EXIT_SUCCESS) {
        debug("failed parsing elif commands");
        return EXIT_FAILURE;
    }
    commands = command_statement_next(stmts, commands, LT_ELIF, scratch);

    if (parser_peek(lexemes, *n) == OP_ELIF) {
        debug("found another elif to process");
        res = parser_elif_statements(shell, lexemes, n, stmts, scratch);
        if (res != EXIT_SUCCESS) {
            debug("failed parsing another elif");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int parser_if(Shell* shell, Lexemes* restrict lexemes, size_t* restrict n, Statements* restrict stmts, Arena* restrict scratch)
{
    debug("processing if");
    if (!parser_consume(lexemes, n, OP_IF)) {
        debug("no OP_IF");
        return EXIT_FAILURE;
    }

    int res = parser_conditions(shell, lexemes, n, LT_IF_CONDITIONS, stmts, scratch);
    if (res != EXIT_SUCCESS) {
        return res;
    }

    res = parser_if_statements(shell, lexemes, n, stmts, scratch);
    if (res != EXIT_SUCCESS) {
        debug("can't process if statements");
        return res;
    }

    if (parser_peek(lexemes, *n) == OP_FI) {
        parser_consume(lexemes, n, OP_FI);
        debug("returning success, OP_FI found, no else");
        stmts->type = ST_IF;
        return EXIT_SUCCESS;
    }

    enum Ops peeked = parser_peek(lexemes, *n);
    if (peeked != OP_ELSE && peeked != OP_ELIF) {
        debug("no OP_ELSE or OP_ELIF");
        return EXIT_FAILURE;
    }

    res = EXIT_FAILURE;
    if (peeked == OP_ELSE) {
        stmts->type = ST_IF_ELSE;
        res = parser_else_statements(shell, lexemes, n, stmts, scratch);
    }
    else if (peeked == OP_ELIF) {
        res = parser_elif_statements(shell, lexemes, n, stmts, scratch);
        if (res != EXIT_SUCCESS) {
            return res;
        }

        peeked = parser_peek(lexemes, *n);
        if (peeked == OP_ELSE) {
            stmts->type = ST_IF_ELIF_ELSE;
            res = parser_else_statements(shell, lexemes, n, stmts, scratch);
        }
        else {
            stmts->type = ST_IF_ELIF;
        }
    }

    if (res != EXIT_SUCCESS) {
        return res;
    }

    if (!parser_consume(lexemes, n, OP_FI)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int parser_parse(Lexemes* restrict lexemes, Statements* stmts, Shell* restrict shell, Arena* restrict scratch)
{
    assert(stmts);
    assert(scratch);

    statements_init(stmts, scratch);
    if (!lexemes->count) {
        return EXIT_SUCCESS;
    }
    assert(stmts->statements->commands);
    assert(stmts->statements->commands->vals);

    int res;
    commands = stmts->statements[stmts->pos].commands;
    commands->pos = commands->count;
    assert(commands);

    if (lexemes->ops[0] == OP_CONSTANT) {
        // TODO: check aliases in certain other conditions, like after && or ||.
        expansion_alias(lexemes, 0, scratch);
    }

    for (size_t i = 0; i < lexemes->count && lexemes->vals[i]; ++i) {
        if (commands->pos >= commands->cap - 1) {
            commands_realloc(stmts, scratch);
        }

        switch (lexemes->ops[i]) {
        case OP_HOME_EXPANSION: {
            expansion_home(shell, lexemes, i, scratch);
            break;
        }
        case OP_EQUALS:
        case OP_LESS_THAN:
        case OP_GREATER_THAN: {
            commands->current_op = lexemes->ops[i];
            commands->prev_op = lexemes->ops[i];
            break;
        }
        case OP_IF: {
            res = parser_if(shell, lexemes, &i, stmts, scratch);
            if (res != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            continue;
        }
        case OP_VARIABLE: {
            expansion_variable(lexemes->vals[i], lexemes->lens[i], commands, /*stmts,*/ shell, scratch);
            continue;
        }
        case OP_GLOB_EXPANSION: {
            expansion_glob(lexemes->vals[i], commands, scratch);
            continue;
        }
        case OP_PIPE: {
            ++stmts->pipes_count;
            commands[commands->pos - 1].current_op = OP_PIPE;
            commands = command_next(commands, scratch);
            commands->prev_op = OP_PIPE;
            continue;
        }
        case OP_BACKGROUND_JOB: {
            stmts->is_bg_job = true;
            continue;
        }
        case OP_ASSIGNMENT: {
            if (commands->pos != 0) {
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
            commands->current_op = lexemes->ops[i];
            commands = command_next(commands, scratch);
            commands->prev_op = lexemes->ops[i];
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
            stmts->redirect_type = lexemes->ops[i];
            stmts->redirect_filename = lexemes->vals[i + 1];
            ++i; // skip filename and redirect type, not needed in commands
            continue;
        }
        }

        commands->vals[commands->pos] = lexemes->vals[i];
        commands->lens[commands->pos] = lexemes->lens[i];
        commands->ops[commands->pos] = lexemes->ops[i];
        ++commands->pos;
    }

    if (commands->pos > 0) {
        if (!stmts->count) {
            stmts->count = !stmts->pos ? 1 : stmts->pos;
        }
        if (!stmts->statements->count) {
            ++stmts->statements->count;
        }
        commands->count = commands->pos;
    }

    // no branch is fine, this value not used unless pipes are present.
    ++stmts->pipes_count; // count the number of commands, not pipes

    return EXIT_SUCCESS;
}
