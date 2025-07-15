/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "lib/unibilium.h"
#include "tcaps.h"
#include "ttyterm.h"

unibi_term* uterm;
termcaps tcaps;
Terminal term;

static struct termios otios;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
void fatal__(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // WARN: Don't use term_fprint here, if term size not set can cause infinite recursion.
    fprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);

    abort();
}
#pragma GCC diagnostic pop

Coordinates term_size_get__()
{
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
    return (Coordinates){.x = window.ws_col, .y = window.ws_row};
}

void term_init()
{
    term = (Terminal){0};
    term.size = term_size_get__();
    assert(term.size.x && term.size.y);

    uterm = unibi_from_term(getenv("TERM"));
    if (!uterm)
        fatal__("\nCan't find TERM from environment variables. Specify a terminal type with `setenv TERM <yourtype>'.\n");

    tcaps_init();

    if (!isatty(STDIN_FILENO)) {
        term_fprint(stderr, "Not running in a terminal.\n");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(STDIN_FILENO, &otios) != 0) {
        perror("Could not get terminal settings");
        exit(EXIT_FAILURE);
    }

    // mouse support? investigate
    // printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");

    struct termios tios = otios;
    tios.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    tios.c_cc[VMIN] = 1;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tios) != 0) {
        perror("Could not set terminal settings");
    }
}

void term_reset()
{
    fflush(stdout);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &otios) != 0) {
        perror("Could not restore terminal settings");
    }
    unibi_destroy(uterm);
}

void term_y_update__(int printed)
{
    if (term.pos.y < term.size.y - 1) {
        if (!printed)  {
            ++term.pos.y;
        }
        else {
            term.pos.y += (int)(printed / term.size.x);
        }
    }
}

void term_size_update__(int printed)
{
    assert(printed != EOF);
    if (!term.size.x)
        fatal__("\nTerm size not set.\n");

    if (printed + term.pos.x + 1 > term.size.x) {
        term_y_update__(printed);
        term.pos.x = printed == 1 ? 1 : (printed % term.size.x) + 1;
    }
    else {
        term.pos.x += printed == 1 ? 1 : printed + 1;
    }
}

int term_putc(const char c)
{
    [[maybe_unused]] int printed = write(STDOUT_FILENO, &c, 1);
    assert(printed != EOF && printed == 1);
    term_size_update__(1);
    return 1;
}

