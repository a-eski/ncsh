cmake_minimum_required(VERSION 3.28.3)

project(ncsh)

add_executable(ncsh src/main.c src/ncsh.c src/ncsh_noninteractive.c src/ncsh_readline.c src/ncsh_vm.c src/ncsh_terminal.c src/eskilib/eskilib_string.c src/eskilib/eskilib_file.c src/eskilib/eskilib_hashtable.c src/ncsh_parser.c src/ncsh_builtins.c src/ncsh_history.c src/ncsh_autocompletions.c src/ncsh_config.c src/z/z.c src/z/fzf.c)

set(CMAKE_C_STANDARD 23)
target_compile_options(ncsh PUBLIC -Wall -Wsign-conversion -Wformat=2 -Wshadow -Wvla -O3 -DNDEBUG)
