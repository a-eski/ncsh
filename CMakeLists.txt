cmake_minimum_required(VERSION 3.31.3)

project(ncsh)

add_executable(ncsh src/ncsh.c src/ncsh_vm.c src/ncsh_terminal.c src/eskilib/eskilib_string.c src/eskilib/eskilib_file.c src/ncsh_parser.c src/ncsh_builtins.c src/ncsh_history.c src/ncsh_autocompletions.c src/ncsh_config.c src/z/z.c)

set(CMAKE_C_STANDARD 99)

