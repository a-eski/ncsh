std = -std=c2x
debug_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -g
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -O3 -DNDEBUG
objects = obj/main.o obj/ncsh.o obj/ncsh_vm.o obj/ncsh_terminal.o obj/eskilib_string.o obj/eskilib_file.o obj/ncsh_debug.o obj/ncsh_args.o obj/ncsh_parser.o obj/ncsh_builtins.o obj/ncsh_history.o obj/ncsh_autocompletions.o
install_destination = /usr/local
target = bin/ncsh
history_file = .ncsh_history

CC ?= gcc

ifeq ($(CC), gcc)
	release_flags += -s
endif

RELEASE ?= 1

ifeq ($(RELEASE), 1)
	run_command = make
	run_new_command = make -B
	CFLAGS ?= $(release_flags)
	cc_with_flags = $(CC) $(std) $(CFLAGS)
else
	run_command = make RELEASE=0
	run_new_command = make -B RELEASE=0
	CFLAGS ?= $(debug_flags)
	cc_with_flags = $(CC) $(std) $(CFLAGS)
endif

$(target) : $(objects)
	$(cc_with_flags) -o $(target) $(objects)

obj/main.o : src/main.c src/ncsh.h
	$(cc_with_flags) -c src/main.c -o obj/main.o
obj/ncsh.o : src/ncsh.c src/ncsh.h src/ncsh_vm.h src/ncsh_terminal.h src/eskilib/eskilib_string.h src/eskilib/eskilib_colors.h src/ncsh_parser.h src/ncsh_args.h src/ncsh_autocompletions.h
	$(cc_with_flags) -c src/ncsh.c -o obj/ncsh.o
obj/ncsh_vm.o : src/ncsh_vm.h src/eskilib/eskilib_string.h src/eskilib/eskilib_colors.h src/ncsh_terminal.h src/ncsh_args.h src/ncsh_builtins.h
	$(cc_with_flags) -c src/ncsh_vm.c -o obj/ncsh_vm.o
obj/ncsh_builtins.o : src/ncsh_builtins.h src/ncsh_args.h src/eskilib/eskilib_string.h
	$(cc_with_flags) -c src/ncsh_builtins.c -o obj/ncsh_builtins.o
obj/ncsh_history.o : src/ncsh_history.h src/eskilib/eskilib_string.h src/eskilib/eskilib_file.h
	$(cc_with_flags) -c src/ncsh_history.c -o obj/ncsh_history.o
obj/ncsh_autocompletions.o : src/ncsh_autocompletions.h src/eskilib/eskilib_string.h
	$(cc_with_flags) -c src/ncsh_autocompletions.c -o obj/ncsh_autocompletions.o
obj/ncsh_terminal.o : src/ncsh_terminal.c src/ncsh_terminal.h
	$(cc_with_flags) -c src/ncsh_terminal.c -o obj/ncsh_terminal.o
obj/ncsh_parser.o : src/ncsh_parser.c src/ncsh_args.h
	$(cc_with_flags) -c src/ncsh_parser.c -o obj/ncsh_parser.o
obj/ncsh_args.o : src/ncsh_args.c src/ncsh_args.h
	$(cc_with_flags) -c src/ncsh_args.c -o obj/ncsh_args.o
obj/eskilib_string.o : src/eskilib/eskilib_string.c src/eskilib/eskilib_string.h
	$(cc_with_flags) -c src/eskilib/eskilib_string.c -o obj/eskilib_string.o
obj/eskilib_file.o : src/eskilib/eskilib_file.c src/eskilib/eskilib_file.h
	$(cc_with_flags) -c src/eskilib/eskilib_file.c -o obj/eskilib_file.o
obj/ncsh_debug.o : src/ncsh_debug.c src/ncsh_debug.h src/ncsh_args.h
	$(cc_with_flags) -c src/ncsh_debug.c -o obj/ncsh_debug.o

.PHONY: run
run :
	$(run_command)
	./$(target)

.PHONY: runnew
runnew :
	$(run_new_command)
	./$(target)

.PHONY: install
install : $(target)
	install -C $(target) $(history_file) $(install_destination)

.PHONY: check
check :
	chmod +x ./tests_harness.sh
	chmod +x ./tests.sh
	./tests_harness.sh

.PHONY: fuzz_history
fuzz_history :
	clang -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG ./src/tests/ncsh_history_fuzzing.c ./src/ncsh_history.c ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_file.c
	./a.out NCSH_HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: fuzz_autocompletions
fuzz_autocompletions :
	clang -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG ./src/tests/ncsh_autocompletions_fuzzing.c ./src/ncsh_autocompletions.c ./src/eskilib/eskilib_string.c
	./a.out NCSH_AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: fuzz_parser
fuzz_parser :
	clang -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG ./src/tests/ncsh_parser_fuzzing.c ./src/ncsh_parser.c ./src/eskilib/eskilib_string.c ./src/ncsh_args.c
	./a.out NCSH_PARSER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: clean
clean :
	rm $(target) $(objects)
