STD = -std=c2x
CC ?= gcc
# $(info *** Using CC = $(CC))
DESTDIR ?= /bin
RELEASE ?= 1

main_flags = -Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-all -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts

debug_flags = $(main_flags) -D_FORTIFY_SOURCE=2 -fsanitize=address,undefined,leak -g
# -fprofile-arcs -ftest-coverage

test_flags =  $(debug_flags)

release_flags = $(main_flags) -flto -O3 -ffast-math -march=native -DNDEBUG

fuzz_flags = $(debug_flags)

objects = obj/main.o obj/arena.o obj/noninteractive.o obj/io.o obj/pipe.o obj/redirection.o obj/vm.o obj/semantic_analyzer.o obj/interpreter.o obj/parser.o obj/prompt.o obj/efile.o obj/hashset.o obj/vars.o obj/lexer.o obj/lexemes.o obj/expansions.o obj/statements.o obj/builtins.o obj/history.o obj/ac.o obj/env.o obj/alias.o obj/config.o obj/fzf.o obj/z.o obj/ttyterm.o obj/tcaps.o obj/unibilium.o obj/uninames.o obj/uniutil.o

target = ./bin/ncsh

ifeq ($(RELEASE), 1)
	CFLAGS ?= $(release_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS)
else
	CFLAGS ?= $(debug_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS)
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
TTYTERM_DEFINES ?= -DTERMINFO='$(TERMINFO)' -DTERMINFO_DIRS='$(TERMINFO_DIRS)'

$(target) : $(objects)
	$(cc_with_flags) -o $(target) $(objects)

obj/%.o: src/ttyterm/lib/%.c
	$(cc_with_flags) $(TTYTERM_DEFINES) -c $< -o $@

obj/%.o: src/ttyterm/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/readline/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/interpreter/%.c
	$(cc_with_flags) -c $< -o $@

obj/%.o: src/interpreter/vm/%.c
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

# Normal debug build
debug :
	make -B RELEASE=0

# Unity/jumbo release build
unity :
	$(CC) $(STD) $(release_flags) src/unity.c -o $(target)

u :
	make unity

# Unity/jumbo debug build
unity_debug :
	$(CC) $(STD) $(debug_flags) src/unity.c -o $(target)
ud:
	make unity_debug

# Unity/jumbo debug build, with history, z database, rc file in place
in_place_debug :
	$(CC) $(STD) $(debug_flags) -DNCSH_IN_PLACE src/unity.c -o $(target)

# Unity/jumbo release build, with history, z database, rc file in place
in_place_release :
	$(CC) $(STD) $(release_flags) -DNCSH_IN_PLACE src/unity.c -o $(target)

# Install locally to DESTDIR (default /usr/bin/)
.PHONY: install
install : $(target)
	strip --strip-all $(target)
	install -C $(target) $(DESTDIR)

# Run the tests that get ran in CI
.PHONY: check
check :
	set -e
	make test_fzf
	make test_str
	make test_arena
	make test_alias
	# make test_prompt
	make test_ac
	make test_hashset
	make test_history
	make test_lexer
	make test_parser
	make test_vars
	make test_vm_next
	make test_expansions
.PHONY: c
c :
	make check

# Run User Acceptance Tests
.PHONY: acceptance_tests
acceptance_tests :
	chmod +x ./acceptance_tests.sh
	./acceptance_tests.sh
.PHONY: at
at :
	make acceptance_tests

# Run all tests including user acceptance tests
.PHONY: check_local
check_local :
	set -e
	make check
	make test_vm
	make test_env
	make test_config
	make test_z
	make at
.PHONY: l
l :
	make check_local

# Run history tests
test_history :
	$(CC) $(STD) $(test_flags) -DNCSH_HISTORY_TEST $(TTYTERM_DEFINES) ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/eskilib/efile.c ./src/readline/hashset.c ./src/arena.c ./src/readline/history.c ./tests/readline/history_tests.c -o ./bin/history_tests
	./bin/history_tests
th :
	make test_history

# Run history fuzzer
fuzz_history :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) -DNCSH_HISTORY_TEST ./tests/fuzz/history_fuzzing.c ./src/arena.c ./src/readline/history.c ./src/eskilib/efile.c ./src/readline/hashset.c -o ./bin/history_fuzz
	./bin/history_fuzz HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
fh :
	make fuzz_history

# Run autocompletion tests
test_ac :
	 $(CC) $(STD) $(test_flags) ./src/arena.c ./src/readline/ac.c ./tests/readline/ac_tests.c -o ./bin/ac_tests
	 ./bin/ac_tests
