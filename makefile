STD = -std=c2x
CC ?= gcc
# $(info *** Using CC = $(CC))
DESTDIR ?= /bin
RELEASE ?= 1

main_flags = -Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts
# -pg

debug_flags = $(main_flags) -D_FORTIFY_SOURCE=3 -fsanitize=address,undefined,leak -g
# -fprofile-arcs -ftest-coverage

test_flags =  $(debug_flags)

release_flags = $(main_flags) -flto -O3 -ffast-math -march=native -DNDEBUG
# release_flags = $(main_flags) -flto -O3 -ffast-math -march=native -DNDEBUG

fuzz_flags = $(debug_flags) -fsanitize=fuzzer -DNDEBUG -O3

objects = obj/main.o obj/bestline.o obj/arena.o obj/pipe.o obj/redirection.o obj/vm_math.o obj/vm.o obj/interpreter.o obj/parse.o obj/prompt.o obj/efile.o obj/hashset.o obj/lex.o obj/expand.o obj/vars.o obj/builtins.o obj/ac.o obj/env.o obj/alias.o obj/conf.o obj/fzf.o obj/z.o obj/ttyio.o obj/tcaps.o obj/terminfo.o obj/unibilium.o obj/uninames.o obj/uniutil.o

target = ./bin/ncsh

ifeq ($(RELEASE), 1)
	CFLAGS ?= $(release_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS) $(DEFINES)
else
	CFLAGS ?= $(debug_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS) $(DEFINES)
endif

ifneq ($(OS),Windows_NT)
  	TERMINFO="$(shell ncursesw6-config --terminfo 2>/dev/null || \
                         ncurses6-config  --terminfo 2>/dev/null || \
                         ncursesw5-config --terminfo 2>/dev/null || \
                         ncurses5-config  --terminfo 2>/dev/null || \
                         echo "/usr/share/terminfo")"
  	TERMINFO_DIRS="$(shell ncursesw6-config --terminfo-dirs 2>/dev/null || \
                         ncurses6-config  --terminfo-dirs 2>/dev/null || \
                         ncursesw5-config --terminfo-dirs 2>/dev/null || \
                         ncurses5-config  --terminfo-dirs 2>/dev/null || \
                         echo "/etc/terminfo:/lib/terminfo:/usr/share/terminfo:/usr/lib/terminfo:/usr/local/share/terminfo:/usr/local/lib/terminfo")"
else
  	TERMINFO_DIRS=""
  	TERMINFO=""
endif
TTYIO_DEFINES ?= -DTERMINFO='$(TERMINFO)' -DTERMINFO_DIRS='$(TERMINFO_DIRS)'
TTYIO_FILES = ./src/ttyio/lib/unibilium.c ./src/ttyio/lib/uninames.c ./src/ttyio/lib/uniutil.c ./src/ttyio/tcaps.c ./src/ttyio/terminfo.c ./src/ttyio/ttyio.c
TTYIO_IN = $(TTYIO_DEFINES) $(TTYIO_FILES)

$(target): $(objects)
	$(cc_with_flags) -o $(target) $(objects)

obj/%.o: src/ttyio/lib/%.c
	$(cc_with_flags) $(TTYIO_DEFINES) -c $< -o $@

obj/%.o: src/ttyio/%.c
	$(cc_with_flags) -DTTY_USE_NEWLINE_FB -c $< -o $@

obj/%.o: src/io/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/interpreter/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/eskilib/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/z/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/%.c
	$(cc_with_flags) -c $< -o $@

# Normal release build
release:
	make RELEASE=1
r:
	make release

# Normal debug build
debug:
	make RELEASE=0
d:
	make debug

# Unity/jumbo release build
unity:
	echo "warn: unity release builds not currently working for GCC."
	$(CC) $(STD) $(release_flags) src/unity.c -o $(target)

u:
	make unity

# Unity/jumbo debug build
unity_debug:
	$(CC) $(STD) $(debug_flags) src/unity.c -o $(target)
