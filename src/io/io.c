#include "io.h"
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "../defines.h"
#include "../env.h"
#include "../ttyio/ttyio.h"
#include "prompt.h"
#include "input.h"

#define AUTOCOMPLETE_YELLOW 220
#define AUTOCOMPLETE_DIM 244

extern volatile int sigwinch_caught;

void io_prompt_init()
{
    bool show_user;
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    show_user = true;
#elif NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NONE
    show_user = false;
#else
    show_user = true;
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */

    enum Dir_Type dir_type;
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
    dir_type = DIR_NORMAL;
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
    dir_type = DIR_SHORT;
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NONE
    dir_type = DIR_NONE;
#else
    dir_type = DIR_NORMAL;
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_PROMPT_DIRECTORY */

    prompt_init(show_user, dir_type);
}

/* io_init
 * Allocates memory that lives for the lifetime of the shell and is used by io to process user input.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...)
 */
int io_init(Config* restrict config, Env* restrict env, Input* restrict input, Arena* restrict arena)
{
    io_prompt_init();
    Str user_key = Str_New_Literal(NCSH_USER_VAL);
    input->user = env_add_or_get(env, user_key);
    if (!input->user->value) {
        input->user->length = 0;
    }
    input->buffer = arena_malloc(arena, NCSH_MAX_INPUT, char);

    if (history_init(config->location, &input->history, arena) != E_SUCCESS) {
        tty_perror("ncsh: Error occurred while setting up history");
        return EXIT_FAILURE;
    }

    input->current_autocompletion = arena_malloc(arena, NCSH_MAX_INPUT, char);
    input->autocompletions_tree = ac_alloc(arena);
    ac_add_multiple(input->history.entries, input->history.count, input->autocompletions_tree, arena);

    return EXIT_SUCCESS;
}

/* io_exit
 * Saves history changes and restores the terminal settings from before the shell was started.
 */
void io_deinit(Input* restrict input, Arena scratch)
{
    if (input && input->history.file.value && input->history.entries) {
        history_save(&input->history, &scratch);
    }
}

void io_cursor_restore(Input* restrict input)
{
    size_t prev_size_y = term.size.y;
    size_t prev_pos_y = term.pos.y;
    tty_send(&tcaps.cursor_restore);
    if (prev_pos_y == prev_size_y - 1) {
        tty_send_n(&tcaps.cursor_up, input->lines_y);
    }
}

int io_error([[maybe_unused]] Input* restrict input)
{
    return EXIT_FAILURE_CONTINUE;
}

int io_putchar(Input* restrict input);

int io_char(Input* restrict input)
{
    if (input->pos == NCSH_MAX_INPUT - 1) {
        tty_send(&tcaps.newline);
        tty_color_set(TTYIO_RED_ERROR);
        tty_fprintln(stderr, "ncsh: Hit max input.");
        tty_send(&tcaps.color_reset);
        input->buffer[0] = '\0';
        input->pos = 0;
        input->max_pos = 0;
        return EXIT_CONTINUE;
    }

    return io_putchar(input);
}

int io_exit([[maybe_unused]] Input* restrict input)
{
    tty_send(&tcaps.line_clr_to_eol);
    tty_send(&tcaps.newline);
    return EXIT_SUCCESS_END;
}

// IO
/* enum Line_Adjustment
 * Represents if the cursor was moved to previous line, next line, or not at all.
 */
enum Line_Adjustment : uint8_t {
    L_NONE = 0,
    L_NEXT,
    L_PREVIOUS
};

/* io_adjust_line_if_needed
 * Checks if a newline needs to be inserted.
 * For moving to the next line, nothing happens except increasing lines_y and current_y, which track y position relative
 * to the line the prompt started on. For moving to the previous line, we manually move the cursor to the end of the
 * previous line, decrease lines_y and current_y. Returns: enum Line_Adjustment, a value that indicates whether any line
 * change has happened or not
 */
enum Line_Adjustment io_adjust_line_if_needed(Input* restrict input)
{
    if (!input->pos || input->pos < term.size.x) {
        return L_NONE;
    }

    // Is eol
    if (term.pos.x >= term.size.x - 1) {
        ++input->lines_y;
        input->current_y = input->lines_y;
        return L_NEXT;
    }

