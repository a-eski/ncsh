/* experiment with using curses to get terminfo */

#include <assert.h>
// #include <stdlib.h>
// #include <string.h>
#include <term.h>
#include <curses.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int result = setupterm((char*)0, STDOUT_FILENO, (int*)0);
    assert(!result);
    // enter_ca_mode;
    printf("%s\n", cur_term->type.term_names);
    printf("%s\n", CUR term_names);

    printf("Booleans: %s\n", cur_term->type.Booleans);
    printf("ext_str_table: %s\n", cur_term->type.ext_str_table);
    printf("str_table: %s\n", cur_term->type.str_table);

    printf("Strings[0]: %s\n", cur_term->type.Strings[0]);
    printf("delete key: %s\n", CUR Strings[59]); // Equivalent to tigetstr("key_dc")

    /*char* clear_screen_seq = tigetstr("clear");
    if (!clear_screen_seq)
        printf("found nothing");
    else
        printf("%s\n", clear_screen_seq);*/

    exit_ca_mode;
    /*reset_shell_mode();
    const char shell_name[] = "/bin/ncsh";
    char* shell_heap = malloc(sizeof(shell_name));
    memcpy(shell_heap, shell_name, sizeof(shell_name));
    char** args = { &shell_heap };
    execv(shell_heap, args);
    reset_prog_mode();
    exit_ca_mode;
    free(shell_heap);*/
}
