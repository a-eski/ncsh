/* Copyright eskilib by Alex Eski 2024 */

#ifndef eskilib_output_h
#define eskilib_output_h

#define RESET	"\x1B[0m"

#define BLACK		"\x1B[30m"
#define RED		"\x1B[31m"
#define GREEN		"\x1B[32m"
#define YELLOW		"\x1B[33m"
#define BLUE		"\x1B[34m"
#define MAGENTA		"\x1B[35m"
#define CYAN		"\x1B[36m"
#define WHITE		"\x1B[37m"
#define DEFAULT		"\x1B[39m"

#define WHITE_DIM "\033[2m"

#define BLACK_BG	"\x1B[40"
#define RED_BG		"\x1B[41m"
#define GREEN_BG	"\x1B[42m"
#define YELLOW_BG	"\x1B[43m"
#define BLUE_BG		"\x1B[44m"
#define MAGENTA_BG	"\x1B[45m"
#define CYAN_BG		"\x1B[46m"
#define WHITE_BG	"\x1B[47m"
#define DEFAULT_BG	"\x1B[49m"

#define BLACK_BRIGHT		"\x1B[90"
#define RED_BRIGHT		"\x1B[91m"
#define GREEN_BRIGHT		"\x1B[92m"
#define YELLOW_BRIGHT		"\x1B[93m"
#define BLUE_BRIGHT		"\x1B[94m"
#define MAGENTA_BRIGHT		"\x1B[95m"
#define CYAN_BRIGHT		"\x1B[96m"
#define WHITE_BRIGHT		"\x1B[97m"

// original ncsh color scheme
#define ncsh_GREEN		"\x1B[38;5;46m"
#define ncsh_CYAN		"\x1B[38;5;36m"
#define ncsh_BLUE		"\x1B[38;5;39m"
#define ncsh_PURPLE		"\x1B[38;5;57m"
#define ncsh_YELLOW		"\x1B[38;5;226m"

// ncsh color scheme
#define ncsh_MAGENTA "\033[38;2;200;8;134m"
#define ncsh_TURQUOISE "\033[38;2;0;150;154m"
#define ncsh_INDIGO "\033[38;2;66;6;84m"
#define ncsh_BLACK "\033[38;2;2;3;20m"

#endif // !eskilib_output_h

