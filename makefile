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
objects = obj/main.o obj/arena.o obj/noninteractive.o obj/ncreadline.o obj/vm_buffer.o obj/vm.o obj/syntax_validator.o obj/logic.o obj/preprocessor.o obj/terminal.o obj/efile.o obj/hashset.o obj/vars.o obj/args.o obj/parser.o obj/builtins.o obj/history.o obj/ac.o obj/env.o obj/alias.o obj/config.o obj/fzf.o obj/z.o
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
release:
	make RELEASE=1

.PHONY: debug
debug :
	make -B RELEASE=0

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
ud:
	make unity_debug

.PHONY: install
install : $(target)
	strip $(target)
	install -C $(target) $(DESTDIR)

.PHONY: check
check :
	set -e
	make test_fzf
	make test_ac
	make test_history
	make test_parser
	make test_alias
	make test_readline
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
	make at
.PHONY: l
l :
	make check_local

.PHONY: test_history
test_history :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/efile.c ./src/readline/hashset.c ./src/arena.c ./src/readline/history.c ./tests/history_tests.c -o ./bin/history_tests
	./bin/history_tests
.PHONY: th
th :
	make test_history

.PHONY: fuzz_history
fuzz_history :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) -DNCSH_HISTORY_TEST ./tests/history_fuzzing.c ./src/arena.c ./src/readline/history.c ./src/eskilib/efile.c ./src/readline/hashset.c -o ./bin/history_fuzz
	./bin/history_fuzz HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fh
fh :
	make fuzz_history

.PHONY: test_ac
test_ac :
	 # $(CC) $(STD) $(debug_flags) -DAC_DEBUG ./src/arena.c ./src/readline/ac.c ./tests/ac_tests.c -o ./bin/ac_tests
	 $(CC) $(STD) $(debug_flags) ./src/arena.c ./src/readline/ac.c ./tests/ac_tests.c -o ./bin/ac_tests
	 ./bin/ac_tests
.PHONY: tac
tac :
	make test_ac

.PHONY: fuzz_ac
fuzz_ac :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/ac_fuzzing.c ./src/arena.c ./src/readline/ac.c -o ./bin/ac_fuzz
	./bin/ac_fuzz AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fac
fac :
	make fuzz_ac

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

.PHONY: test_parser
test_parser :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/args.c ./src/parser.c ./tests/parser_tests.c -o ./bin/parser_tests
	./bin/parser_tests
.PHONY: tp
tp :
	make test_parser

.PHONY: fuzz_parser
fuzz_parser :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/parser_fuzzing.c ./src/arena.c ./src/args.c ./src/parser.c -o ./bin/parser_fuzz
	./bin/parser_fuzz PARSER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096
.PHONY: fp
fp :
	make fuzz_parser

.PHONY: bench_parser
bench_parser :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/arena.c ./src/vm/vars.c ./src/args.c ./src/parser.c ./tests/parser_bench.c -o ./bin/parser_bench
	hyperfine --warmup 1000 --shell=none './bin/parser_bench'
.PHONY: bp
bp :
	make bench_parser

.PHONY: bench_parser_tests
bench_parser_tests :
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/arena.c ./src/vm/vars.c ./src/args.c ./src/parser.c ./tests/parser_tests.c -o ./bin/parser_tests
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
	gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DZ_TEST ./src/arena.c ./src/z/fzf.c ./src/z/z.c ./tests/z_tests.c -o ./bin/z_tests
	./bin/z_tests
.PHONY: tz
tz :
	make test_z

.PHONY: fuzz_z
fuzz_z :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/z_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_fuzz
	./bin/z_fuzz Z_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fz
fz :
	make fuzz_z

.PHONY: fuzz_z_add
fuzz_z_add :
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/z_add_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_add_fuzz
	./bin/z_add_fuzz Z_ADD_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192
.PHONY: fza
fza :
	make fuzz_z_add

.PHONY: test_fzf
test_fzf :
	$(CC) $(STD) -fsanitize=address,undefined,leak -g ./src/arena.c ./src/z/fzf.c ./tests/lib/examiner.c ./tests/fzf_tests.c -o ./bin/fzf_tests
	@LD_LIBRARY_PATH=/usr/local/lib:./bin/:${LD_LIBRARY_PATH} ./bin/fzf_tests
.PHONY: tf
tf :
	make test_fzf

.PHONY: test_alias
test_alias :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/alias.c ./tests/alias_tests.c -o ./bin/alias_tests
	./bin/alias_tests
.PHONY: tal
tal :
	make test_alias

.PHONY: test_readline
test_readline :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/eskilib/efile.c ./src/readline/hashset.c ./src/readline/terminal.c ./src/readline/ac.c ./src/readline/history.c ./src/readline/ncreadline.c ./tests/ncreadline_tests.c -o ./bin/ncreadline_tests
	./bin/ncreadline_tests
.PHONY: tr
tr :
	make test_readline

.PHONY: test_arena
test_arena :
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./tests/arena_tests.c -o ./bin/arena_tests
	./bin/arena_tests
.PHONY: ta
ta :
	make test_arena

.PHONY: test_str
test_str :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./tests/str_tests.c -o ./bin/str_tests
	./bin/str_tests
.PHONY: ts
ts :
	make test_str

.PHONY: test_vars
test_vars :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/vm/vars.c ./tests/vars_tests.c -o ./bin/vars_tests
	./bin/vars_tests
.PHONY: tv
tv :
	make test_vars

.PHONY: test_vm
test_vm :
	$(CC) $(STD) $(debug_flags) -DNCSH_VM_TEST ./src/arena.c ./src/args.c ./src/parser.c ./src/eskilib/efile.c ./src/readline/hashset.c ./src/vm/vars.c ./src/readline/history.c ./src/z/fzf.c ./src/z/z.c ./src/env.c ./src/alias.c ./src/config.c ./src/vm/logic.c ./src/vm/vm_buffer.c ./src/vm/vm.c ./src/vm/syntax_validator.c ./src/vm/preprocessor.c ./src/vm/builtins.c ./tests/vm_tests.c -o ./bin/vm_tests
	./bin/vm_tests
.PHONY: tvm
tvm :
	make test_vm

.PHONY: test_hashset
test_hashset :
	$(CC) $(STD) $(debug_flags) -DNCSH_VM_TEST ./src/arena.c ./src/readline/hashset.c ./tests/hashset_tests.c -o ./bin/hashset_tests
	./bin/hashset_tests
.PHONY: ths
ths :
	make test_hashset

.PHONY: test_logic
test_logic :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/args.c ./src/parser.c ./src/vm/logic.c ./tests/logic_tests.c -o ./bin/logic_tests
	./bin/logic_tests
.PHONY: tl
tl :
	make test_logic

.PHONY: test_vm_buffer
test_vm_buffer :
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/alias.c ./src/env.c ./src/vm/vars.c ./src/args.c ./src/parser.c ./src/vm/logic.c ./src/vm/vm_buffer.c ./src/vm/preprocessor.c ./tests/vm_buffer_tests.c -o ./bin/vm_buffer_tests
	./bin/vm_buffer_tests
.PHONY: tvb
tvb :
	make test_vm_buffer

.PHONY: clang_format
clang_format :
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
.PHONY: cf
cf :
	make clang_format

.PHONY: scan_build
scan_build:
	scan-build-19 -analyze-headers make

.PHONY: clean
clean :
	rm $(target) $(objects)
.PHONY: cl
cl :
	make clean
