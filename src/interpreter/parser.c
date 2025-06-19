/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.c: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../defines.h"
#include "lexemes.h"
#include "ops.h"
#include "statements.h"
#include "expansions.h"
#include "parser.h"

size_t command_n;
size_t statement_n;
Commands* commands;

void command_next(enum Ops op, Arena* rst scratch)
{
    commands->count = command_n == 0 ? 1 : command_n;
    commands->next = commands_alloc(scratch);

    commands = commands->next;
    commands->prev_op = op;
    command_n = 0;
}

void statement_next(Statements* rst statements, enum Statement_Type type, Arena* rst scratch)
{
    statements->statements[statement_n].type = type;
    ++statements->statements[statement_n].count;
    ++statement_n;
    statements->statements[statement_n].commands = commands_alloc(scratch);
    commands->count = command_n == 0 ? 1 : command_n; // update last commands count
    commands = statements->statements[statement_n].commands;
    command_n = 0; // reset count
}

bool parser_consume(Lexemes* rst lexemes, size_t* rst n, enum Ops expected)
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

enum Ops parser_peek(Lexemes* rst lexemes, size_t n)
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
    case OP_GREATER_THAN: {
        return true;
    }
    default: {
        return false;
    }
    }
}

int parser_commands_process(Lexemes* rst lexemes, size_t* rst n, Arena* rst scratch)
{
    if (!parser_is_valid_statement(parser_peek(lexemes, *n))) {
        debug("no valid statement");
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

        commands->vals[command_n] = lexemes->vals[*n];
        commands->lens[command_n] = lexemes->lens[*n];
        commands->ops[command_n] = lexemes->ops[*n];
        ++command_n;
        ++*n;

        enum Ops peeked = parser_peek(lexemes, *n);
        if (end_of_statement || peeked == OP_AND || peeked == OP_OR) {
            command_next(lexemes->ops[*n], scratch);
            debug("end of statement");
        }
    } while (parser_is_valid_statement(parser_peek(lexemes, *n)));

    debug("commands processed");
    return EXIT_SUCCESS;
}

int parser_conditions(Lexemes* rst lexemes, size_t* rst n, Statements* rst statements, Arena* rst scratch)
{
    debug("processing conditions");
    int res =  parser_commands_process(lexemes, n, scratch);
    if (res != EXIT_SUCCESS) {
        return res;
    }
    statement_next(statements, ST_CONDITIONS, scratch);
    return EXIT_SUCCESS;
}

int parser_if_statements(Lexemes* rst lexemes, size_t* rst n, Statements* rst statements, Arena* rst scratch)
{
    debug("processing if statements");
    int res =  parser_commands_process(lexemes, n, scratch);
    if (res != EXIT_SUCCESS) {
        return res;
    }
    statement_next(statements, ST_IF, scratch);
    return res;
}

int parser_else_statements(Lexemes* rst lexemes, size_t* rst n, Statements* rst statements, Arena* rst scratch)
{
    debug("processing else");
    if (!parser_consume(lexemes, n, OP_ELSE)) {
        return EXIT_FAILURE;
    }

    int res = parser_commands_process(lexemes, n, scratch);
    if (res != EXIT_SUCCESS) {
        return res;
    }

    statement_next(statements, ST_ELSE, scratch);
    return EXIT_SUCCESS;
}