tac :
	make test_ac

# Run autocompletion fuzzer
fuzz_ac :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/fuzz/ac_fuzzing.c ./src/arena.c ./src/readline/ac.c -o ./bin/ac_fuzz
	./bin/ac_fuzz AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
fac :
	make fuzz_ac

# Run autocompletions benchmarks
bench_ac :
	$(CC) $(STD) $(test_flags) -DNDEBUG ./src/arena.c ./src/readline/ac.c ./tests/bench/ac_bench.c -o ./bin/ac_bench
	hyperfine --warmup 1000 --shell=none './bin/ac_bench'
bac :
	make bench_ac

bench_acr :
	$(CC) $(STD) $(release_flags) -DNDEBUG ./src/arena.c ./src/readline/ac.c ./tests/bench/ac_bench.c -o ./bin/ac_bench
	hyperfine --warmup 1000 --shell=none './bin/ac_bench'
bacr :
	make bench_acr

bench_ac_tests :
	$(CC) $(STD) $(test_flags) -DNDEBUG ./src/arena.c ./src/readline/ac.c ./tests/readline/ac_tests.c -o ./bin/ac_tests
	hyperfine --warmup 1000 --shell=none './bin/ac_tests'
bact :
	make bench_ac_tests

# Run lexer tests
test_lexer :
	$(CC) $(STD) $(test_flags) ./src/arena.c ./src/interpreter/lexemes.c ./src/interpreter/lexer.c ./tests/interpreter/lexer_tests.c -o ./bin/lexer_tests
	./bin/lexer_tests
tlx :
	make test_lexer

# Run lexer fuzzer
fuzz_lexer :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/fuzz/lexer_fuzzing.c ./src/arena.c ./src/interpreter/lexer.c -o ./bin/lexer_fuzz
	./bin/lexer_fuzz LEXER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
fp :
	make fuzz_lexer

# Run parser tests
test_parser :
	$(CC) $(STD) $(test_flags) $(TTYTERM_DEFINES) ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/arena.c ./src/alias.c ./src/env.c ./src/interpreter/expansions.c ./src/interpreter/vars.c ./src/interpreter/lexer.c ./src/interpreter/lexemes.c ./src/interpreter/statements.c ./src/interpreter/parser.c ./tests/interpreter/parser_tests.c -o ./bin/parser_tests
	./bin/parser_tests
tp :
	make test_parser

bench_parser :
	$(CC) $(STD) $(test_flags) $(TTYTERM_DEFINES) ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/arena.c ./src/alias.c ./src/env.c ./src/interpreter/lexer.c ./src/interpreter/vars.c ./src/interpreter/parser.c ./tests/interpreter/parser_tests.c -o ./bin/parser_tests
	hyperfine --warmup 1000 --shell=none './bin/parser_tests'
bp :
	make bench_parser

# Run z tests
test_z :
	$(CC) $(STD) $(test_flags) -DZ_TEST $(TTYTERM_DEFINES) ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/arena.c ./src/z/fzf.c ./src/z/z.c ./tests/z/z_tests.c -o ./bin/z_tests
	./bin/z_tests
tz :
	make test_z

# Run z fuzzer
fuzz_z :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/fuzz/z_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_fuzz
	./bin/z_fuzz Z_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
fz :
	make fuzz_z

# Run z add fuzzer
fuzz_z_add :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/fuzz/z_add_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_add_fuzz
	./bin/z_add_fuzz Z_ADD_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
fza :
	make fuzz_z_add

# Run fzf tests
test_fzf :
	$(CC) $(STD) -fsanitize=address,undefined,leak -g ./src/arena.c ./src/z/fzf.c ./tests/lib/examiner.c ./tests/z/fzf_tests.c -o ./bin/fzf_tests
	@LD_LIBRARY_PATH=/usr/local/lib:./bin/:${LD_LIBRARY_PATH} ./bin/fzf_tests
tf :
	make test_fzf

# Run alias tests
test_alias :
	$(CC) $(STD) $(test_flags) $(TTYTERM_DEFINES) -DNCSH_HISTORY_TEST ./src/arena.c ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/alias.c ./tests/alias_tests.c -o ./bin/alias_tests
	./bin/alias_tests
tal :
	make test_alias

