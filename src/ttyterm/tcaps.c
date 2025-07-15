/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */

#include <string.h> // used by macros cap_New && cap_New_Lit

#include "lib/unibilium.h"
#include "tcaps.h"

extern termcaps tcaps;
extern unibi_term* uterm;

#define FB_BS "\b \b" /* Keys */
#define FB_DEL " \b"
#define FB_NEWLINE "\n"

#define FB_CLR_SCR "\033[2J" /* Screen */
#define FB_CLR_SCR_TO_EOS "\033[J"

#define FB_CURSOR_HOME "\033[H" /* Cursor */
#define FB_CURSOR_RIGHT "\033[1C"
#define FB_CURSOR_LEFT "\033[1D"
#define FB_CURSOR_UP "\033[1A"
#define FB_CURSOR_DOWN "\033[1B"

#define FB_CURSOR_SAVE_POS "\0337"
#define FB_CURSOR_RESTORE_POS "\0338"

#define FB_CURSOR_HIDE "\033[?25l"
#define FB_CURSOR_SHOW "\033[?25h"

#define FB_CLR_TO_EOL "\033[K" /* Line */
#define FB_CLR_TO_BOL "\033[1K"
#define FB_GOTO_BOL "\r"

#define FB_COLOR_RESET "\033[0m" /* Colors */

#define tcaps_set(str, cap, fb, t)                                                                                     \
do { \
    if (!str || !*str)                                                                                                 \
        cap = cap_New_Lit(fb, t);                                                                                      \
    else                                                                                                               \
        cap = cap_New(str, t); \
} while(0)

#define tcaps_set_no_fb(str, cap, t)                                                                                   \
    if (str && *str)                                                                                                   \
    cap = cap_New(str, t)

void tcaps_init()
{
    tcaps_init_opts(true);
}

void tcaps_init_opts(bool init_advanced_caps)
{
    tcaps = (termcaps){0};
    tcaps_init_keys();
    tcaps_init_scr();
    tcaps_init_cursor();
    tcaps_init_line();
    tcaps_init_colors();
    if (init_advanced_caps) {
        tcaps_init_goto_prev_eol();
    }
}

void tcaps_init_keys()
{
    // TODO: backspace currently only uses the fallback,
    // investigate using unibi cap.
    // tcaps.bs = cap_New_Lit(FB_BS, CAP_BS);
    tcaps.bs = cap_New_Lit(FB_BS, CAP_BS);

    const char* del = unibi_get_str(uterm, unibi_key_dc);
    tcaps_set(del, tcaps.del, FB_DEL, CAP_DEL);

    const char* newline = unibi_get_str(uterm, unibi_newline);
    tcaps_set(newline, tcaps.newline, FB_NEWLINE, CAP_NEWLINE);
}

void tcaps_init_scr()
{
    const char* scr_clr = unibi_get_str(uterm, unibi_clear_screen);
    tcaps_set(scr_clr, tcaps.scr_clr, FB_CLR_SCR, CAP_SCR_CLR);

    const char* scr_clr_to_eos = unibi_get_str(uterm, unibi_clr_eos);
    tcaps_set(scr_clr_to_eos, tcaps.scr_clr_to_eos, FB_CLR_SCR_TO_EOS, CAP_SCR_CLR_TO_EOS);
}

void tcaps_init_cursor()
{
    const char* home = unibi_get_str(uterm, unibi_cursor_home);
    tcaps_set(home, tcaps.cursor_home, FB_CURSOR_HOME, CAP_CURSOR_HOME);

    const char* left = unibi_get_str(uterm, unibi_cursor_left);
    tcaps_set(left, tcaps.cursor_left, FB_CURSOR_LEFT, CAP_CURSOR_LEFT);

    const char* right = unibi_get_str(uterm, unibi_cursor_right);
    tcaps_set(right, tcaps.cursor_right, FB_CURSOR_RIGHT, CAP_CURSOR_RIGHT);

    const char* up = unibi_get_str(uterm, unibi_cursor_up);
    tcaps_set(up, tcaps.cursor_up, FB_CURSOR_UP, CAP_CURSOR_UP);

    const char* down = unibi_get_str(uterm, unibi_cursor_down);
    tcaps_set(down, tcaps.cursor_down, FB_CURSOR_DOWN, CAP_CURSOR_DOWN);

    const char* save_pos = unibi_get_str(uterm, unibi_save_cursor);
    tcaps_set(save_pos, tcaps.cursor_save, FB_CURSOR_SAVE_POS, CAP_CURSOR_SAVE);

    const char* restore_pos = unibi_get_str(uterm, unibi_restore_cursor);
    tcaps_set(restore_pos, tcaps.cursor_restore, FB_CURSOR_RESTORE_POS, CAP_CURSOR_RESTORE);

    const char* cursor_hide = unibi_get_str(uterm, unibi_cursor_invisible);
    tcaps_set(cursor_hide, tcaps.cursor_hide, FB_CURSOR_HIDE, CAP_CURSOR_HIDE);

    const char* cursor_show = unibi_get_str(uterm, unibi_cursor_visible);
    tcaps_set(cursor_show, tcaps.cursor_show, FB_CURSOR_SHOW, CAP_CURSOR_SHOW);

    const char* cursor_pos = unibi_get_str(uterm, unibi_cursor_address);
    tcaps_set_no_fb(cursor_pos, tcaps.cursor_pos, CAP_CURSOR_POS);
}

void tcaps_init_line()
{
    const char* clr_to_eol = unibi_get_str(uterm, unibi_clr_eol);
    tcaps_set(clr_to_eol, tcaps.line_clr_to_eol, FB_CLR_TO_EOL, CAP_LINE_CLR_TO_EOL);

    const char* clr_to_bol = unibi_get_str(uterm, unibi_clr_bol);
    tcaps_set(clr_to_bol, tcaps.line_clr_to_bol, FB_CLR_TO_BOL, CAP_LINE_CLR_TO_BOL);

    const char* goto_bol = unibi_get_str(uterm, unibi_carriage_return);
    tcaps_set(goto_bol, tcaps.line_goto_bol, FB_GOTO_BOL, CAP_LINE_GOTO_BOL);
}

void tcaps_init_colors()
{
    tcaps.color_max = unibi_get_num(uterm, unibi_max_colors);

    const char* reset = unibi_get_str(uterm, unibi_exit_attribute_mode);
    tcaps_set(reset, tcaps.color_reset, FB_COLOR_RESET, CAP_COLOR_RESET);

    const char* color_set = unibi_get_str(uterm, unibi_set_a_foreground);
    tcaps_set_no_fb(color_set, tcaps.color_set, CAP_COLOR_SET);

    const char* color_bg_set = unibi_get_str(uterm, unibi_set_a_background);
    tcaps_set_no_fb(color_bg_set, tcaps.color_bg_set, CAP_COLOR_BG_SET);
}

void tcaps_init_goto_prev_eol()
{
    const char* cursor_pos = unibi_get_str(uterm, unibi_cursor_address);
    if (cursor_pos && *cursor_pos) {
        tcaps.line_goto_prev_eol.fallback = FB_NONE;
    }
    else {
        tcaps.line_goto_prev_eol.fallback = FB_FIRST;
    }
}