    // is start of line?
    if (term.pos.y > 0 && term.pos.x == 0) {
        --input->lines_y;
        input->current_y = input->lines_y;

        tty_send(&tcaps.line_clr_to_eol);
        tty_goto_prev_eol();
        return L_PREVIOUS;
    }

    return L_NONE;
}

[[nodiscard]]
int io_word_delete(Input* restrict input)
{
    if (!input->pos && !input->max_pos) {
        return EXIT_SUCCESS;
    }

    tty_send(&tcaps.bs);
    tty_send(&tcaps.line_clr_to_eol);
    input->buffer[input->pos] = '\0';
    --input->pos;

    while (input->pos > 0) {
        io_adjust_line_if_needed(input);
        /*if (io_adjust_line_if_needed(input) == L_PREVIOUS) {
            tty_send(&tcaps.line_clr_to_eol);
        }*/
        if (input->buffer[input->pos] == ' ') {
            break;
        }

        tty_send(&tcaps.bs);
        input->buffer[input->pos] = '\0';
        --input->pos;
    }

    input->max_pos = input->pos;

    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_line_delete_forwards(Input* restrict input)
{
    // TODO: BUG: fix this deleting whole line instead of forwards
    if (!input->pos && !input->max_pos) {
        return EXIT_SUCCESS;
    }

    io_cursor_restore(input);
    tty_send(&tcaps.scr_clr_to_eos);

    memset(input->buffer, '\0', input->max_pos + 1);
    input->max_pos = 0;
    input->pos = 0;
    input->lines_y = 0;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_line_delete_backwards(Input* restrict input)
{
    if (!input->pos && !input->max_pos) {
        return EXIT_SUCCESS;
    }

    tty_send_n(&tcaps.bs, input->pos);

    memmove(input->buffer, input->buffer + input->pos + 1, input->max_pos - input->pos + 1);
    input->buffer[input->pos + 1] = '\0';
    tty_write(input->buffer, input->pos + 1);
    tty_send(&tcaps.scr_clr_to_eos);
    input->max_pos = input->pos;
    input->lines_y = 0;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_putchar(Input* restrict input)
{
    char character = input->c;
    char temp_character;

    // midline insertions
    if (input->pos < input->max_pos && input->buffer[input->pos]) {
        input->start_pos = input->pos;

        if (!input->pos) {
            temp_character = input->buffer[0];
            input->buffer[0] = character;
            tty_putc(character);
            character = temp_character;
            ++input->pos;
        }

        for (size_t i = input->pos - 1; i < input->max_pos && i < NCSH_MAX_INPUT; ++i) {
            temp_character = character;
            character = input->buffer[i + 1];
            input->buffer[i + 1] = temp_character;
            tty_putc(temp_character);
            ++input->pos;
        }

        if (input->pos > input->max_pos) {
            input->max_pos = input->pos;
        }

        if (input->pos == input->max_pos) {
            input->buffer[input->pos] = '\0';
        }

        if (!input->pos || input->buffer[1] == '\0') {
            return EXIT_CONTINUE;
        }

        while (input->pos > input->start_pos + 1) {
            tty_send(&tcaps.cursor_left);
            --input->pos;
        }
    }
    else { // end of line insertions
        tty_putc(character);
        input->buffer[input->pos++] = character;

        if (input->pos >= input->max_pos) {
            input->max_pos = input->pos;
            input->buffer[input->pos] = '\0';
        }
    }

    io_adjust_line_if_needed(input);
    return EXIT_SUCCESS;
}

[[nodiscard]]
char io_read()
{
    char character = 0;
    if (read(STDIN_FILENO, &character, 1) < 0) {
        tty_perror(NCSH_ERROR_STDIN);
        return EXIT_IO_FAILURE;
    }

    switch (character) {
    case ESCAPE_CHARACTER: {
        if (read(STDIN_FILENO, &character, 1) < 0) {
            tty_perror(NCSH_ERROR_STDIN);
            return EXIT_IO_FAILURE;
        }

        if (character == '[') {
            if (read(STDIN_FILENO, &character, 1) < 0) {
                tty_perror(NCSH_ERROR_STDIN);
                return EXIT_IO_FAILURE;
            }

            return character;
        }

        break;
    }
    case KEY_NEWLINE:
    case KEY_CARRIAGE_RETURN: {
        return character;
    }
    }

    return '\0';
}

[[nodiscard]]
int io_autocompletions_select_from(Input* restrict input)
{
    tty_send(&tcaps.line_clr_to_eol);
    tty_send(&tcaps.cursor_hide);
    tty_send(&tcaps.newline);

    Autocompletion autocompletion_matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint8_t ac_matches_count = ac_get(input->buffer, autocompletion_matches, input->autocompletions_tree, *input->scratch);

    if (!ac_matches_count) {
        tty_send(&tcaps.cursor_show);
        return EXIT_SUCCESS;
    }

    if (input->buffer) {
        for (int i = 0; i < ac_matches_count; ++i) {
            tty_println("%s%s", input->buffer, autocompletion_matches[i].value);
        }
    }
    else {
        for (int i = 0; i < ac_matches_count; ++i) {
            tty_println("%s", autocompletion_matches[i].value);
        }
    }

    tty_send_n(&tcaps.cursor_up, ac_matches_count);
    tty_send(&tcaps.cursor_show);

    size_t position = 0;
    char character;

    int exit = EXIT_SUCCESS;
    bool continue_input = true;
    while (continue_input) {
        if ((character = io_read()) == EXIT_IO_FAILURE) {
            return EXIT_FAILURE;
        }
        switch (character) {
        case UP_ARROW: {
            if (!position) {
                break;
            }
            tty_send(&tcaps.cursor_up);
            --position;
            break;
        }
        case DOWN_ARROW: {
            if (position == (size_t)(ac_matches_count - 1)) {
                break;
            }
            tty_send(&tcaps.cursor_down);
            ++position;
            break;
        }
        case '\n':
        case '\r': {
            continue_input = false;
            size_t length = strlen(autocompletion_matches[position].value) + 1;
            if (input->pos + length > NCSH_MAX_INPUT) {
                goto failure;
            }
            char* input_buf = input->buffer + input->pos;
            if (!input_buf) {
                goto failure;
            }
            memcpy(input->buffer + input->pos, autocompletion_matches[position].value, length);
            input->pos += length;
            exit = EXIT_SUCCESS_EXECUTE;
            break;
        }
        default: {
            continue_input = false;
            break;
        }
        }
    }

    tty_send_n(&tcaps.cursor_down, ac_matches_count + 1 - position);
    if (input->buffer && exit == EXIT_SUCCESS_EXECUTE) {
        tty_color_set(AUTOCOMPLETE_YELLOW);
        tty_println("%s", input->buffer);
        tty_color_reset();
    }

    return exit;

failure:
    return EXIT_FAILURE;
}

[[nodiscard]]
int io_autocompletions_select(Input* restrict input)
{
    int rv = io_autocompletions_select_from(input);
    if (rv != EXIT_SUCCESS) {
        return rv;
    }

    input->reprint_prompt = true;
    input->buffer[0] = '\0';
    input->pos = 0;
    input->max_pos = 0;
    return EXIT_CONTINUE;
}

/* io_backspace
 * Handles backspace key input in any position, end of line, midline, start of line.
 * Adjusts buffer and buffer position which holds user input.
 */
[[nodiscard]]
int io_backspace(Input* restrict input)
{
    if (!input->pos) {
        return EXIT_SUCCESS;
    }

    input->current_autocompletion[0] = '\0';

    --input->pos;
    if (input->max_pos > 0) {
        --input->max_pos;
    }

    tty_send(&tcaps.bs);
    tty_send(&tcaps.line_clr_to_eol);

    input->start_pos = input->pos;
    memmove(input->buffer + input->pos, input->buffer + input->pos + 1, input->max_pos);
    input->buffer[input->max_pos] = '\0';

    while (input->buffer[input->pos] != '\0') {
        ++input->pos;
    }
    tty_write(&input->buffer[input->start_pos], input->pos - input->start_pos);

    while (input->pos > input->start_pos) {
        if (!input->pos || !input->buffer[input->pos - 1]) {
            break;
        }

        tty_send(&tcaps.cursor_left);
        --input->pos;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_eol(Input* restrict input)
{
    if (!input->pos && !input->buffer[input->pos]) {
        input->reprint_prompt = true;
        tty_send(&tcaps.cursor_hide); // cursor is shown again after prompt is printed
        tty_send(&tcaps.newline);
        return EXIT_CONTINUE;
    }

    tty_send(&tcaps.cursor_hide);
    while (input->pos < input->max_pos && input->buffer[input->pos]) {
        ++input->pos;
        tty_send(&tcaps.cursor_right);
    }

    while (input->pos > 1 && input->buffer[input->pos - 1] == ' ') {
        --input->pos;
    }
    ++input->pos;

    tty_send(&tcaps.line_clr_to_eol);
    tty_send(&tcaps.newline);
    tty_send(&tcaps.cursor_show);
    return EXIT_SUCCESS_EXECUTE;
}

/* io_autocomplete_select
 * render the current autocompletion.
 */
[[nodiscard]]
int io_autocompletion_render(Input* restrict input)
{
    tty_print("%s", input->current_autocompletion);
    for (size_t i = 0; input->current_autocompletion[i] != '\0'; i++) {
        input->buffer[input->pos] = input->current_autocompletion[i];
        ++input->pos;
    }
    input->buffer[input->pos] = '\0';

    if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }

    input->current_autocompletion[0] = '\0';
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_cursor_right(Input* restrict input)
{
    if (input->pos == NCSH_MAX_INPUT - 1 || (!input->buffer[input->pos] && !input->buffer[input->pos + 1])) {
        return EXIT_SUCCESS;
    }

    tty_send(&tcaps.cursor_right);

    ++input->pos;
    if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }

    return EXIT_SUCCESS;
}

/* io_right_arrow
 * Render autocompletion if one is available and at EOL,
 * Move cursor right if not.
 */
[[nodiscard]]
int io_right_arrow(Input* restrict input)
{
    if (!input->pos && !input->max_pos) {
            return EXIT_CONTINUE;
    }

    if (input->pos == input->max_pos && input->buffer[0]) {
        if (io_autocompletion_render(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (io_cursor_right(input) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* io_left_arrow
 * Move cursor left.
 */
[[nodiscard]]
int io_left_arrow_cursor(Input* restrict input)
{
    if (!input->pos || (!input->buffer[input->pos] && !input->buffer[input->pos - 1])) {
        return EXIT_CONTINUE;
    }

    tty_send(&tcaps.cursor_left);
    --input->pos;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_history_prev(Input* restrict input)
{
    input->history_entry = history_get(input->history_position, &input->history);
    if (input->history_entry.length > 0) {
        ++input->history_position;
        io_cursor_restore(input);
        tty_send(&tcaps.line_clr_to_eol);

        input->pos = input->history_entry.length - 1;
        input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        tty_write(input->buffer, input->pos);
    }
    else {
        input->buffer[0] = '\0';
        input->pos = 0;
        input->max_pos = 0;
    }

    io_adjust_line_if_needed(input);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_history_next(Input* restrict input)
{
    input->history_entry = history_get(input->history_position - 2, &input->history);

    io_cursor_restore(input);
    tty_send(&tcaps.line_clr_to_eol);

    if (input->history_entry.length > 0) {
        --input->history_position;
        input->pos = input->history_entry.length - 1;
        input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        tty_write(input->buffer, input->pos);
    }
    else {
        input->buffer[0] = '\0';
        input->pos = 0;
        input->max_pos = 0;
    }

    io_adjust_line_if_needed(input);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_history_beginning(Input* restrict input)
{
    input->history_position = input->history.count - 2;
    input->history_entry = history_get(input->history_position, &input->history);

    io_cursor_restore(input);
    tty_send(&tcaps.line_clr_to_eol);

    if (input->history_entry.length > 0) {
        input->pos = input->history_entry.length - 1;
        input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        tty_write(input->buffer, input->pos);
    }
    else {
        input->buffer[0] = '\0';
        input->pos = 0;
        input->max_pos = 0;
    }

    io_adjust_line_if_needed(input);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_history_end(Input* restrict input)
{
    input->history_position = 0;
    input->history_entry = history_get(0, &input->history);

    io_cursor_restore(input);
    tty_send(&tcaps.line_clr_to_eol);

    if (input->history_entry.length > 0) {
        input->pos = input->history_entry.length - 1;
        input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        tty_write(input->buffer, input->pos);
    }
    else {
        input->buffer[0] = '\0';
        input->pos = 0;
        input->max_pos = 0;
    }

    io_adjust_line_if_needed(input);
    return EXIT_SUCCESS;
}

/* io_delete
 * Handles delete key input in any position, end of line, midline, start of line.
 * Adjusts buffer and buffer position which holds user input.
 * Returns: EXIT_SUCCESS OR EXIT_FAILURE
 */
[[nodiscard]]
int io_delete(Input* restrict input)
{
    if (read(STDIN_FILENO, &input->c, 1) < 0) {
        tty_perror(NCSH_ERROR_STDIN);
        return EXIT_FAILURE;
    }
    if (input->c != DELETE_KEY) {
        return EXIT_CONTINUE;
    }

    tty_send(&tcaps.del);
    tty_send(&tcaps.line_clr_to_eol);

    input->start_pos = input->pos;
    memmove(input->buffer + input->pos, input->buffer + input->pos + 1, input->max_pos);

    if (input->max_pos > 0) {
        --input->max_pos;
    }

    while (input->pos < input->max_pos && input->buffer[input->pos]) {
        ++input->pos;
    }
    tty_write(&input->buffer[input->start_pos], input->pos - input->start_pos);

    if (!input->pos) {
        return EXIT_SUCCESS;
    }

    while (input->pos > input->start_pos && input->pos != 0 && input->buffer[input->pos - 1]) {
        tty_send(&tcaps.cursor_left);
        --input->pos;
    }

    if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_exit_if_empty_or_del(Input* restrict input)
{
    if (!input->pos) {
        return io_exit(input);
    }

    return io_delete(input);
}

[[nodiscard]]
int io_cursor_home(Input* restrict input)
{
    if (!input->pos) {
        return EXIT_CONTINUE;
    }

    io_cursor_restore(input);
    input->pos = 0;
    input->current_y = 0;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_cursor_end(Input* restrict input)
{
    if (input->pos == input->max_pos) {
        return EXIT_CONTINUE;
    }

    if (input->lines_y > input->current_y) {
        input->current_y = input->lines_y - input->current_y;
        assert(input->current_y > 0);
        tty_send_n(&tcaps.cursor_down, input->current_y);
        tty_send(&tcaps.line_goto_bol);
        tty_send_n(&tcaps.cursor_right, term.size.x - term.pos.x - 1);
        tty_send(&tcaps.cursor_show);
        input->pos = input->max_pos;
        return EXIT_SUCCESS;
    }

    while (input->buffer[input->pos]) {
        tty_send(&tcaps.cursor_right);
        ++input->pos;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_cursor_forwards(Input* restrict input)
{
    if (input->pos == input->max_pos) {
        return EXIT_SUCCESS;
    }

    tty_send(&tcaps.cursor_right);
    ++input->pos;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_cursor_backwards(Input* restrict input)
{
    if (input->pos == 0) {
        return EXIT_SUCCESS;
    }

    tty_send(&tcaps.cursor_left);
    --input->pos;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_forwards(Input* restrict input)
{
    if (!input->pos && !input->max_pos) {
        return EXIT_SUCCESS;
    }

    while (input->pos < input->max_pos) {
        tty_send(&tcaps.cursor_right);
        ++input->pos;

        switch (input->buffer[input->pos])
        {
        case ' ': return EXIT_SUCCESS;
        case '\\': return EXIT_SUCCESS;
        case '/': return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_backwards(Input* restrict input)
{
    if (!input->pos && !input->max_pos) {
        return EXIT_SUCCESS;
    }

    while (input->pos > 0) {
        tty_send(&tcaps.cursor_left);
        --input->pos;

        switch (input->buffer[input->pos])
        {
        case ' ': return EXIT_SUCCESS;
        case '\\': return EXIT_SUCCESS;
        case '/': return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int io_clear(Input* restrict input)
{
    tty_send(&tcaps.scr_clr);
    input->reprint_prompt = true;
    return EXIT_SUCCESS;
}

enum io_Type : uint_fast8_t {
    IO_ERROR = 0,
    IO_CHAR,
    IO_CTRL_A,              // home
    IO_CTRL_B,              // backwards
    IO_CTRL_C,              // exit
    IO_CTRL_D,              // exit if empty, delete if not
    IO_CTRL_E,              // end
    IO_CTRL_F,              // forwards
    IO_CTRL_H,              // bs
    IO_CTRL_K,              // delete line backwards
    IO_CTRL_N,              // next history
    IO_CTRL_P,              // previous history
    IO_CTRL_L,              // clear
    IO_CTRL_U,              // delete line forwards
    IO_CTRL_W,              // delete word
    IO_TAB,                 // ac select
    IO_BS,
    IO_ESCAPE_CHAR,
    IO_CARRIAGE_RETURN,
    IO_NEWLINE,
    IO_TYPE_END
};

enum io_Escaped_Type : uint_fast8_t {
    IO_RIGHT_ARROW = IO_TYPE_END,
    IO_LEFT_ARROW,
    IO_UP_ARROW,
    IO_DOWN_ARROW,
    IO_DEL,
    IO_HOME,
    IO_END,
    IO_ALT_LT,                    // beginning of history
    IO_ALT_GT,                    // end of history
    IO_ALT_F,                     // move cursor forwards
    IO_ALT_B,                     // move cursor backwards
};

typedef int (*io_func)(Input* restrict);
int io_next_escaped(Input* restrict input);

io_func io_funcs[] = {
    io_error,
    io_char,
    io_cursor_home,            // CTRL_A
    io_backwards,              // CTRL_B
    io_exit,                   // CTRL_C
    io_exit_if_empty_or_del,   // CTRL_D
    io_cursor_end,             // CTRL_E
    io_forwards,               // CTRL_F
    io_backspace,              // CTRL_H
    io_line_delete_backwards,  // CTRL_K
    io_history_next,           // CTRL_N
    io_history_prev,           // CTRL_P
    io_clear,                  // CTRL_L
    io_line_delete_forwards,   // CTRL_U
    io_word_delete,            // CTRL_W
    io_autocompletions_select, // TAB
    io_backspace,              // BS
    io_next_escaped,           // ESCAPE CHARACTER
    io_eol,                    // \r
    io_eol,                    // \n
    io_right_arrow,            // RIGHT_ARROW
    io_left_arrow_cursor,      // LEFT_ARROW
    io_history_prev,           // UP_ARROW
    io_history_next,           // DOWN_ARROW
    io_delete,                 // DELETE
    io_cursor_home,            // HOME
    io_cursor_end,             // END
    io_history_beginning,      // ALT-<
    io_history_end,            // ALT->
    io_cursor_forwards,        // ALT-f
    io_cursor_backwards,       // ALT-b
};

int io_next_escaped(Input* restrict input)
{
    if (read(STDIN_FILENO, &input->c, 1) < 0) {
        tty_perror(NCSH_ERROR_STDIN);
        return EXIT_FAILURE;
    }

    if (input->c != '[') {
        switch (input->c) {
            case 'b': return io_funcs[IO_ALT_B](input);
            case 'f': return io_funcs[IO_ALT_F](input);
            case '<': return io_funcs[IO_ALT_LT](input);
            case '>': return io_funcs[IO_ALT_GT](input);
            default: return EXIT_SUCCESS;
        }
    }

    if (read(STDIN_FILENO, &input->c, 1) < 0) {
        tty_perror(NCSH_ERROR_STDIN);
        return EXIT_FAILURE;
    }

    switch (input->c) {
        case RIGHT_ARROW: return io_funcs[IO_RIGHT_ARROW](input);
        case LEFT_ARROW: return io_funcs[IO_LEFT_ARROW](input);
        case UP_ARROW: return io_funcs[IO_UP_ARROW](input);
        case DOWN_ARROW: return io_funcs[IO_DOWN_ARROW](input);
        case DELETE_PREFIX_KEY: return io_funcs[IO_DEL](input);
        case HOME_KEY: return io_funcs[IO_HOME](input);
        case END_KEY: return io_funcs[IO_END](input);
        default: return EXIT_SUCCESS;
    }
}

int io_next(Input* restrict input)
{
    if (read(STDIN_FILENO, &input->c, 1) < 0) {
        tty_perror(NCSH_ERROR_STDIN);
        return EXIT_FAILURE_CONTINUE;
    }

    switch (input->c) {
        case CTRL_A: return io_funcs[IO_CTRL_A](input);
        case CTRL_B: return io_funcs[IO_CTRL_B](input);
        case CTRL_C: return io_funcs[IO_CTRL_C](input);
        case CTRL_D: return io_funcs[IO_CTRL_D](input);
        case CTRL_E: return io_funcs[IO_END](input);
        case CTRL_F: return io_funcs[IO_CTRL_F](input);
        case CTRL_H: return io_funcs[IO_CTRL_H](input);
        case CTRL_K: return io_funcs[IO_CTRL_K](input);
        case CTRL_N: return io_funcs[IO_CTRL_N](input);
        case CTRL_P: return io_funcs[IO_CTRL_P](input);
        case CTRL_L: return io_funcs[IO_CTRL_L](input);
        case CTRL_U: return io_funcs[IO_CTRL_U](input);
        case CTRL_W: return io_funcs[IO_CTRL_W](input);
        case TAB_KEY: return io_funcs[IO_TAB](input);
        case BACKSPACE_KEY: return io_funcs[IO_BS](input);
        case ESCAPE_CHARACTER: return io_funcs[IO_ESCAPE_CHAR](input);
        case KEY_CARRIAGE_RETURN: return io_funcs[IO_CARRIAGE_RETURN](input);
        case KEY_NEWLINE: return io_funcs[IO_NEWLINE](input);
        default: return io_funcs[IO_CHAR](input);
    }
}

int io_resize(Input* restrict input)
{
    // TODO need to reset saved cursor position as well on resize

    input->lines_y = 0;
    size_t len = input->pos;
    while (len) {
        if (len < term.size.x) {
            break;
        }
        ++input->lines_y;
        len -= term.size.x;
    }
    input->current_y = input->lines_y;

    return EXIT_SUCCESS;
}

void io_autocomplete(Input* restrict input)
{
    if (!input->buffer[0] || input->buffer[input->pos] != '\0') {
        return;
    }

    uint8_t ac_matches_count =
        ac_first(input->buffer, input->current_autocompletion, input->autocompletions_tree, *input->scratch);

    if (!ac_matches_count) {
        if (input->current_autocompletion[0] == '\0') {
            return;
        }

        tty_send(&tcaps.line_clr_to_eol);
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return;
    }

    input->current_autocompletion_len = strlen(input->current_autocompletion);
    if (input->current_y == 0 &&
        input->prompt_len + term.pos.x + input->current_autocompletion_len >
            term.size.x) {
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return;
    }
    else if (term.pos.x + input->current_autocompletion_len >
             term.size.x) {
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return;
    }

    tty_send(&tcaps.line_clr_to_eol);
    tty_color_set(AUTOCOMPLETE_DIM);
    tty_print("%s", input->current_autocompletion);
    tty_color_reset();
    tty_send_n(&tcaps.cursor_left, input->current_autocompletion_len);
}

int io_readline(Input* restrict input, Arena* restrict scratch)
{
    input->scratch = scratch;
    input->reprint_prompt = true;
    int rv = EXIT_SUCCESS;
    tty_init_input_mode(TTY_NONCANONICAL_MODE);

    while (1) {
        if (prompt_if_needed(input) != EXIT_SUCCESS) {
            rv = EXIT_FAILURE;
            break;
        }

        rv = io_next(input);
        if (rv == EXIT_CONTINUE) {
            continue;
        }
        else if (rv != EXIT_SUCCESS) {
            break;
        }

        if (sigwinch_caught) {
            io_resize(input);
            sigwinch_caught = 0;
        }

        io_autocomplete(input);
    }

    tty_deinit_input_mode();
    return rv;
}
