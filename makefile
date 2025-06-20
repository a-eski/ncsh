STD = -std=c2x
CC ?= gcc
DESTDIR ?= /bin
RELEASE ?= 1
# debug_flags = -Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -Wwrite-strings -fstack-protector-all -fsanitize=address,undefined,leak -g
debug_flags = -Wall -Wextra -Werror -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-all -fsanitize=address,undefined,leak -g
# -DNCSH_DEBUG
# release_flags = -Wall -Wextra -Werror -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -Wwrite-strings -O3 -DNDEBUG
release_flags = -Wall -Wextra -Werror -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -O3 -DNDEBUG
# fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wwrite-strings -fsanitize=address,leak,fuzzer -DNDEBUG -g
fuzz_flags = -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,leak,fuzzer -DNDEBUG -g
objects = obj/main.o obj/arena.o obj/noninteractive.o obj/ncreadline.o obj/pipe.o obj/redirection.o obj/vm_buffer.o obj/vm.o obj/semantic_analyzer.o obj/logic.o obj/interpreter.o obj/parser.o obj/terminal.o obj/prompt.o obj/efile.o obj/hashset.o obj/vars.o obj/tokens.o obj/lexer.o obj/builtins.o obj/history.o obj/ac.o obj/env.o obj/alias.o obj/config.o obj/fzf.o obj/z.o
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
.PHONY: release
release:
	make RELEASE=1

# Normal debug build
.PHONY: debug
debug :
	make -B RELEASE=0

# Unity/jumbo release build
.PHONY: unity
unity :
	$(CC) $(STD) $(release_flags) src/unity.c -o $(target)

.PHONY: u
u :
	make unity

# Unity/jumbo debug build
.PHONY: unity_debug
unity_debug :
	$(CC) $(STD) $(debug_flags) src/unity.c -o $(target)
.PHONY: ud
ud:
	make unity_debug

# Unity/jumbo debug build, with history, z database, rc file in place
.PHONY: in_place_debug
in_place_debug :
	$(CC) $(STD) $(debug_flags) -DNCSH_IN_PLACE src/unity.c -o $(target)

# Unity/jumbo release build, with history, z database, rc file in place
.PHONY: in_place_release
in_place_release :
	$(CC) $(STD) $(release_flags) -DNCSH_IN_PLACE src/unity.c -o $(target)

# Install locally to DESTDIR (default /usr/bin/)
.PHONY: install
install : $(target)
	strip $(target)
	install -C $(target) $(DESTDIR)

# Run the tests that get ran in CI
.PHONY: check
check :
	set -e
	make test_fzf
	make test_ac
	make test_history
	make test_lexer
	make test_alias
	make test_prompt
	make test_arena
	make test_hashset
	make test_str
	make test_vars
	make test_logic
	make test_vm
	make test_vm_buffer
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
	make test_z
	make at
.PHONY: l
l :
	make check_local

# Run history tests
.PHONY: test_history
test_history :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/efile.c ./src/readline/hashset.c ./src/arena.c ./src/readline/history.c ./tests/history_tests.c -o ./bin/history_tests
	./bin/history_tests
.PHONY: th
th :
	make test_history

# Run history fuzzer
.PHONY: fuzz_history
fuzz_history :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) -DNCSH_HISTORY_TEST ./tests/history_fuzzing.c ./src/arena.c ./src/readline/history.c ./src/eskilib/efile.c ./src/readline/hashset.c -o ./bin/history_fuzz
	./bin/history_fuzz HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fh
fh :
	make fuzz_history

# Run autocompletion tests
.PHONY: test_ac
test_ac :
	 # $(CC) $(STD) $(debug_flags) -DAC_DEBUG ./src/arena.c ./src/readline/ac.c ./tests/ac_tests.c -o ./bin/ac_tests
	 $(CC) $(STD) $(debug_flags) ./src/arena.c ./src/readline/ac.c ./tests/ac_tests.c -o ./bin/ac_tests
	 ./bin/ac_tests
.PHONY: tac
tac :
	make test_ac

