/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <linux/limits.h>
#include <stddef.h>
#include <unistd.h>

#include "../defines.h" // used for macros
#include "prompt.h"

#define USER_COLOR 147
#define DIRECTORY_COLOR 10

static Prompt_Data prompt_data;

/* prompt_short_directory_get
 * gets a shortened version of the cwd, the last 2 directories in the cwd.
 * i.e. /home/alex/dir becomes /alex/dir
 * Returns: length of the shortened cwd
 */
[[nodiscard]]
size_t prompt_short_directory_get(char* restrict cwd, char* restrict output)
{
    assert(cwd);

    size_t i = 1;
    size_t last_slash_pos = 0;
    size_t second_to_last_slash_pos = 0;

    while (cwd[i] != '\n' && cwd[i] != '\0') {
        if (cwd[i] == '/') {
            second_to_last_slash_pos = last_slash_pos;
            last_slash_pos = i + 1;
        }
        ++i;
    }

    if (second_to_last_slash_pos != 0) { // has at least 3 slashes
        memcpy(output, cwd + second_to_last_slash_pos - 1, i - second_to_last_slash_pos + 1);
        output[i - second_to_last_slash_pos + 1] = '\0';
        return i - second_to_last_slash_pos + 2; // null termination included in len
    }

    if (last_slash_pos == 0) { // 1 slash at beginning of cwd
        memcpy(output, cwd, i);
        return i;
    }

    memcpy(output, cwd + last_slash_pos - 1, i - last_slash_pos + 1); // has 2 slashes
    return i - last_slash_pos + 2;                                    // null termination not included in len
}

[[nodiscard]]
Str prompt_get_short_directory(Input* restrict input, Arena* restrict scratch)
{
    char cwd[PATH_MAX] = {0};
    char directory[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        return Str_Empty;
    }

    size_t dir_len = prompt_short_directory_get(cwd, directory);
    assert(strlen(directory) + 1 == dir_len);

    Str_Builder* sb = sb_new(scratch);
    if (prompt_data.show_user) {
        sb_add(&Str_New_Literal("\033[38;5;147m"), sb, scratch);
        sb_add(input->user, sb, scratch);
        sb_add(&Str_New_Literal(" "), sb, scratch);
    }

    sb_add(&Str_New_Literal("\033[92m"), sb, scratch);
    sb_add(&Str_New(directory, dir_len), sb, scratch);
    sb_add(&Str_New_Literal("\033[0m"), sb, scratch);
    sb_add(&Str_New_Literal(NCSH_PROMPT_ENDING_STRING), sb, scratch);
    return *sb_to_str(sb, scratch);
}

[[nodiscard]]
Str prompt_get_directory(Input* restrict input, Arena* restrict scratch)
{
    char cwd[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        return Str_Empty;
    }

    Str_Builder* sb = sb_new(scratch);
    if (prompt_data.show_user) {
        sb_add(&Str_New_Literal("\033[38;5;147m"), sb, scratch);
        sb_add(input->user, sb, scratch);
        sb_add(&Str_New_Literal(" "), sb, scratch);
    }

    sb_add(&Str_New_Literal("\033[92m"), sb, scratch);
    sb_add(&Str_Get(cwd), sb, scratch);
    sb_add(&Str_New_Literal("\033[0m"), sb, scratch);
    sb_add(&Str_New_Literal(NCSH_PROMPT_ENDING_STRING), sb, scratch);
    return *sb_to_str(sb, scratch);
}

[[nodiscard]]
Str prompt_get_no_directory(Input* restrict input, Arena* restrict scratch)
{
    if (!prompt_data.show_user) {
        return Str_New_Literal(NCSH_PROMPT_ENDING_STRING);
    }

    Str_Builder* sb = sb_new(scratch);
    sb_add(&Str_New_Literal("\033[38;5;147m"), sb, scratch);
    sb_add(input->user, sb, scratch);
    sb_add(&Str_New_Literal("\033[0m"), sb, scratch);
    sb_add(&Str_New_Literal(NCSH_PROMPT_ENDING_STRING), sb, scratch);
    return *sb_to_str(sb, scratch);
}

/* prompt
 * Prints the prompt based on the current prompt compile-time settings.
 * Returns: the length of the prompt.
 */
[[nodiscard]]
Str prompt_get(Input* restrict input, Arena* restrict scratch)
{
    switch (prompt_data.dir_type) {
        case DIR_SHORT:
            return prompt_get_short_directory(input, scratch);
        case DIR_NORMAL:
            return prompt_get_directory(input, scratch);
        case DIR_NONE:
            return prompt_get_no_directory(input, scratch);
        default:
            unreachable();
            return Str_Empty;
    }
}

void prompt_init()
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

    prompt_data.show_user = show_user;
    prompt_data.dir_type = dir_type;
}
