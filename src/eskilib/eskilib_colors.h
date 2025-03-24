/* Copyright eskilib by Alex Eski 2024 */

#pragma once

/*
Escape Key
    Ctrl-Key: ^[
    Octal: \033
    Unicode: \u001b
    Hexadecimal: \x1B
    Decimal: 27
*/

#define RESET "\033[0m"

#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define DEFAULT "\033[39m"

#define WHITE_DIM "\033[2m"

#define BLACK_BG "\033[40"
#define RED_BG "\033[41m"
#define GREEN_BG "\033[42m"
#define YELLOW_BG "\033[43m"
#define BLUE_BG "\033[44m"
#define MAGENTA_BG "\033[45m"
#define CYAN_BG "\033[46m"
#define WHITE_BG "\033[47m"
#define DEFAULT_BG "\033[49m"

#define BLACK_BRIGHT "\033[90"
#define RED_BRIGHT "\033[91m"
#define GREEN_BRIGHT "\033[92m"
#define YELLOW_BRIGHT "\033[93m"
#define BLUE_BRIGHT "\033[94m"
#define MAGENTA_BRIGHT "\033[95m"
#define CYAN_BRIGHT "\033[96m"
#define WHITE_BRIGHT "\033[97m"

#define ncsh_GREEN "\033[38;5;46m"
#define ncsh_CYAN "\033[38;5;36m"
#define ncsh_BLUE "\033[38;5;39m"
#define ncsh_PURPLE "\033[38;5;57m"
#define ncsh_YELLOW "\033[38;5;226m"

#define ncsh_MAGENTA "\033[38;2;200;8;134m"
#define ncsh_TURQUOISE "\033[38;2;0;225;154m"
#define ncsh_INDIGO "\033[38;2;66;6;84m"
#define ncsh_BLACK "\033[38;2;2;3;20m"
