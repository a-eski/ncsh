STD = -std=c2x
CC ?= gcc
DESTDIR ?= /bin
RELEASE ?= 1
# debug_flags = -Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -Wwrite-strings -fstack-protector-all -fsanitize=address,undefined,leak -g
debug_flags = -Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-all -fsanitize=address,undefined,leak -g
# -DNCSH_DEBUG
# release_flags = -Wall -Wextra -Werror -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -Wwrite-strings -O3 -DNDEBUG
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -O3 -DNDEBUG
# fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wwrite-strings -fsanitize=address,leak,fuzzer -DNDEBUG -g
fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,leak,fuzzer -DNDEBUG -g
objects = obj/main.o obj/arena.o obj/noninteractive.o obj/ncreadline.o obj/vm.o obj/vm_tokenizer.o obj/terminal.o obj/efile.o obj/emap.o obj/var.o obj/parser.o obj/vm_builtins.o obj/history.o obj/autocompletions.o obj/config.o obj/fzf.o obj/z.o
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
	strip $(target)
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
	make test_emap
	make test_estr
	make test_var
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
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/etest.c ./src/eskilib/efile.c ./src/eskilib/emap.c ./src/arena.c ./src/readline/history.c ./tests/history_tests.c -o ./bin/history_tests
	./bin/history_tests
.PHONY: th
th :
	make test_history

.PHONY: fuzz_history
fuzz_history :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) -DNCSH_HISTORY_TEST ./tests/history_fuzzing.c ./src/arena.c ./src/readline/history.c ./src/eskilib/efile.c ./src/eskilib/emap.c -o ./bin/history_fuzz
	./bin/history_fuzz HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fh
fh :
	make fuzz_history

.PHONY: test_autocompletions
test_autocompletions :
	 $(CC) $(STD) $(debug_flags) ./src/eskilib/etest.c ./src/arena.c ./src/readline/autocompletions.c ./tests/autocompletions_tests.c -o ./bin/autocompletions_tests
	 ./bin/autocompletions_tests
.PHONY: tac
tac :
	make test_autocompletions

.PHONY: fuzz_autocompletions
fuzz_autocompletions :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/autocompletions_fuzzing.c ./src/arena.c ./src/readline/autocompletions.c -o ./bin/autocompletions_fuzz
	./bin/autocompletions_fuzz AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fa
fa :
	make fuzz_autocompletions

.PHONY: bench_autocompletions
bench_autocompletions :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/readline/autocompletions.c ./tests/autocompletions_bench.c -o ./bin/autocompletions_bench
	hyperfine --warmup 1000 --shell=none './bin/autocompletions_bench'
.PHONYE: ba
ba :
	make bench_autocompletions

.PHONY: bench_autocompletions_tests
bench_autocompletions_tests :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/readline/autocompletions.c ./tests/autocompletions_tests.c -o ./bin/autocompletions_tests
	hyperfine --warmup 1000 --shell=none './bin/autocompletions_tests'
.PHONYE: bat
bat :
	make bench_autocompletions_tests

.PHONY: test_parser
test_parser :
	$(CC) $(STD) $(debug_flags) ./src/eskilib/etest.c ./src/arena.c ./src/var.c ./src/parser.c ./tests/parser_tests.c -o ./bin/parser_tests
	./bin/parser_tests
.PHONY: tp
tp :
	make test_parser

.PHONY: fuzz_parser
fuzz_parser :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/parser_fuzzing.c ./src/arena.c ./src/parser.c -o ./bin/parser_fuzz
	./bin/parser_fuzz PARSER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fp
fp :
	make fuzz_parser

.PHONY: bench_parser
bench_parser :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/var.c ./src/parser.c ./tests/parser_bench.c -o ./bin/parser_bench
	hyperfine --warmup 1000 --shell=none './bin/parser_bench'
.PHONY: bp
bp :
	make bench_parser

.PHONY: bench_parser_tests
bench_parser_tests :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/var.c ./src/parser.c ./tests/parser_tests.c -o ./bin/parser_tests
	hyperfine --warmup 1000 --shell=none './bin/parser_tests'
.PHONY: bpt
bpt :
	make bench_parser_tests

.PHONY: bench_parser_and_vm
bench_parser_and_vm :
	hyperfine --warmup 100 --shell /bin/ncsh 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
	rm t.txt t2.txt
.PHONY: bpv
bpv :
	make bench_parser_and_vm

.PHONY: bash_bench_parser_and_vm
bash_bench_parser_and_vm :
	hyperfine --warmup 100 --shell /bin/bash 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
	rm t.txt t2.txt
.PHONY: bbpv
bbpv :
	make bash_bench_parser_and_vm

.PHONY: test_z
test_z :
	gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DZ_TEST ./src/arena.c ./src/eskilib/etest.c ./src/z/fzf.c ./src/z/z.c ./tests/z_tests.c -o ./bin/z_tests
	./bin/z_tests
.PHONY: tz
tz :
	make test_z

.PHONY: fuzz_z
fuzz_z:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/z_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_fuzz
	./bin/z_fuzz Z_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fz
fz:
	make fuzz_z

.PHONY: fuzz_z_add
fuzz_z_add:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/z_add_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_add_fuzz
	./bin/z_add_fuzz Z_ADD_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fza
fza:
	make fuzz_z_add

.PHONY: test_fzf
test_fzf :
	$(CC) $(STD) -fsanitize=address,undefined,leak -g ./src/arena.c ./src/z/fzf.c ./tests/lib/examiner.c ./tests/fzf_tests.c -o ./bin/fzf_tests
	@LD_LIBRARY_PATH=/usr/local/lib:./bin/:${LD_LIBRARY_PATH} ./bin/fzf_tests
.PHONY: tf
tf :
	make test_fzf

.PHONY: test_config
test_config :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/eskilib/etest.c ./src/eskilib/efile.c ./src/config.c ./tests/config_tests.c -o ./bin/config_tests
	./bin/config_tests
.PHONY: tc
tc :
	make test_config

.PHONY: test_readline
test_readline :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/eskilib/etest.c ./src/eskilib/efile.c ./src/eskilib/emap.c ./src/readline/terminal.c ./src/readline/autocompletions.c ./src/readline/history.c ./src/readline/ncreadline.c ./tests/ncreadline_tests.c -o ./bin/ncreadline_tests
	./bin/ncreadline_tests
.PHONY: tr
tr :
	make test_readline

.PHONY: test_arena
test_arena :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/eskilib/etest.c ./tests/arena_tests.c -o ./bin/arena_tests
	./bin/arena_tests
.PHONY: ta
ta :
	make test_arena

.PHONY: test_emap
test_emap :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/eskilib/emap.c ./src/eskilib/etest.c ./tests/emap_tests.c -o ./bin/emap_tests
	./bin/emap_tests

.PHONY: test_estr
test_estr :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/eskilib/etest.c ./tests/estr_tests.c -o ./bin/estr_tests
	./bin/estr_tests

.PHONY: test_var
test_var :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/var.c ./src/eskilib/etest.c ./tests/var_tests.c -o ./bin/var_tests
	./bin/var_tests

.PHONY: clang_format
clang_format :
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;

.PHONY: clean
clean :
	rm $(target) $(objects)
