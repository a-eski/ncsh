STD = -std=c2x
CC ?= gcc
DESTDIR ?= /usr/local
RELEASE ?= 1
debug_flags = -Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wconversion -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-all -fsanitize=address,undefined,leak -g
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wconversion -Wsign-conversion -Wformat=2 -O3 -DNDEBUG
fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,leak,fuzzer -DNDEBUG -g
objects = obj/main.o obj/ncsh.o obj/ncsh_noninteractive.o obj/ncsh_vm.o obj/ncsh_terminal.o obj/eskilib_string.o obj/eskilib_file.o obj/eskilib_hashtable.o obj/ncsh_parser.o obj/ncsh_builtins.o obj/ncsh_history.o obj/ncsh_autocompletions.o obj/ncsh_config.o obj/z.o obj/ncsh_interpreter.o
# obj/ncsh_arena.o
target = bin/ncsh

ifeq ($(CC), gcc)
	release_flags += -s
endif

ifeq ($(RELEASE), 1)
	CFLAGS ?= $(release_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS)
else
	CFLAGS ?= $(debug_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS)
endif

$(target) : $(objects)
	$(cc_with_flags) -o $(target) $(objects)

obj/main.o :
	$(cc_with_flags) -c src/main.c -o obj/main.o
obj/ncsh.o :
	$(cc_with_flags) -c src/ncsh.c -o obj/ncsh.o
obj/ncsh_noninteractive.o :
	$(cc_with_flags) -c src/ncsh_noninteractive.c -o obj/ncsh_noninteractive.o
obj/ncsh_interpreter.o :
	$(cc_with_flags) -c src/ncsh_interpreter.c -o obj/ncsh_interpreter.o
obj/ncsh_vm.o :
	$(cc_with_flags) -c src/ncsh_vm.c -o obj/ncsh_vm.o
obj/ncsh_builtins.o :
	$(cc_with_flags) -c src/ncsh_builtins.c -o obj/ncsh_builtins.o
obj/ncsh_history.o :
	$(cc_with_flags) -c src/ncsh_history.c -o obj/ncsh_history.o
obj/ncsh_autocompletions.o :
	$(cc_with_flags) -c src/ncsh_autocompletions.c -o obj/ncsh_autocompletions.o
obj/ncsh_terminal.o :
	$(cc_with_flags) -c src/ncsh_terminal.c -o obj/ncsh_terminal.o
obj/ncsh_parser.o :
	$(cc_with_flags) -c src/ncsh_parser.c -o obj/ncsh_parser.o
obj/ncsh_config.o :
	$(cc_with_flags) -c src/ncsh_config.c -o obj/ncsh_config.o
obj/z.o :
	$(cc_with_flags) -c src/z/z.c -o obj/z.o
obj/eskilib_hashtable.o :
	$(cc_with_flags) -c src/eskilib/eskilib_hashtable.c -o obj/eskilib_hashtable.o
obj/eskilib_string.o :
	$(cc_with_flags) -c src/eskilib/eskilib_string.c -o obj/eskilib_string.o
obj/eskilib_file.o :
	$(cc_with_flags) -c src/eskilib/eskilib_file.c -o obj/eskilib_file.o
# obj/ncsh_arena.o :
	# $(cc_with_flags) -c src/ncsh_arena.c -o obj/ncsh_arena.o

.PHONY: debug
debug :
	make -B RELEASE=0

.PHONY: unity
unity :
	$(CC) $(STD) $(release_flags) src/unity.c -o $(target)

.PHONY: unity_debug
unity_debug :
	$(CC) $(STD) $(debug_flags) src/unity.c -o $(target)

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

.PHONY: integration_tests
integration_tests :
	chmod +x ./tests_it.sh
	./tests_it.sh

.PHONY: l
l :
	set -e
	make check
	make integration_tests

.PHONY: test_history
test_history :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/eskilib/eskilib_hashtable.c ./src/ncsh_history.c ./src/tests/ncsh_history_tests.c -o ./bin/ncsh_history_tests
	./bin/ncsh_history_tests

.PHONY: fuzz_history
fuzz_history :
	clang $(STD) $(fuzz_flags) -DNCSH_HISTORY_TEST ./src/tests/ncsh_history_fuzzing.c ./src/ncsh_history.c ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_file.c
	./a.out NCSH_HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: test_autocompletions
test_autocompletions :
	 $(CC) $(STD) $(debug_flags) ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_autocompletions.c ./src/tests/ncsh_autocompletions_tests.c -o ./bin/ncsh_autocompletions_tests
	 ./bin/ncsh_autocompletions_tests

.PHONY: fuzz_autocompletions
fuzz_autocompletions :
	clang $(STD) $(fuzz_flags) ./src/tests/ncsh_autocompletions_fuzzing.c ./src/ncsh_autocompletions.c ./src/eskilib/eskilib_string.c
	./a.out NCSH_AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192

.PHONY test_parser :
	$(CC) $(STD) $(debug_flags) ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_parser.c ./src/tests/ncsh_parser_tests.c -o ./bin/ncsh_parser_tests
	./bin/ncsh_parser_tests

.PHONY: fuzz_parser
fuzz_parser :
	clang $(STD) $(fuzz_flags) ./src/tests/ncsh_parser_fuzzing.c ./src/ncsh_parser.c ./src/eskilib/eskilib_string.c
	./a.out NCSH_PARSER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: test_z
test_z :
	chmod +x ./tests_z.sh
	./tests_z.sh

# .PHONY: test_arena
# test_arena :
# 	$(CC) $(STD) $(debug_flags) ./src/eskilib/eskilib_test.c ./src/ncsh_arena.c ./src/tests/ncsh_arena_tests.c -o ./bin/ncsh_arena_tests
# 	./bin/ncsh_arena_tests

.PHONY: clean
clean :
	rm $(target) $(objects)