# Run autocompletion fuzzer
.PHONY: fuzz_ac
fuzz_ac :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/ac_fuzzing.c ./src/arena.c ./src/readline/ac.c -o ./bin/ac_fuzz
	./bin/ac_fuzz AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fac
fac :
	make fuzz_ac

# Run autocompletions benchmarks
.PHONY: bench_ac
bench_ac :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/arena.c ./src/readline/ac.c ./tests/ac_bench.c -o ./bin/ac_bench
	hyperfine --warmup 1000 --shell=none './bin/ac_bench'
.PHONY: bac
bac :
	make bench_ac

.PHONY: bench_acr
bench_acr :
	$(CC) $(STD) $(release_flags) -DNDEBUG ./src/arena.c ./src/readline/ac.c ./tests/ac_bench.c -o ./bin/ac_bench
	hyperfine --warmup 1000 --shell=none './bin/ac_bench'
.PHONY: bacr
bacr :
	make bench_acr

.PHONY: bench_ac_tests
bench_ac_tests :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/arena.c ./src/readline/ac.c ./tests/ac_tests.c -o ./bin/ac_tests
	hyperfine --warmup 1000 --shell=none './bin/ac_tests'
.PHONY: bact
bact :
	make bench_ac_tests

# Run lexer tests
.PHONY: test_lexer
test_lexer :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/interpreter/tokens.c ./src/interpreter/lexer.c ./tests/lexer_tests.c -o ./bin/lexer_tests
	./bin/lexer_tests
.PHONY: tlx
tlx :
	make test_lexer

# Run lexer fuzzer
.PHONY: fuzz_lexer
fuzz_lexer :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/lexer_fuzzing.c ./src/arena.c ./src/interpreter/tokens.c ./src/interpreter/lexer.c -o ./bin/lexer_fuzz
	./bin/lexer_fuzz LEXER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fp
fp :
	make fuzz_lexer

# Run lexer benchmarks
.PHONY: bench_lexer
bench_lexer :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/arena.c ./src/interpreter/vars.c ./src/interpreter/tokens.c ./src/interpreter/lexer.c ./tests/lexer_bench.c -o ./bin/lexer_bench
	hyperfine --warmup 1000 --shell=none './bin/lexer_bench'
.PHONY: bl
bl :
	make bench_lexer

.PHONY: bench_lexer_tests
bench_lexer_tests :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/arena.c ./src/interpreter/vars.c ./src/interpreter/tokens.c ./src/interpreter/lexer.c ./tests/lexer_tests.c -o ./bin/lexer_tests
	hyperfine --warmup 1000 --shell=none './bin/lexer_tests'
.PHONY: blt
blt :
	make bench_lexer_tests

# Run parser tests
.PHONY: test_parser
test_parser :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/alias.c ./src/env.c ./src/interpreter/lexer.c ./src/interpreter/vars.c ./src/interpreter/logic.c ./src/interpreter/tokens.c ./src/interpreter/parser.c ./tests/parser_tests.c -o ./bin/parser_tests
	./bin/parser_tests
.PHONY: tp
tp :
	make test_parser

.PHONY: bench_parser
bench_parser :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/alias.c ./src/env.c ./src/interpreter/lexer.c ./src/interpreter/vars.c ./src/interpreter/logic.c ./src/interpreter/tokens.c ./src/interpreter/parser.c ./tests/parser_tests.c -o ./bin/parser_tests
	hyperfine --warmup 1000 --shell=none './bin/parser_tests'
.PHONY: bp
bp :
	make bench_parser

# Run z tests
.PHONY: test_z
test_z :
	gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DZ_TEST ./src/arena.c ./src/z/fzf.c ./src/z/z.c ./tests/z_tests.c -o ./bin/z_tests
	./bin/z_tests
.PHONY: tz
tz :
	make test_z

# Run z fuzzer
.PHONY: fuzz_z
fuzz_z :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/z_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_fuzz
	./bin/z_fuzz Z_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fz
fz :
	make fuzz_z

