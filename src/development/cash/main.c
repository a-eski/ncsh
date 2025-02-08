// ncurses experiment

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <linux/limits.h>
#include <ncurses.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#if __has_c_attribute(nodiscard)
#define cash_no_discard [[nodiscard]]
#else
#define cash_no_discard
#endif

#define CASH_MAX_INPUT 528

#define PAIR_NULL 0 // don't use
#define PAIR_USER 1
#define PAIR_DIRECTORY 2
#define PAIR_AUTOCOMPLETE 3
#define PAIR_ERROR 4

#define printw_color(fmt, str, color_pair)                                                                             \
    attron(COLOR_PAIR(color_pair));                                                                                    \
    printw(fmt, str);                                                                                                  \
    attroff(COLOR_PAIR(color_pair))

struct ncsh_Directory
{
    bool reprint_prompt;
    char *user;
    char path[PATH_MAX];
};

void cash_perror(const char *msg)
{
    char err_msg[256];

    // Save the current cursor position
    int y, x;
    getyx(stdscr, y, x);

    // Move to the bottom left corner of the screen
    move(LINES - 1, 0);

    // Print the user-provided message, if any
    if (msg)
        printw_color("%s: ", msg, PAIR_ERROR);

    // Get the error message from errno and print it
    snprintf(err_msg, sizeof(err_msg), "%s", strerror(errno));
    printw("%s", err_msg);

    // Refresh the screen to display the message
    refresh();

    // Restore the original cursor position
    move(y, x);
}

void cash_clear_perror()
{
    // Save the current cursor position
    int y, x;
    getyx(stdscr, y, x);

    // Move to the bottom left corner of the screen
    move(LINES - 1, 0);

    clrtoeol();

    // Refresh the screen to display the message
    refresh();

    // Restore the original cursor position
    move(y, x);
}

void ncsh_prompt_directory(char *cwd, char *output)
{
    uint_fast32_t i = 1;
    uint_fast32_t last_slash_pos = 0;
    uint_fast32_t second_to_last_slash = 0;

    while (cwd[i] != '\n' && cwd[i] != '\0')
    {
        if (cwd[i] == '/')
        {
            second_to_last_slash = last_slash_pos;
            last_slash_pos = i + 1;
        }
        ++i;
    }

    memcpy(output, &cwd[second_to_last_slash] - 1, i - second_to_last_slash + 1);
}

cash_no_discard int_fast32_t prompt(struct ncsh_Directory *prompt_info)
{
    char *wd_result = getcwd(prompt_info->path, sizeof(prompt_info->path));
    if (wd_result == NULL)
    {
        cash_perror("cash: Could not get current working directory");
        return EXIT_FAILURE;
    }

    printw_color("%s ", prompt_info->user, PAIR_USER);
    printw_color("%s ", prompt_info->path, PAIR_DIRECTORY);
    /*attron(COLOR_PAIR(PAIR_USER));
    printw("%s ", prompt_info->user);
    attroff(COLOR_PAIR(PAIR_USER));
    attron(COLOR_PAIR(PAIR_DIRECTORY));
    printw("%s ", prompt_info->path);
    attroff(COLOR_PAIR(PAIR_DIRECTORY));*/

    return EXIT_SUCCESS;
}

void cash_init_color(void)
{
    start_color();
    init_pair(PAIR_ERROR, COLOR_RED, COLOR_BLACK);
    init_pair(PAIR_USER, COLOR_GREEN, COLOR_BLACK);
    init_pair(PAIR_DIRECTORY, COLOR_CYAN, COLOR_BLACK);
    init_pair(PAIR_AUTOCOMPLETE, COLOR_WHITE | COLOR_BLACK, COLOR_BLACK);
}

void cash_init(void)
{
    initscr();
    // reset_prog_mode();
    // reset_shell_mode();
    raw();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(1);
    scrollok(stdscr, TRUE);
    cash_init_color();
    clear();
}

void cash_exit(void)
{
    endwin();
}

