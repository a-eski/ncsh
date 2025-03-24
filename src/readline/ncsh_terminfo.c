/* experiment with using terminfo similar to how nvim does it */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <term.h>
#include <unistd.h>

#include "../ncsh_arena.h"
#include "../eskilib/eskilib_string.h"
#include "ncsh_terminfo_defs.h"
#include "ncsh_terminfo.h"

#define ANSI_TERM_NAME "ansi"
#define INTERIX_TERM_NAME "interix-8color"
#define ITERM_TERM_NAME "iterm-256color"
#define LINUX_TERM_NAME "linux-16color"
#define PUTTY_TERM_NAME "putty-256color"
#define RXVT_TERM_NAME  "rxvt-256color"
#define SCREEN_TERM_NAME  "screen-256color"
#define ST_TERM_NAME  "st-256color"
#define TMUX_TERM_NAME  "tmux-256color"
#define VTE_TERM_NAME  "vte-256color"
#define XTERM_TERM_NAME  "xterm-256color"
#define CYGWIN_TERM_NAME  "cygwin"
#define WIN32CON_TERM_NAME  "win32con"
#define CONEMU_TERM_NAME  "conemu"
#define VTPCON_TERM_NAME  "vtpcon"

struct ncsh_Terminfo {
  size_t name_len;
  const char* const name;
  const int8_t* terminfo;
};

const struct ncsh_Terminfo supported_terminals[] = {
  { .name = ANSI_TERM_NAME, .name_len = sizeof(ANSI_TERM_NAME), .terminfo = ansi_terminfo },
  { .name = INTERIX_TERM_NAME, .name_len = sizeof(INTERIX_TERM_NAME), .terminfo = interix_8color_terminfo },
  { .name = ITERM_TERM_NAME, .name_len = sizeof(ITERM_TERM_NAME), .terminfo = iterm_256color_terminfo },
  { .name = LINUX_TERM_NAME, .name_len = sizeof(LINUX_TERM_NAME), .terminfo = linux_16color_terminfo },
  { .name = PUTTY_TERM_NAME, .name_len = sizeof(PUTTY_TERM_NAME), .terminfo = putty_256color_terminfo },
  { .name = RXVT_TERM_NAME, .name_len = sizeof(RXVT_TERM_NAME), .terminfo = rxvt_256color_terminfo },
  { .name = SCREEN_TERM_NAME, .name_len = sizeof(SCREEN_TERM_NAME), .terminfo = screen_256color_terminfo },
  { .name = ST_TERM_NAME, .name_len = sizeof(ST_TERM_NAME), .terminfo = st_256color_terminfo },
  { .name = TMUX_TERM_NAME, .name_len = sizeof(TMUX_TERM_NAME), .terminfo = tmux_256color_terminfo },
  { .name = VTE_TERM_NAME, .name_len = sizeof(VTE_TERM_NAME), .terminfo = vte_256color_terminfo },
  { .name = XTERM_TERM_NAME, .name_len = sizeof(XTERM_TERM_NAME), .terminfo = xterm_256color_terminfo },
  { .name = CYGWIN_TERM_NAME, .name_len = sizeof(CYGWIN_TERM_NAME), .terminfo = cygwin_terminfo },
  { .name = WIN32CON_TERM_NAME, .name_len = sizeof(WIN32CON_TERM_NAME), .terminfo = win32con_terminfo },
  { .name = CONEMU_TERM_NAME, .name_len = sizeof(CONEMU_TERM_NAME), .terminfo = conemu_terminfo },
  { .name = VTPCON_TERM_NAME, .name_len = sizeof(VTPCON_TERM_NAME), .terminfo = vtpcon_terminfo }
};

constexpr size_t supported_terminals_count = sizeof(supported_terminals) / sizeof(struct ncsh_Terminfo);

void ncsh_terminfo_capabilities_set(struct ncsh_Arena* const scratch_arena)
{
  int result = setupterm((char*)0, STDOUT_FILENO, (int*)0);
  assert(!result);
  if (result) {
    puts("ncsh: couldn't load terminfo. Will assume default capabilities, may not work with all terminals.");
    return;
  }

  printf("%s\n", CUR term_names);
  size_t len = strlen(CUR term_names) + 1;
  char* terminfo_name = arena_malloc(scratch_arena, len, char);
  size_t terminfo_name_len = 0;
  for (size_t i = 0; i < len; ++i) {
    if (CUR term_names[i] == '|') {
      terminfo_name[i] = '\0';
      terminfo_name_len = i + 1;
      break;
    }
    terminfo_name[i] = CUR term_names[i];
  }
  printf("terminfo_name: %s, len: %zu\n", terminfo_name, terminfo_name_len);

  const struct ncsh_Terminfo* term = NULL;
  for (size_t i = 0; i < supported_terminals_count; ++i) {
    if (eskilib_string_compare_const(terminfo_name, terminfo_name_len, supported_terminals[i].name, supported_terminals[i].name_len)) {
      term =  &supported_terminals[i];
    }
  }

  if (!term) {
    puts("ncsh: couldn't load termtype. Some things may not work, please submit bug report if you encounter issues.");
    return;
  }

  assert(term->terminfo);
  capabilities = term->terminfo;
  printf("capabilities[0] %hhd\n", capabilities[0]);

#ifdef NCSH_DEBUG
  printf("set terminfo capabilities for %s\n", term_name);
#endif
}
