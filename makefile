STD = c2x
std = -std=$(STD)
CC ?= gcc
DESTDIR ?= /usr/local
RELEASE ?= 1
debug_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -g
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -O3 -DNDEBUG
fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,leak,fuzzer -DNDEBUG -g
objects = obj/main.o obj/ncsh.o obj/ncsh_noninteractive.o obj/ncsh_vm.o obj/ncsh_terminal.o obj/eskilib_string.o obj/eskilib_file.o obj/ncsh_parser.o obj/ncsh_builtins.o obj/ncsh_history.o obj/ncsh_autocompletions.o obj/ncsh_config.o obj/z.o
# obj/ncsh_arena.o
target = bin/ncsh

ifeq ($(CC), gcc)
	release_flags += -s
endif

ifeq ($(RELEASE), 1)
	CFLAGS ?= $(release_flags)
	cc_with_flags = $(CC) $(std) $(CFLAGS)
else
	CFLAGS ?= $(debug_flags)
	cc_with_flags = $(CC) $(std) $(CFLAGS)
endif

$(target) : $(objects)
	$(cc_with_flags) -o $(target) $(objects)

obj/main.o : src/ncsh.h src/ncsh_noninteractive.h
	$(cc_with_flags) -c src/main.c -o obj/main.o
obj/ncsh.o : src/ncsh.c src/ncsh_vm.h src/ncsh_terminal.h src/eskilib/eskilib_string.h src/eskilib/eskilib_colors.h src/ncsh_parser.h src/ncsh_autocompletions.h
	$(cc_with_flags) -c src/ncsh.c -o obj/ncsh.o
obj/ncsh_noninteractive.o : src/ncsh_noninteractive.c src/ncsh_parser.h
	$(cc_with_flags) -c src/ncsh_noninteractive.c -o obj/ncsh_noninteractive.o
obj/ncsh_vm.o : src/ncsh_vm.h src/eskilib/eskilib_string.h src/eskilib/eskilib_colors.h src/ncsh_terminal.h src/ncsh_builtins.h src/ncsh_parser.h
	$(cc_with_flags) -c src/ncsh_vm.c -o obj/ncsh_vm.o
obj/ncsh_builtins.o : src/ncsh_builtins.h src/eskilib/eskilib_string.h
	$(cc_with_flags) -c src/ncsh_builtins.c -o obj/ncsh_builtins.o
obj/ncsh_history.o : src/ncsh_history.h src/eskilib/eskilib_string.h src/eskilib/eskilib_file.h
	$(cc_with_flags) -c src/ncsh_history.c -o obj/ncsh_history.o
obj/ncsh_autocompletions.o : src/ncsh_autocompletions.h src/eskilib/eskilib_string.h
	$(cc_with_flags) -c src/ncsh_autocompletions.c -o obj/ncsh_autocompletions.o
obj/ncsh_terminal.o : src/ncsh_terminal.c src/ncsh_terminal.h
	$(cc_with_flags) -c src/ncsh_terminal.c -o obj/ncsh_terminal.o
obj/ncsh_parser.o : src/ncsh_parser.c src/ncsh_parser.h src/eskilib/eskilib_string.h
	$(cc_with_flags) -c src/ncsh_parser.c -o obj/ncsh_parser.o
obj/ncsh_config.o : src/ncsh_config.c src/ncsh_config.h
	$(cc_with_flags) -c src/ncsh_config.c -o obj/ncsh_config.o
obj/z.o : src/z/z.c src/z/z.h src/eskilib/eskilib_string.h
	$(cc_with_flags) -c src/z/z.c -o obj/z.o
obj/eskilib_string.o : src/eskilib/eskilib_string.c src/eskilib/eskilib_string.h
	$(cc_with_flags) -c src/eskilib/eskilib_string.c -o obj/eskilib_string.o
obj/eskilib_file.o : src/eskilib/eskilib_file.c src/eskilib/eskilib_file.h
	$(cc_with_flags) -c src/eskilib/eskilib_file.c -o obj/eskilib_file.o
# obj/ncsh_arena.o : src/ncsh_arena.c src/ncsh_arena.h
	# $(cc_with_flags) -c src/ncsh_arena.c -o obj/ncsh_arena.o

.PHONY: debug
debug :
	make -B RELEASE=0

.PHONY: unity
unity :
	$(CC) $(std) $(release_flags) src/unity.c -o $(target)

.PHONY: unity_debug
unity_debug :
	$(CC) $(std) $(debug_flags) src/unity.c -o $(target)

.PHONY: install
install : $(target)
	install -C $(target) $(DESTDIR)

.PHONY: check
check :
	set -e
	make test_autocompletions
	make test_history
	make test_parser
	make test_z

.PHONY: l
l :
	set -e
	make check
	chmod +x ./tests_z.sh
	./tests_z.sh
	chmod +x ./tests_it.sh
	./tests_it.sh

.PHONY: test_history
test_history :
	$(CC) $(std) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/ncsh_history.c ./src/tests/ncsh_history_tests.c -o ./bin/ncsh_history_tests
	./bin/ncsh_history_tests

.PHONY: fuzz_history
fuzz_history :
	clang $(std) $(fuzz_flags) -DNCSH_HISTORY_TEST ./src/tests/ncsh_history_fuzzing.c ./src/ncsh_history.c ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_file.c
	./a.out NCSH_HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: test_autocompletions
test_autocompletions :
	 $(CC) $(std) $(debug_flags) ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_autocompletions.c ./src/tests/ncsh_autocompletions_tests.c -o ./bin/ncsh_autocompletions_tests
	 ./bin/ncsh_autocompletions_tests

.PHONY: fuzz_autocompletions
fuzz_autocompletions :
	clang $(std) $(fuzz_flags) ./src/tests/ncsh_autocompletions_fuzzing.c ./src/ncsh_autocompletions.c ./src/eskilib/eskilib_string.c
	./a.out NCSH_AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192

.PHONY test_parser :
	$(CC) $(std) $(debug_flags) ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_parser.c ./src/tests/ncsh_parser_tests.c -o ./bin/ncsh_parser_tests
	./bin/ncsh_parser_tests

.PHONY: fuzz_parser
fuzz_parser :
	clang $(std) $(fuzz_flags) ./src/tests/ncsh_parser_fuzzing.c ./src/ncsh_parser.c ./src/eskilib/eskilib_string.c
	./a.out NCSH_PARSER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: test_z
test_z :
	chmod +x ./tests_z.sh
	./tests_z.sh

# .PHONY: test_arena
# test_arena :
# 	$(CC) $(std) $(debug_flags) ./src/eskilib/eskilib_test.c ./src/ncsh_arena.c ./src/tests/ncsh_arena_tests.c -o ./bin/ncsh_arena_tests
# 	./bin/ncsh_arena_tests

.PHONY: clean
clean :
	rm $(target) $(objects)