ud:
	make unity_debug

# TODO: finish impl
# Unity/jumbo debug build, with history, z database, rc file in place
in_place_debug:
	$(CC) $(STD) $(debug_flags) -DNCSH_IN_PLACE src/unity.c -o $(target)

# TODO: finish impl
# Unity/jumbo release build, with history, z database, rc file in place
in_place_release:
	$(CC) $(STD) $(release_flags) -DNCSH_IN_PLACE src/unity.c -o $(target)

# Install locally to DESTDIR (default /usr/bin/)
.PHONY: install
install: $(target)
	strip --strip-all $(target)
	install -C $(target) $(DESTDIR)

# Run the tests that get ran in CI
.PHONY: check
check:
	set -e
	make test_fzf
	make test_str
	make test_arena
	make test_alias
	make test_ac
	make test_hashset
	make test_lex
	make test_parse
	make test_vm_next
	make test_vm_math
.PHONY: c
c:
	make check

# Run User Acceptance Tests
.PHONY: acceptance_tests
acceptance_tests:
	chmod +x ./acceptance_tests.sh
	./acceptance_tests.sh
.PHONY: at
at:
	make acceptance_tests

# Run all tests including user acceptance tests
.PHONY: check_local
check_local:
	set -e
	make check
	make test_expand
	make test_vm
	make test_env
	make test_conf
	make test_z
	make at
.PHONY: l
l:
	make check_local

# Run autocompletion tests
test_ac:
	 $(CC) $(STD) $(test_flags) ./src/arena.c ./src/io/ac.c ./tests/io/ac_tests.c -o ./bin/ac_tests
	 ./bin/ac_tests
tac:
	make test_ac

# Run autocompletion fuzzer
fuzz_ac:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/fuzz/ac_fuzzing.c ./src/arena.c ./src/io/ac.c -o ./bin/ac_fuzz
	./bin/ac_fuzz AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
fac:
	make fuzz_ac

# Run autocompletions benchmarks
bench_ac:
	$(CC) $(STD) $(test_flags) -DNDEBUG ./src/arena.c ./src/io/ac.c ./tests/bench/ac_bench.c -o ./bin/ac_bench
	hyperfine --warmup 1000 --shell=none './bin/ac_bench'
bac:
	make bench_ac

bench_acr:
	$(CC) $(STD) $(release_flags) -DNDEBUG ./src/arena.c ./src/io/ac.c ./tests/bench/ac_bench.c -o ./bin/ac_bench
	hyperfine --warmup 1000 --shell=none './bin/ac_bench'
bacr:
	make bench_acr

bench_ac_tests:
	$(CC) $(STD) $(test_flags) -DNDEBUG ./src/arena.c ./src/io/ac.c ./tests/io/ac_tests.c -o ./bin/ac_tests
	hyperfine --warmup 1000 --shell=none './bin/ac_tests'
bact:
	make bench_ac_tests

# Run lexer tests
test_lex:
	$(CC) $(STD) $(test_flags) ./src/arena.c ./src/interpreter/lex.c ./tests/interpreter/lex_tests.c -o ./bin/lex_tests
	./bin/lex_tests
tlx:
	make test_lex

# Run lexer fuzzer
fuzz_lex:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/fuzz/lex_fuzzing.c ./src/arena.c ./src/interpreter/lex.c -o ./bin/lex_fuzz
	./bin/lex_fuzz LEXER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
fp:
	make fuzz_lex

# Run parser tests
.PHONY: test_parse
test_parse:
	$(CC) $(STD) $(test_flags) $(TTYIO_IN) ./src/arena.c ./src/alias.c ./src/env.c ./src/interpreter/lex.c ./src/interpreter/parse.c ./tests/interpreter/parse_tests.c -o ./bin/parse_tests
	./bin/parse_tests
.PHONY: tp
tp:
	make test_parse