// addch(ch | A_BOLD | A_UNDERLINE) aka waddch(stdscrn, ch | A_BOLD | A_UNDERLINE)
// printw (works like printf)
// addstr (works like print)

void cash_vm(char *buffer)
{
    pid_t pid = fork();

    if (pid == -1)
        return;

    if (pid == 0)
    {
	char** args = malloc(sizeof(char*));
	args[0] = buffer;
	printw("%s\n", args[0]);
        if (execvp(args[0], args) == -1)
        {
            kill(getpid(), SIGTERM);
        }

        int result;
        while (1)
        {
            int status = 0;
            result = waitpid(pid, &status, WUNTRACED);

            // check for errors
            if (result == -1)
            {
                /* ignore EINTR, occurs when SA_RESTART is not specified in sigaction flags */
                if (errno == EINTR)
                    continue;

                status = EXIT_FAILURE;
                break;
            }

            // check if child process has exited
            if (result == pid)
            {
                if (WIFEXITED(status))
                {
                    if (WEXITSTATUS(status))
                        printw("ncsh: Command child process failed with status %d\n", WEXITSTATUS(status));
                    else
                        printw("ncsh: Command child process exited successfully.\n");
                }
                else if (WIFSIGNALED(status))
                {
                    printw("ncsh: Command child process died from signal #%d\n", WTERMSIG(status));
                }
                else
                {
                    if (write(STDERR_FILENO, "ncsh: Command child process died, cause unknown.\n", 49) == -1)
                    {
                        cash_perror("ncsh: Error writing to stderr");
                    }
                }

                break;
            }
        }
    }
}

void handle_input(char *buffer, struct ncsh_Directory *prompt_info)
{
    size_t pos = 0;
    int character;

    printw("cash shell\n");
    cash_perror("cash: Perror test");
    refresh();

    while (1)
    {
        if (prompt(prompt_info) != EXIT_SUCCESS)
        {
            break;
        }
        refresh();

        while ((character = getch()) != '\004')
        {
            if (character == KEY_BACKSPACE)
            { // Handle backspace
                if (pos > 0)
                {
                    pos--;
                    move(getcury(stdscr), getcurx(stdscr) - 1);
                    delch();
                    buffer[pos] = '\0';
                }
            }
            else if (character == KEY_LEFT)
            { // Handle left arrow
                if (pos > 0)
                {
                    pos--;
                    move(getcury(stdscr), getcurx(stdscr) - 1);
                }
            }
            else if (character == KEY_RIGHT)
            { // Handle right arrow
                if (pos < strlen(buffer))
                {
                    pos++;
                    move(getcury(stdscr), getcurx(stdscr) + 1);
                }
            }
            else if (character == KEY_DC)
            { // Handle delete
                if (pos < strlen(buffer))
                {
                    delch();
                    memmove(&buffer[pos], &buffer[pos + 1], strlen(buffer) - pos);
                }
            }
            else if (character == '\n' || character == '\r')
            {
                move(getcury(stdscr), getcurx(stdscr) + 1);
                break;
            }
            else
            { // Handle other characters
                if (pos < CASH_MAX_INPUT - 1)
                {
                    insch(character);
                    move(getcury(stdscr), getcurx(stdscr) + 1);
                    buffer[pos++] = character;
                }
            }
            refresh();
        }

        if (strcmp(buffer, "exit") == 0)
        {
            break;
        }

        if (buffer && !buffer[0]) {
            system(buffer);
            // move(getcury(stdscr) + 1, getcurx(stdscr) - 1000);
            refresh();
            addch('\n');
            refresh();
        }
        else {
            // move(getcury(stdscr) + 1, getcurx((stdscr)));
            addch('\n');
            refresh();
        }

        cash_clear_perror();
        pos = 0;
        memset(buffer, '\0', CASH_MAX_INPUT);
    }
}

int main(void)
{
    struct ncsh_Directory prompt_info = {0};
    prompt_info.user = getenv("USER");
    char *buffer = calloc(sizeof(char), CASH_MAX_INPUT);
    cash_init();

    handle_input(buffer, &prompt_info);

    cash_exit();
    return 0;
}