int term_write(const char* buf, const size_t n)
{
    int printed = write(STDOUT_FILENO, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    term_size_update__(printed);
    return printed;
}

int term_writeln(const char* buf, const size_t n)
{
    int printed = write(STDOUT_FILENO, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    term_size_update__(printed);
    term_send(&tcaps.newline);
    return printed;
}

int term_fwrite(const int fd, const char* buf, const size_t n)
{
    int printed = write(fd, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    term_size_update__(printed);
    return printed;
}

int term_fwriteln(const int fd, const char* buf, const size_t n)
{
    int printed = write(fd, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    term_size_update__(printed);
    term_dsend(fd, &tcaps.newline);
    return printed;
}

int term_puts(const char* restrict str)
{
    int printed = puts(str);
    term.pos.x = 0;
    term_y_update__(0);
    return printed;
}

int term_fputs(const char* restrict str, FILE* restrict file)
{
    int printed = fputs(str, file);
    term.pos.x = 0;
    term_send(&tcaps.newline);
    return printed;
}

int term_print(const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    return printed;
}

int term_println(const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    term_send(&tcaps.newline);
    return printed;
}

int term_fprint(FILE* restrict file, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(file, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    return printed;
}

int term_fprintln(FILE* restrict file, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(file, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    return printed;
}

int term_dprint(const int fd, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vdprintf(fd, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    return printed;
}

int term_dprintln(const int fd, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vdprintf(fd, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    term_send(&tcaps.newline);
    return printed;
}

int term_perror(const char* restrict msg)
{
    char* err_str = strerror(errno);
    term_color_set(TERM_RED_ERROR);
    int printed = term_fprint(stderr, "%s: ", msg);
    term_color_reset();
    printed += term_fprint(stderr, err_str);

    term_size_update__(printed);
    term_fsend(&tcaps.newline, stderr);
    return printed;
}

int term_send(cap* restrict c)
{
    return term_dsend(STDOUT_FILENO, c);
}

int term_fsend(cap* restrict c, FILE* restrict file)
{
    assert(c && c->len);
    fwrite(c->val, sizeof(char), c->len, file);
    fflush(file);

    switch (c->type) {
    case CAP_BS:
        --term.pos.x;
        break;
    case CAP_CURSOR_HOME: {
        term.pos.x = 0;
        term.pos.y = 0;
        break;
    }
    case CAP_CURSOR_RIGHT:
        ++term.pos.x;
        break;
    case CAP_CURSOR_LEFT:
        --term.pos.x;
        break;
    case CAP_CURSOR_UP:
        --term.pos.y;
        break;
    case CAP_CURSOR_DOWN:
        ++term.pos.y;
        break;
    case CAP_CURSOR_SAVE:
        term.saved_pos.x = term.pos.x;
        term.saved_pos.y = term.pos.y;
        break;
    case CAP_CURSOR_RESTORE:
        term.pos.x = term.saved_pos.x;
        term.pos.y = term.saved_pos.y;
        break;
    case CAP_NEWLINE:
        term.pos.x = 0;
        term_y_update__(0);
    default:
        break;
    }

    return 0;
}

int term_dsend(const int fd, cap* restrict c)
{
    assert(c && c->len);
    if (write(fd, c->val, c->len) == -1)
        return 1;
    fflush(stdout);

    switch (c->type) {
    case CAP_BS:
        --term.pos.x;
        break;
    case CAP_CURSOR_HOME: {
        term.pos.x = 0;
        term.pos.y = 0;
        break;
    }
    case CAP_CURSOR_RIGHT:
        ++term.pos.x;
        break;
    case CAP_CURSOR_LEFT:
        --term.pos.x;
        break;
    case CAP_CURSOR_UP:
        --term.pos.y;
        break;
    case CAP_CURSOR_DOWN:
        ++term.pos.y;
        break;
    case CAP_CURSOR_SAVE:
        term.saved_pos.x = term.pos.x;
        term.saved_pos.y = term.pos.y;
        break;
    case CAP_CURSOR_RESTORE:
        term.pos.x = term.saved_pos.x;
        term.pos.y = term.saved_pos.y;
        break;
    case CAP_NEWLINE:
        term.pos.x = 0;
        term_y_update__(0);
    default:
        break;
    }

    return 0;
}

void term_send_n(cap* restrict c, const size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        term_send(c);
    }
}

void term_fsend_n(cap* restrict c, const size_t n, FILE* restrict file)
{
    for (size_t i = 0; i < n; ++i) {
        term_fsend(c, file);
    }
}

void term_dsend_n(const int fd, cap* restrict c, const size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        term_dsend(fd, c);
    }
}

int term_color_set(int color)
{
    if (!tcaps.color_max)
        return 0;

    constexpr size_t size = 64;
    char buf[size] = {0};
    size_t len = unibi_run(tcaps.color_set.val, (unibi_var_t[9]){[0] = unibi_var_from_num(color)}, buf, size);

    if (write(STDOUT_FILENO, buf, len) == -1)
        return 1;
    fflush(stdout);
    return 0;
}

int term_color_bg_set(int color)
{
    if (!tcaps.color_max)
        return 0;

    constexpr size_t size = 64;
    char buf[size] = {0};
    size_t len = unibi_run(tcaps.color_bg_set.val, (unibi_var_t[9]){[0] = unibi_var_from_num(color)}, buf, size);

    if (write(STDOUT_FILENO, buf, len) == -1)
        return 1;
    fflush(stdout);
    return 0;
}

int term_color_reset()
{
    return term_send(&tcaps.color_reset);
}

int term_goto_prev_eol()
{
    if (tcaps.line_goto_prev_eol.fallback == FB_NONE) {
        constexpr size_t size = 64;
        char buf[size] = {0};
        assert(term.pos.y > 0);
        size_t len = unibi_run(tcaps.cursor_pos.val,
                        (unibi_var_t[9]){
                            [0] = unibi_var_from_num(term.pos.y == 0 ? 0 : term.pos.y - 1),
                            [1] = unibi_var_from_num(term.size.x - 1)
                        },
                        buf, size);

        if (write(STDOUT_FILENO, buf, len) == -1)
            return 1;
        fflush(stdout);
        term.pos.x = term.size.x - term.pos.x - 1;
        --term.pos.y;
        return 0;
    }
    if (tcaps.line_goto_prev_eol.fallback >= FB_FIRST) {
        term_send(&tcaps.cursor_up);
        term_send_n(&tcaps.cursor_right, term.size.x - term.pos.x - 1);
        fflush(stdout);
        return 0;
    }

    unreachable();
}

#pragma GCC diagnostic pop
