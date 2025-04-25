#pragma once

#ifdef NCSH_RL_TEST

extern char* ncrl_test_input;
extern size_t ncrl_test_input_pos;
extern char* ncrl_test_output;
extern size_t ncrl_test_output_pos;

int read_mock(int fd, char* character, int n) {
    *character = ncrl_test_input[ncrl_test_input_pos++];
    return 1;
}

int putchar_mock(char character) {
    ncrl_test_output[ncrl_test_output_pos++] = character;
    return 0;
}

#define printf(fmt, ...)
#define read(fd, character, n) read_mock(fd, character, n)
#define putchar(character) putchar_mock(character)
#define write(fd, str, len) 1

#endif /* NCSH_RL_TEST */
