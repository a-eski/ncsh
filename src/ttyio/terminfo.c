#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <string.h>

#include "terminfo_defs.h"
#include "lib/unibilium.h"
#include "ttyplatform.h" // used for including stdbool in cases its needed, defining strdup

bool terminfo_is_term_family(const char *term, const char *family)
{
    if (!term) {
        return false;
    }
    size_t tlen = strlen(term);
    size_t flen = strlen(family);
    return tlen >= flen
        && 0 == memcmp(term, family, flen)
        // Per commentary in terminfo, minus is the only valid suffix separator.
        // The screen terminfo may have a terminal name like screen.xterm. By making
        // the dot(.) a valid separator, such terminal names will also be the
        // terminal family of the screen.
        && ('\000' == term[flen] || '-' == term[flen] || '.' == term[flen]);
}

static unibi_term *terminfo_builtin(const char *term, char **termname)
{
    if (terminfo_is_term_family(term, "xterm")) {
        *termname = strdup("builtin_xterm");
        return unibi_from_mem((const char *)xterm_256colour_terminfo,
                          sizeof xterm_256colour_terminfo);
    } else if (terminfo_is_term_family(term, "screen")) {
        *termname = strdup("builtin_screen");
        return unibi_from_mem((const char *)screen_256colour_terminfo,
                          sizeof screen_256colour_terminfo);
    } else if (terminfo_is_term_family(term, "tmux")) {
        *termname = strdup("builtin_tmux");
        return unibi_from_mem((const char *)tmux_256colour_terminfo,
                          sizeof tmux_256colour_terminfo);
    } else if (terminfo_is_term_family(term, "rxvt")) {
        *termname = strdup("builtin_rxvt");
        return unibi_from_mem((const char *)rxvt_256colour_terminfo,
                          sizeof rxvt_256colour_terminfo);
    } else if (terminfo_is_term_family(term, "putty")) {
        *termname = strdup("builtin_putty");
        return unibi_from_mem((const char *)putty_256colour_terminfo,
                          sizeof putty_256colour_terminfo);
    } else if (terminfo_is_term_family(term, "linux")) {
        *termname = strdup("builtin_linux");
        return unibi_from_mem((const char *)linux_16colour_terminfo,
                          sizeof linux_16colour_terminfo);
    } else if (terminfo_is_term_family(term, "interix")) {
        *termname = strdup("builtin_interix");
        return unibi_from_mem((const char *)interix_8colour_terminfo,
                          sizeof interix_8colour_terminfo);
    } else if (terminfo_is_term_family(term, "iterm")
             || terminfo_is_term_family(term, "iterm2")
             || terminfo_is_term_family(term, "iTerm.app")
             || terminfo_is_term_family(term, "iTerm2.app")) {
        *termname = strdup("builtin_iterm");
        return unibi_from_mem((const char *)iterm_256colour_terminfo,
                          sizeof iterm_256colour_terminfo);
    } else if (terminfo_is_term_family(term, "st")) {
        *termname = strdup("builtin_st");
        return unibi_from_mem((const char *)st_256colour_terminfo,
                          sizeof st_256colour_terminfo);
    } else if (terminfo_is_term_family(term, "gnome")
             || terminfo_is_term_family(term, "vte")) {
        *termname = strdup("builtin_vte");
        return unibi_from_mem((const char *)vte_256colour_terminfo,
                          sizeof vte_256colour_terminfo);
    } else if (terminfo_is_term_family(term, "cygwin")) {
        *termname = strdup("builtin_cygwin");
        return unibi_from_mem((const char *)cygwin_terminfo,
                          sizeof cygwin_terminfo);
    } else if (terminfo_is_term_family(term, "win32con")) {
        *termname = strdup("builtin_win32con");
        return unibi_from_mem((const char *)win32con_terminfo,
                          sizeof win32con_terminfo);
    } else if (terminfo_is_term_family(term, "conemu")) {
        *termname = strdup("builtin_conemu");
        return unibi_from_mem((const char *)conemu_terminfo,
                          sizeof conemu_terminfo);
    } else if (terminfo_is_term_family(term, "vtpcon")) {
        *termname = strdup("builtin_vtpcon");
        return unibi_from_mem((const char *)vtpcon_terminfo,
                          sizeof vtpcon_terminfo);
    } else {
        *termname = strdup("builtin_ansi");
        return unibi_from_mem((const char *)ansi_terminfo,
                          sizeof ansi_terminfo);
    }
}

unibi_term *terminfo_from_builtin(const char *term, char **termname)
{
    unibi_term *ut = terminfo_builtin(term, termname);
    if (*termname == NULL) {
        *termname = strdup("builtin_?");
    }
    return ut;
}