int parser_elif_statements(Lexemes* rst lexemes, size_t* rst n, Statements* rst statements, Arena* rst scratch)
{
    debug("processing elif");
    if (!parser_consume(lexemes, n, OP_ELIF)) {
        return EXIT_FAILURE;
    }

    int res = parser_commands_process(lexemes, n, scratch);
    if (res != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    statement_next(statements, ST_ELIF, scratch);

    if (parser_peek(lexemes, *n) == OP_ELIF) {
        res = parser_elif_statements(lexemes, n, statements, scratch);
        if (res != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int parser_if(Lexemes* rst lexemes, size_t* rst n, Statements* rst statements, Arena* rst scratch)
{
    debug("processing if");
    if (!parser_consume(lexemes, n, OP_IF)) {
        debug("no OP_IF");
        return EXIT_FAILURE;
    }

    if (!parser_consume(lexemes, n, OP_CONDITION_START)) {
        debug("no OP_CONDITION_START");
        return EXIT_FAILURE;
    }

    int res = parser_conditions(lexemes, n, statements, scratch);
    if (res != EXIT_SUCCESS) {
        return res;
    }

    if (!parser_consume(lexemes, n, OP_CONDITION_END)) {
        debug("no OP_CONDITION_END");
        return EXIT_FAILURE;
    }

    if (!parser_consume(lexemes, n, OP_THEN)) {
        return EXIT_FAILURE;
    }

    res = parser_if_statements(lexemes, n, statements, scratch);
    if (res != EXIT_SUCCESS) {
        debug("can't process if statements");
        return res;
    }

    if (parser_peek(lexemes, *n) == OP_FI) {
        parser_consume(lexemes, n, OP_FI);
        debug("returning success, OP_FI found, no else");
        return EXIT_SUCCESS;
    }

    enum Ops peeked = parser_peek(lexemes, *n);
    if (peeked != OP_ELSE && peeked != OP_ELIF) {
        debug("no OP_ELSE or OP_ELIF");
        return EXIT_FAILURE;
    }

    res = EXIT_FAILURE;
    if (peeked == OP_ELSE) {
        res = parser_else_statements(lexemes, n, statements, scratch);
    }
    else if (peeked == OP_ELIF) {
        res = parser_elif_statements(lexemes, n, statements, scratch);
    }

    if (res != EXIT_SUCCESS) {
        return res;
    }

    if (!parser_consume(lexemes, n, OP_FI)) {
        return EXIT_FAILURE;
    }

    ++statements->count;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int parser_parse(Lexemes* rst lexemes, Statements* rst statements, Shell* rst shell, Arena* rst scratch)
{
    assert(statements);
    assert(scratch);

    statements_init(statements, scratch);
    if (!lexemes->count) {
        return EXIT_SUCCESS;
    }

    int res;
    command_n = 0;
    statement_n = 0;
    commands = statements->statements[statement_n].commands;
    assert(commands);

    for (size_t i = 0; i < lexemes->count; ++i) {
        if (!lexemes->vals[i]) {
            break;
        }

        if (command_n >= commands->cap) {
            commands_realloc(statements, scratch);
        }

        if (i + 1 < lexemes->count &&
            (lexemes->ops[i + 1] == OP_STDIN_REDIRECTION || lexemes->ops[i + 1] == OP_STDIN_REDIRECTION_APPEND)) {
                statements->redirect_type = lexemes->ops[i + 1];
                statements->redirect_filename = lexemes->vals[i];
                ++i;
                continue;
        }
        else {
            switch (lexemes->ops[i]) {
                case OP_PIPE:
                    ++statements->pipes_count;
                    break;
                case OP_HOME_EXPANSION:
                    expansion_home(lexemes, i, scratch);
                    break;
                case OP_IF:
                    res = parser_if(lexemes, &i, statements, scratch);
                    if (res != EXIT_SUCCESS)
                        return EXIT_FAILURE;
                    break;
                case OP_BACKGROUND_JOB:
                    statements->is_bg_job = true;
                    continue;
                case OP_ASSIGNMENT:
                    expansion_assignment(lexemes + i, i, &shell->vars, scratch);
                    if (i + 1 < lexemes->count && (lexemes->ops[i + 1] == OP_AND || lexemes->ops[i + 1] == OP_OR)) {
                        ++i; // skip || or && on assignment, assigment not included in commands
                    }
                    continue;
                case OP_AND:
                case OP_OR: {
                    command_next(lexemes->ops[i], scratch);
                    continue;
                }
                case OP_STDOUT_REDIRECTION:
                case OP_STDOUT_REDIRECTION_APPEND:
                case OP_STDERR_REDIRECTION:
                case OP_STDERR_REDIRECTION_APPEND:
                case OP_STDOUT_AND_STDERR_REDIRECTION:
                case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND:
                    statements->redirect_type = lexemes->ops[i];
                    statements->redirect_filename = lexemes->vals[i + 1];
                    ++i; // skip filename and redirect type, not needed in commands
                    continue;
            }
        }

        commands->vals[command_n] = lexemes->vals[i];
        commands->lens[command_n] = lexemes->lens[i];
        commands->ops[command_n] = lexemes->ops[i];
        ++command_n;
    }

    if (command_n > 0) {
        if (!statements->count) {
            statements->count = !statement_n ? 1 : statement_n;
        }
        if (!statements->statements->count) {
            ++statements->statements->count;
        }
        commands->count = command_n;
    }

    // expansions_process(lexemes, shell, scratch);
    // ops_process

    return EXIT_SUCCESS;
}