bench_parse:
	$(CC) $(STD) $(test_flags) $(TTYIO_IN) ./src/arena.c ./src/alias.c ./src/env.c ./src/interpreter/lex.c ./src/interpreter/parse.c ./tests/interpreter/parse_tests.c -o ./bin/parse_tests
	hyperfine --warmup 1000 --shell=none './bin/parse_tests'
bp:
	make bench_parse

# Run z tests
test_z:
	$(CC) $(STD) $(test_flags) -DZ_TEST $(TTYIO_IN) ./src/arena.c ./src/z/fzf.c ./src/z/z.c ./tests/z/z_tests.c -o ./bin/z_tests
	./bin/z_tests
tz:
	make test_z

# Run z fuzzer
fuzz_z:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) -DZ_TEST ./src/arena.c ./tests/fuzz/z_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_fuzz
	./bin/z_fuzz Z_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
fz:
	make fuzz_z

# Run z add fuzzer
fuzz_z_add:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) -DZ_TEST ./src/arena.c ./tests/fuzz/z_add_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_add_fuzz
	./bin/z_add_fuzz Z_ADD_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
fza:
	make fuzz_z_add

# Run fzf tests
test_fzf:
	$(CC) $(STD) -fsanitize=address,undefined,leak -g ./src/arena.c ./src/z/fzf.c ./tests/lib/examiner.c ./tests/z/fzf_tests.c -o ./bin/fzf_tests
	@LD_LIBRARY_PATH=/usr/local/lib:./bin/:${LD_LIBRARY_PATH} ./bin/fzf_tests
tf:
	make test_fzf

# Run alias tests
test_alias:
	$(CC) $(STD) $(test_flags) -DNCSH_HISTORY_TEST $(TTYIO_IN) ./src/arena.c ./src/alias.c ./tests/alias_tests.c -o ./bin/alias_tests
	./bin/alias_tests
tal:
	make test_alias

