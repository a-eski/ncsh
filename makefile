STD = -std=c2x
CC ?= gcc
DESTDIR ?= /bin
RELEASE ?= 1
debug_flags = -Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-all -fsanitize=address,undefined,leak -g
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -O3 -DNDEBUG
fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,leak,fuzzer -DNDEBUG -g
objects = obj/main.o obj/ncsh.o obj/ncsh_arena.o obj/ncsh_noninteractive.o obj/ncsh_readline.o obj/ncsh_vm.o obj/ncsh_vm_tokenizer.o obj/ncsh_terminal.o obj/eskilib_file.o obj/eskilib_hashtable.o obj/ncsh_parser.o obj/ncsh_vm_builtins.o obj/ncsh_history.o obj/ncsh_autocompletions.o obj/ncsh_config.o obj/fzf.o obj/z.o
target = ./bin/ncsh

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
	make test_arena
	make test_hashtable
	make test_string
.PHONY: c
c :
	make check

.PHONY: acceptance_tests
acceptance_tests :
	chmod +x ./acceptance_tests.sh
	./acceptance_tests.sh
.PHONY: at
at :
	make acceptance_tests

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
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/eskilib/eskilib_hashtable.c ./src/ncsh_arena.c ./src/readline/ncsh_history.c ./tests/ncsh_history_tests.c -o ./bin/ncsh_history_tests
	./bin/ncsh_history_tests
.PHONY: th
th :
	make test_history

.PHONY: fuzz_history
fuzz_history :
	clang-19 $(STD) $(fuzz_flags) -DNCSH_HISTORY_TEST ./tests/ncsh_history_fuzzing.c ./src/ncsh_arena.c ./src/readline/ncsh_history.c ./src/eskilib/eskilib_file.c ./src/eskilib/eskilib_hashtable.c -o ./bin/history_fuzz
	./bin/history_fuzz NCSH_HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fh
fh :
	make fuzz_history

.PHONY: test_autocompletions
test_autocompletions :
	 $(CC) $(STD) $(debug_flags) ./src/eskilib/eskilib_test.c ./src/ncsh_arena.c ./src/readline/ncsh_autocompletions.c ./tests/ncsh_autocompletions_tests.c -o ./bin/ncsh_autocompletions_tests
	 ./bin/ncsh_autocompletions_tests
.PHONY: tac
tac :
	make test_autocompletions

.PHONY: fuzz_autocompletions
fuzz_autocompletions :
	clang-19 $(STD) $(fuzz_flags) ./tests/ncsh_autocompletions_fuzzing.c ./src/ncsh_arena.c ./src/readline/ncsh_autocompletions.c -o ./bin/autocompletions_fuzz
	./bin/autocompletions_fuzz NCSH_AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fa
fa :
	make fuzz_autocompletions

.PHONY: bench_autocompletions
bench_autocompletions :
	hyperfine --warmup 500 --shell=none './bin/ncsh_autocompletions_tests'
.PHONYE: ba
ba :
	make bench_autocompletions

.PHONY: test_parser
test_parser :
	$(CC) $(STD) $(debug_flags) ./src/eskilib/eskilib_test.c ./src/ncsh_arena.c ./src/ncsh_parser.c ./tests/ncsh_parser_tests.c -o ./bin/ncsh_parser_tests
	./bin/ncsh_parser_tests
.PHONY: tp
tp :
	make test_parser

.PHONY: fuzz_parser
fuzz_parser :
	clang-19 $(STD) $(fuzz_flags) ./tests/ncsh_parser_fuzzing.c ./src/ncsh_arena.c ./src/ncsh_parser.c -o ./bin/parser_fuzz
	./bin/parser_fuzz NCSH_PARSER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fp
fp :
	make fuzz_parser

.PHONY: bench_parser_and_vm
bench_parser_and_vm :
	hyperfine --warmup --shell /bin/ncsh 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
	rm t.txt t2.txt
.PHONY: bpv
bpv :
	make bench_parser_and_vm

.PHONY: bash_bench_parser_and_vm
bash_bench_parser_and_vm :
	hyperfine --warmup --shell /bin/bash 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
	rm t.txt t2.txt
.PHONY: bbpv
bbpv :
	make bash_bench_parser_and_vm

.PHONY: test_z
test_z :
	gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DZ_TEST ./src/ncsh_arena.c ./src/eskilib/eskilib_test.c ./src/z/fzf.c ./src/z/z.c ./tests/z_tests.c -o ./bin/z_tests
	./bin/z_tests
.PHONY: tz
tz :
	make test_z

.PHONY: fuzz_z
fuzz_z:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/ncsh_arena.c ./tests/z_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_fuzz
	./bin/z_fuzz Z_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fz
fz:
	make fuzz_z

.PHONY: fuzz_z_add
fuzz_z_add:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/ncsh_arena.c ./tests/z_add_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_add_fuzz
	./bin/z_add_fuzz Z_ADD_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fza
fza:
	make fuzz_z_add

.PHONY: test_fzf
test_fzf :
	$(CC) $(STD) -fsanitize=address,undefined,leak -g ./src/ncsh_arena.c ./src/z/fzf.c ./tests/lib/examiner.c ./tests/fzf_tests.c -o ./bin/fzf_tests
	@LD_LIBRARY_PATH=/usr/local/lib:./bin/:${LD_LIBRARY_PATH} ./bin/fzf_tests
.PHONY: tf
tf :
	make test_fzf

.PHONY: test_config
test_config :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/ncsh_arena.c ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/ncsh_config.c ./tests/ncsh_config_tests.c -o ./bin/ncsh_config_tests
	./bin/ncsh_config_tests
.PHONY: tc
tc :
	make test_config

.PHONY: test_readline
test_readline :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/ncsh_arena.c ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/eskilib/eskilib_hashtable.c ./src/readline/ncsh_terminal.c ./src/readline/ncsh_autocompletions.c ./src/readline/ncsh_history.c ./src/readline/ncsh_readline.c ./tests/ncsh_readline_tests.c -o ./bin/ncsh_readline_tests
	./bin/ncsh_readline_tests
.PHONY: tr
tr :
	make test_readline

.PHONY: test_arena
test_arena :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/ncsh_arena.c ./src/eskilib/eskilib_test.c ./tests/ncsh_arena_tests.c -o ./bin/ncsh_arena_tests
	./bin/ncsh_arena_tests
.PHONY: ta
ta :
	make test_arena

.PHONY: test_hashtable
test_hashtable :
	$(CC) $(STD) $(debug_flags) ./src/ncsh_arena.c ./src/eskilib/eskilib_hashtable.c ./src/eskilib/eskilib_test.c ./tests/eskilib_hashtable_tests.c -o ./bin/eskilib_hashtable_tests
	./bin/eskilib_hashtable_tests
.PHONY: tht
tht :
	make test_hashtable

.PHONY: test_string
test_string :
	$(CC) $(STD) $(debug_flags) ./src/ncsh_arena.c ./src/eskilib/eskilib_test.c ./tests/eskilib_string_tests.c -o ./bin/eskilib_string_tests
	./bin/eskilib_string_tests
.PHONY: ts
ts :
	make test_string

.PHONY: clang_format
clang_format :
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;

.PHONY: clean
clean :
	rm $(target) $(objects)
