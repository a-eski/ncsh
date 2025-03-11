STD = -std=c2x
CC ?= gcc
DESTDIR ?= /bin
BUILDDIR ?= bin
RELEASE ?= 1
debug_flags = -Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-all -fsanitize=address,undefined,leak -g
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -O3 -DNDEBUG
fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,leak,fuzzer -DNDEBUG -g
objects = obj/main.o obj/ncsh.o obj/ncsh_noninteractive.o obj/ncsh_readline.o obj/ncsh_vm.o obj/ncsh_vm_tokenizer.o obj/ncsh_terminal.o obj/eskilib_string.o obj/eskilib_file.o obj/eskilib_hashtable.o obj/ncsh_parser.o obj/ncsh_vm_builtins.o obj/ncsh_history.o obj/ncsh_autocompletions.o obj/ncsh_config.o obj/fzf.o obj/z.o
target = $(BUILDDIR)/ncsh

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

obj/%.o: src/readline/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/vm/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/eskilib/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/z/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/%.c
	$(cc_with_flags) -c $< -o $@

.PHONY: release
release :
	make RELEASE=1
.PHONY: r
r :
	make release

.PHONY: debug
debug :
	make -B RELEASE=0
.PHONY: d
d:
	make debug

.PHONY: unity
unity :
	$(CC) $(STD) $(release_flags) src/unity.c -o $(target)
.PHONY: u
u :
	make unity

.PHONY: unity_debug
unity_debug :
	$(CC) $(STD) $(debug_flags) src/unity.c -o $(target)
.PHONY: ud
ud :
	make unity_debug

.PHONY: install
install : $(target)
	install -C $(target) $(DESTDIR)

.PHONY: check
check :
	set -e
	make test_fzf
	make test_autocompletions
	make test_history
	make test_parser
	make test_config
	make test_readline
.PHONY: c
c :
	make check

.PHONY: acceptance_tests
acceptance_tests :
	chmod +x ./tests_at.sh
	./tests_at.sh
.PHONY: at
at :
	make acceptance_tests

# make check_local aka make l

.PHONY: check_local
check_local :
	set -e
	make check
	make test_z
	make acceptance_tests
.PHONY: l
l :
	make check_local

.PHONY: test_history
test_history :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/eskilib/eskilib_hashtable.c ./src/readline/ncsh_history.c ./src/tests/ncsh_history_tests.c -o ./$(BUILDDIR)/ncsh_history_tests
	./$(BUILDDIR)/ncsh_history_tests
.PHONY: th
th :
	make test_history

.PHONY: fuzz_history
fuzz_history :
	clang $(STD) $(fuzz_flags) -DNCSH_HISTORY_TEST ./src/tests/ncsh_history_fuzzing.c ./src/readline/ncsh_history.c ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_file.c
	./a.out NCSH_HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fh
fh :
	make fuzz_history

.PHONY: test_autocompletions
test_autocompletions :
	 $(CC) $(STD) $(debug_flags) ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/readline/ncsh_autocompletions.c ./src/tests/ncsh_autocompletions_tests.c -o ./$(BUILDDIR)/ncsh_autocompletions_tests
	 ./$(BUILDDIR)/ncsh_autocompletions_tests
.PHONY: ta
ta :
	make test_autocompletions

.PHONY: fuzz_autocompletions
fuzz_autocompletions :
	clang $(STD) $(fuzz_flags) ./src/tests/ncsh_autocompletions_fuzzing.c ./src/readline/ncsh_autocompletions.c ./src/eskilib/eskilib_string.c
	./a.out NCSH_AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fa
fa :
	make fuzz_autocompletions

.PHONY: test_parser
test_parser :
	$(CC) $(STD) $(debug_flags) ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_parser.c ./src/tests/ncsh_parser_tests.c -o ./$(BUILDDIR)/ncsh_parser_tests
	./$(BUILDDIR)/ncsh_parser_tests
.PHONY: tp
tp :
	make test_parser

.PHONY: fuzz_parser
fuzz_parser :
	clang $(STD) $(fuzz_flags) ./src/tests/ncsh_parser_fuzzing.c ./src/ncsh_parser.c ./src/eskilib/eskilib_string.c
	./a.out NCSH_PARSER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fp
fp :
	make fuzz_parser

.PHONY: test_z
test_z :
	chmod +x ./tests_z.sh
	./tests_z.sh
.PHONY: tz
tz :
	make test_z

.PHONY: test_fzf
test_fzf :
	$(CC) $(STD) -fsanitize=address,undefined,leak -g ./src/z/fzf.c ./src/z/tests/lib/examiner.c ./src/z/tests/fzf_tests.c -o ./$(BUILDDIR)/fzf_tests
	@LD_LIBRARY_PATH=/usr/local/lib:./bin:${LD_LIBRARY_PATH} ./$(BUILDDIR)/fzf_tests
.PHONY: tf
tf :
	make test_fzf

.PHONY: test_config
test_config :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/ncsh_config.c ./src/tests/ncsh_config_tests.c -o ./$(BUILDDIR)/ncsh_config_tests
	./$(BUILDDIR)/ncsh_config_tests
.PHONY: tc
tc :
	make test_config

.PHONY: test_readline
test_readline :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/eskilib/eskilib_hashtable.c ./src/readline/ncsh_terminal.c ./src/readline/ncsh_autocompletions.c ./src/readline/ncsh_history.c ./src/readline/ncsh_readline.c ./src/tests/ncsh_readline_tests.c -o ./$(BUILDDIR)/ncsh_readline_tests
	./$(BUILDDIR)/ncsh_readline_tests
.PHONY: tr
tr :
	make test_readline

.PHONY: clang_format
clang_format :
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;

.PHONY: clean
clean :
	rm $(target) $(objects)