# Run arena tests
test_arena:
	$(CC) $(STD) $(test_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./tests/arena_tests.c -o ./bin/arena_tests
	./bin/arena_tests
ta:
	make test_arena

# Run str tests
test_str:
	$(CC) $(STD) $(test_flags) ./src/arena.c ./tests/eskilib/str_tests.c -o ./bin/str_tests
	./bin/str_tests
ts:
	make test_str

.PHONY: bench_str
bench_str:
	$(CC) $(STD) $(release_flags) ./src/arena.c ./tests/bench/str_bench.c -o ./bin/str_bench
	hyperfine --warmup 10000 --shell=none './bin/str_bench'

# Run VM sanity tests
test_vm:
	$(CC) $(STD) $(test_flags) -DNCSH_VM_TEST $(TTYIO_IN) ./src/arena.c ./src/vars.c ./src/interpreter/lex.c ./src/eskilib/efile.c ./src/io/bestline.c ./src/io/hashset.c ./src/z/fzf.c ./src/z/z.c ./src/env.c ./src/alias.c ./src/conf.c ./src/interpreter/vm_math.c ./src/interpreter/vm.c ./src/interpreter/parse.c ./src/interpreter/builtins.c ./src/interpreter/expand.c ./src/interpreter/pipe.c ./src/interpreter/redirection.c ./tests/interpreter/vm_tests.c -o ./bin/vm_tests
	./bin/vm_tests
tvm:
	make test_vm

test_vm_next:
	$(CC) $(STD) $(test_flags) -DNCSH_VM_TEST $(TTYIO_IN) ./src/arena.c ./src/interpreter/lex.c ./src/eskilib/efile.c ./src/io/bestline.c ./src/io/hashset.c ./src/z/fzf.c ./src/z/z.c ./src/env.c ./src/alias.c ./src/conf.c ./src/interpreter/vm_math.c ./src/interpreter/vm.c ./src/interpreter/parse.c ./src/interpreter/builtins.c ./src/vars.c ./src/interpreter/expand.c ./src/interpreter/pipe.c ./src/interpreter/redirection.c ./tests/interpreter/vm_next_tests.c -o ./bin/vm_next_tests
	./bin/vm_next_tests
tvmn:
	make test_vm_next

test_vm_math:
	$(CC) $(STD) $(test_flags) -DNCSH_VM_TEST $(TTYIO_IN) ./src/arena.c ./src/interpreter/lex.c ./src/eskilib/efile.c ./src/io/bestline.c ./src/io/hashset.c ./src/z/fzf.c ./src/z/z.c ./src/env.c ./src/alias.c ./src/conf.c ./src/vars.c ./src/interpreter/vm_math.c ./src/interpreter/vm.c ./src/interpreter/parse.c ./src/interpreter/builtins.c ./src/interpreter/expand.c ./src/interpreter/pipe.c ./src/interpreter/redirection.c ./tests/interpreter/vm_math_tests.c -o ./bin/vm_math_tests
	./bin/vm_math_tests
tvmm:
	make test_vm_math

# Run hashset tests
test_hashset:
	$(CC) $(STD) $(test_flags) -DNCSH_VM_TEST ./src/arena.c ./src/io/hashset.c ./tests/io/hashset_tests.c -o ./bin/hashset_tests
	./bin/hashset_tests
ths:
	make test_hashset

# Run expand tests
test_expand:
	$(CC) $(STD) $(test_flags) $(TTYIO_IN) ./src/arena.c ./src/alias.c ./src/env.c ./src/vars.c ./src/interpreter/lex.c ./src/interpreter/parse.c ./src/interpreter/expand.c ./tests/interpreter/expand_tests.c -o ./bin/expand_tests
	./bin/expand_tests
te:
	make test_expand

# Run environment tests
test_env:
	$(CC) $(STD) $(test_flags) ./src/arena.c ./src/env.c ./tests/env_tests.c -o ./bin/env_tests
	./bin/env_tests
ten:
	make test_env

# Run conf tests
test_conf:
	$(CC) $(STD) $(test_flags) $(TTYIO_IN) ./src/arena.c ./src/alias.c ./src/env.c ./src/vars.c ./src/conf.c ./src/eskilib/efile.c ./tests/conf_tests.c -o ./bin/conf_tests
	./bin/conf_tests
tc:
	make test_conf

# Test interpreter
fuzz_interpreter:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) -DZ_TEST -DNCSH_VM_TEST $(TTYIO_IN) ./src/arena.c ./src/interpreter/lex.c ./src/eskilib/efile.c ./src/io/hashset.c ./src/z/fzf.c ./src/z/z.c ./src/env.c ./src/alias.c ./src/conf.c ./src/interpreter/vm_math.c ./src/interpreter/vm.c ./src/interpreter/parse.c ./src/interpreter/builtins.c ./src/interpreter/expand.c ./src/interpreter/pipe.c ./src/interpreter/redirection.c ./src/io/bestline.c ./src/interpreter/interpreter.c ./tests/fuzz/interpreter_fuzzing.c -o ./bin/interpreter_fuzz
	./bin/interpreter_fuzz INTERPRETER_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192

# Format the project
clang_format:
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
cf:
	make clang_format

# Perform static analysis on the project
.PHONY: scan_build
scan_build:
	scan-build-19 -analyze-headers make

.PHONY: checksec
checksec:
	checksec --file=./bin/ncsh --output=json

.PHONY: dumpdot
dumpdot:
	$(CC) $(STD) $(test_flags) $(TTYIO_IN) ./src/alias.c ./src/env.c ./src/eskilib/efile.c ./src/io/hashset.c ./src/arena.c ./src/io/ac.c ./src/conf.c ./tests/io/ac_dump_dot.c -o ./bin/ac_dump_dot
	./bin/ac_dump_dot
	dot -Tpng trie -o trie.png
	dot -Tsvg trie -o trie.svg

# Clean-up
.PHONY: clean
clean:
	rm $(target) $(objects)
.PHONY: cl
cl :
	make clean

# obj/*.gcno obj/*.gcda