# Run prompt tests
test_prompt :
	$(CC) $(STD) $(test_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/readline/prompt.c ./tests/readline/prompt_tests.c -o ./bin/prompt_tests
	./bin/prompt_tests
tpr :
	make test_prompt

# Run arena tests
test_arena :
	$(CC) $(STD) $(test_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./tests/arena_tests.c -o ./bin/arena_tests
	./bin/arena_tests
ta :
	make test_arena

# Run str tests
test_str :
	$(CC) $(STD) $(test_flags) ./src/arena.c ./tests/eskilib/str_tests.c -o ./bin/str_tests
	./bin/str_tests
ts :
	make test_str

# Run variables (vars) tests
test_vars :
	$(CC) $(STD) $(test_flags) ./src/arena.c ./src/interpreter/vars.c ./tests/interpreter/vars_tests.c -o ./bin/vars_tests
	./bin/vars_tests
tv :
	make test_vars

# Run VM sanity tests
test_vm :
	$(CC) $(STD) $(test_flags) -DNCSH_VM_TEST $(TTYTERM_DEFINES) ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/arena.c ./src/interpreter/lexer.c ./src/eskilib/efile.c ./src/readline/hashset.c ./src/interpreter/vars.c ./src/readline/history.c ./src/z/fzf.c ./src/z/z.c ./src/env.c ./src/alias.c ./src/config.c ./src/interpreter/vm/vm.c ./src/interpreter/semantic_analyzer.c ./src/interpreter/parser.c ./src/interpreter/vm/builtins.c ./src/interpreter/lexemes.c ./src/interpreter/statements.c ./src/interpreter/expansions.c ./src/interpreter/vm/pipe.c ./src/interpreter/vm/redirection.c ./tests/interpreter/vm/vm_tests.c -o ./bin/vm_tests
	./bin/vm_tests
tvm :
	make test_vm

test_vm_next :
	$(CC) $(STD) $(test_flags) -DNCSH_VM_TEST $(TTYTERM_DEFINES) ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/arena.c ./src/interpreter/lexer.c ./src/eskilib/efile.c ./src/readline/hashset.c ./src/interpreter/vars.c ./src/readline/history.c ./src/z/fzf.c ./src/z/z.c ./src/env.c ./src/alias.c ./src/config.c ./src/interpreter/vm/vm.c ./src/interpreter/semantic_analyzer.c ./src/interpreter/parser.c ./src/interpreter/vm/builtins.c ./src/interpreter/lexemes.c ./src/interpreter/statements.c ./src/interpreter/expansions.c ./src/interpreter/vm/pipe.c ./src/interpreter/vm/redirection.c ./tests/interpreter/vm/vm_next_tests.c -o ./bin/vm_next_tests
	./bin/vm_next_tests
tvmn:
	make test_vm_next

# Run hashset tests
test_hashset :
	$(CC) $(STD) $(test_flags) -DNCSH_VM_TEST ./src/arena.c ./src/readline/hashset.c ./tests/readline/hashset_tests.c -o ./bin/hashset_tests
	./bin/hashset_tests
ths :
	make test_hashset

# Run expansions tests
test_expansions :
	$(CC) $(STD) $(test_flags) $(TTYTERM_DEFINES) ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/arena.c ./src/alias.c ./src/env.c ./src/interpreter/lexemes.c ./src/interpreter/statements.c ./src/interpreter/vars.c ./src/interpreter/expansions.c ./tests/interpreter/expansions_tests.c -o ./bin/expansions_tests
	./bin/expansions_tests
te :
	make test_expansions

# Run environment tests
test_env :
	$(CC) $(STD) $(test_flags) ./src/arena.c ./src/env.c ./tests/env_tests.c -o ./bin/env_tests
	./bin/env_tests
ten :
	make test_env

# Run config tests
test_config :
	$(CC) $(STD) $(test_flags) $(TTYTERM_DEFINES) ./src/ttyterm/lib/unibilium.c ./src/ttyterm/lib/uninames.c ./src/ttyterm/lib/uniutil.c ./src/ttyterm/tcaps.c ./src/ttyterm/ttyterm.c ./src/arena.c ./src/alias.c ./src/env.c ./src/config.c ./src/eskilib/efile.c ./tests/config_tests.c -o ./bin/config_tests
	./bin/config_tests
tc :
	make test_config

# Format the project
clang_format :
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
cf :
	make clang_format

# Perform static analysis on the project
.PHONY: scan_build
scan_build:
	scan-build-19 -analyze-headers make

# Clean-up
.PHONY: clean
clean :
	rm $(target) $(objects)

# obj/*.gcno obj/*.gcda
