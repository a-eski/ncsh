/* Copyright ttyio (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */
/* ttyio.h: public interface for the ttyio library */

#ifndef TTYIO_GUARD_H_
#define TTYIO_GUARD_H_

/* WARN: current implementation is not tested to be threadsafe,
 * despite unibilium being threadsafe.
 */

#include "tcaps.h"

#define TTYIO_RED_ERROR 196

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    size_t x;
    size_t y;
} Coordinates;

typedef struct {
    Coordinates pos;
    Coordinates size;
    Coordinates saved_pos;
} Terminal;

/* enum input_type
 * Canonical: read line by line, only get the line after user presses enter. a lot of programs work this way.
 * Noncanonical: read character by character. programs who need control over each input need to use this.
 */
#if __STDC_VERSION__ >= 202311L /* C23 */
enum input_type: unsigned char {
    TTY_NONE = 0,
    TTY_CANONICAL_MODE = 1,
    TTY_NONCANONICAL_MODE = 2
};
#else
enum input_type {
    TTY_NONE = 0,
    TTY_CANONICAL_MODE = 1,
    TTY_NONCANONICAL_MODE = 2
};
#endif /* C23 */

extern termcaps tcaps;
extern Terminal term;

/* Just init term and tcaps */
void tty_init_caps(void);
/* Just init the input mode (canonical or noncanonical). */
void tty_init_input_mode(enum input_type input_type);
 /* Init everything (set input mode and setup term & tcaps) */
void tty_init(enum input_type input_type);

/* Just deinit term and tcaps (free internally used memory) */
void tty_deinit_caps(void);
/* Just deinit the input mode (reset to original) */
void tty_deinit_input_mode(void);
/* Deinit everything (reset input mode and free internally used memory) */
void tty_deinit(void);

/* Output, tracks pos of cursor for you and stores in term */
int tty_putc(char c);
int tty_fputc(FILE* restrict file, char c);
int tty_dputc(int fd, char c);

int tty_write(const char* restrict buf, size_t n);
int tty_writeln(const char* restrict buf, size_t n);
int tty_fwrite(FILE* restrict file, const char* restrict buf, size_t n);
int tty_fwriteln(FILE* restrict file, const char* restrict buf, size_t n);
int tty_dwrite(int fd, const char* restrict buf, size_t n);
int tty_dwriteln(int fd, const char* restrict buf, size_t n);

int tty_puts(const char* restrict str);
int tty_fputs(const char* restrict str, FILE* restrict file);

int tty_print(const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 1, 2)));
int tty_println(const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 1, 2)));
int tty_fprint(FILE* restrict file, const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
int tty_fprintln(FILE* restrict file, const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
int tty_dprint(int fd, const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
int tty_dprintln(int fd, const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));

int tty_perror(const char* restrict msg);

/* Output using tcaps, fallsback to ASCII control characters if cap not found. */
int tty_send(cap* restrict c);
int tty_dsend(int fd, cap* restrict c);
int tty_fsend(cap* restrict c, FILE* restrict file);
void tty_send_n(cap* restrict c, size_t n);
void tty_dsend_n(int fd, cap* restrict c, size_t n);
void tty_fsend_n(cap* restrict c, size_t n, FILE* restrict file);

/* Colors */
/* Colors don't have a fallback. If tcaps.color_max is 0, no color is set. */
int tty_color_set(int color);
int tty_color_bg_set(int color);
#define tty_color_reset() tty_send(&tcaps.color_reset)

/* Advanced Output which can have multiple fallbacks.
 * Fallback handling is in ttyio, tcaps just determines which method to use.
 */
int tty_goto_prev_eol(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* !TTYIO_GUARD_H_ */
