/* Copyright ttyio (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */

#pragma once

#ifndef TCAPS_GUARD_H_
#define TCAPS_GUARD_H_

#include <stddef.h>
#include <stdio.h>

#include "ttyplatform.h" // used for including stdbool in cases its needed

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum caps {
    CAP_BS,
    CAP_DEL,
    CAP_NEWLINE,

    CAP_SCR_CLR,
    CAP_SCR_CLR_TO_EOS,

    CAP_CURSOR_HOME,
    CAP_CURSOR_LEFT,
    CAP_CURSOR_RIGHT,
    CAP_CURSOR_UP,
    CAP_CURSOR_DOWN,
    CAP_CURSOR_SHOW,
    CAP_CURSOR_HIDE,
    CAP_CURSOR_SAVE,
    CAP_CURSOR_RESTORE,
    CAP_CURSOR_POS,

    CAP_LINE_CLR_TO_EOL,
    CAP_LINE_CLR_TO_BOL,
    CAP_LINE_GOTO_BOL, // i.e. carriage return

    CAP_COLOR_RESET,
    CAP_COLOR_SET,
    CAP_COLOR_BG_SET,
    CAPS_END
};

enum advanced_caps__ {
    CAP_LINE_GOTO_PREV_EOL = CAPS_END
};

typedef struct {
    enum caps type;
    size_t len;
    const char* val;
} cap;

// Advanced caps can use multiple fallbacks that must be handled by functions in ttyio.
typedef struct {
    enum { FB_NONE, FB_FIRST, FB_SECOND } fallback;
    enum advanced_caps__ type;
} advanced_cap__;

typedef struct {
    cap bs; /* Keys */
    cap del;
    cap newline;

    cap scr_clr; /* Screen */
    cap scr_clr_to_eos;

    cap cursor_home; /* Cursor */
    cap cursor_left;
    cap cursor_right;
    cap cursor_up;
    cap cursor_down;
    cap cursor_show;
    cap cursor_hide;
    cap cursor_save;
    cap cursor_restore;
    cap cursor_pos;

    cap line_clr_to_eol; /* Line */
    cap line_clr_to_bol;
    cap line_goto_bol;
    advanced_cap__ line_goto_prev_eol;

    int color_max; /* Colors */
    cap color_reset;
    cap color_set;
    cap color_bg_set;

} termcaps;

/* Init all caps */
void tcaps_init(void);
/* Init all caps but can exclude advanced caps like line_goto_prev_eol */
void tcaps_init_opts(bool init_advanced_caps);

/* Specific caps initialization */
void tcaps_init_keys(void);
void tcaps_init_scr(void);
void tcaps_init_cursor(void);
void tcaps_init_line(void);
void tcaps_init_colors(void);

/* Advanced cap initiailization */
void tcaps_init_goto_prev_eol(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !TCAPS_GUARD_H_