# Run z add fuzzer
.PHONY: fuzz_z_add
fuzz_z_add :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/z_add_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_add_fuzz
	./bin/z_add_fuzz Z_ADD_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fza
fza :
	make fuzz_z_add

# Run fzf tests
.PHONY: test_fzf
test_fzf :
	$(CC) $(STD) -fsanitize=address,undefined,leak -g ./src/arena.c ./src/z/fzf.c ./tests/lib/examiner.c ./tests/fzf_tests.c -o ./bin/fzf_tests
	@LD_LIBRARY_PATH=/usr/local/lib:./bin/:${LD_LIBRARY_PATH} ./bin/fzf_tests
.PHONY: tf
tf :
	make test_fzf

# Run alias tests
.PHONY: test_alias
test_alias :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/alias.c ./tests/alias_tests.c -o ./bin/alias_tests
	./bin/alias_tests
.PHONY: tal
tal :
	make test_alias

# Run prompt tests
.PHONY: test_prompt
test_prompt :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/readline/prompt.c ./tests/prompt_tests.c -o ./bin/prompt_tests
	./bin/prompt_tests
.PHONY: tpr
tpr :
	make test_prompt

# Run arena tests
.PHONY: test_arena
test_arena :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./tests/arena_tests.c -o ./bin/arena_tests
	./bin/arena_tests
.PHONY: ta
ta :
	make test_arena

# Run str tests
.PHONY: test_str
test_str :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./tests/str_tests.c -o ./bin/str_tests
	./bin/str_tests
.PHONY: ts
ts :
	make test_str

# Run variables (vars) tests
.PHONY: test_vars
test_vars :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/interpreter/vars.c ./tests/vars_tests.c -o ./bin/vars_tests
	./bin/vars_tests
.PHONY: tv
tv :
	make test_vars

# Run VM sanity tests
.PHONY: test_vm
test_vm :
	$(CC) $(STD) $(debug_flags) -DNCSH_VM_TEST ./src/arena.c ./src/interpreter/tokens.c ./src/interpreter/lexer.c ./src/eskilib/efile.c ./src/readline/hashset.c ./src/interpreter/vars.c ./src/readline/history.c ./src/z/fzf.c ./src/z/z.c ./src/env.c ./src/alias.c ./src/config.c ./src/interpreter/logic.c ./src/interpreter/vm/vm_buffer.c ./src/interpreter/vm/vm.c ./src/interpreter/semantic_analyzer.c ./src/interpreter/parser.c ./src/interpreter/vm/builtins.c ./src/interpreter/vm/pipe.c ./src/interpreter/vm/redirection.c ./tests/vm_tests.c -o ./bin/vm_tests
	./bin/vm_tests
.PHONY: tvm
tvm :
	make test_vm

# Run hashset tests
.PHONY: test_hashset
test_hashset :
	$(CC) $(STD) $(debug_flags) -DNCSH_VM_TEST ./src/arena.c ./src/readline/hashset.c ./tests/hashset_tests.c -o ./bin/hashset_tests
	./bin/hashset_tests
.PHONY: ths
ths :
	make test_hashset

# Run VM logic tests
.PHONY: test_logic
test_logic :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/interpreter/tokens.c ./src/interpreter/lexer.c ./src/interpreter/logic.c ./tests/logic_tests.c -o ./bin/logic_tests
	./bin/logic_tests
.PHONY: tl
tl :
	make test_logic

# Run VM buffer processing tests
.PHONY: test_vm_buffer
test_vm_buffer :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/alias.c ./src/env.c ./src/interpreter/vars.c ./src/interpreter/tokens.c ./src/interpreter/lexer.c ./src/interpreter/logic.c ./src/interpreter/vm/vm_buffer.c ./src/interpreter/parser.c ./tests/vm_buffer_tests.c -o ./bin/vm_buffer_tests
	./bin/vm_buffer_tests
.PHONY: tvb
tvb :
	make test_vm_buffer

# Format the project
.PHONY: clang_format
clang_format :
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
.PHONY: cf
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
.PHONY: cl
cl :
	make clean
