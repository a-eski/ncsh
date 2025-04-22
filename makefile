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
objects = obj/main.o obj/arena.o obj/noninteractive.o obj/ncreadline.o obj/vm.o obj/vm_tokenizer.o obj/terminal.o obj/efile.o obj/emap.o obj/var.o obj/parser.o obj/vm_builtins.o obj/history.o obj/ac.o obj/config.o obj/fzf.o obj/z.o
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

.PHONY: release, r
release r:
	make RELEASE=1

.PHONY: debug, d
debug d :
	make -B RELEASE=0

.PHONY: unity, u
unity u :
	$(CC) $(STD) $(release_flags) src/unity.c -o $(target)

.PHONY: unity_debug, ud
unity_debug ud :
	$(CC) $(STD) $(debug_flags) src/unity.c -o $(target)

.PHONY: install
install : $(target)
	strip $(target)
	install -C $(target) $(DESTDIR)

.PHONY: check, c
check c:
	set -e
	make test_fzf
	make test_ac
	make test_history
	make test_parser
	make test_config
	make test_readline
	make test_arena
	make test_emap
	make test_estr
	make test_var
	make test_vm

.PHONY: acceptance_tests, at
acceptance_tests at:
	chmod +x ./acceptance_tests.sh
	./acceptance_tests.sh

.PHONY: check_local, l
check_local l:
	set -e
	make check
	make test_z
	make at

.PHONY: test_history, th
test_history th:
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/eskilib/etest.c ./src/eskilib/efile.c ./src/eskilib/emap.c ./src/arena.c ./src/readline/history.c ./tests/history_tests.c -o ./bin/history_tests
	./bin/history_tests

.PHONY: fuzz_history, fh
fuzz_history fh:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) -DNCSH_HISTORY_TEST ./tests/history_fuzzing.c ./src/arena.c ./src/readline/history.c ./src/eskilib/efile.c ./src/eskilib/emap.c -o ./bin/history_fuzz
	./bin/history_fuzz HISTORY_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: test_ac, tac
test_ac tac:
	 # $(CC) $(STD) $(debug_flags) -DAC_DEBUG ./src/eskilib/etest.c ./src/arena.c ./src/readline/ac.c ./tests/ac_tests.c -o ./bin/ac_tests
	 $(CC) $(STD) $(debug_flags) ./src/eskilib/etest.c ./src/arena.c ./src/readline/ac.c ./tests/ac_tests.c -o ./bin/ac_tests
	 ./bin/ac_tests

.PHONY: fuzz_ac, fac
fuzz_ac fac:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/ac_fuzzing.c ./src/arena.c ./src/readline/ac.c -o ./bin/ac_fuzz
	./bin/ac_fuzz AUTOCOMPLETIONS_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192

.PHONY: bench_ac, bac
bench_ac bac:
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/readline/ac.c ./tests/ac_bench.c -o ./bin/ac_bench
	hyperfine --warmup 1000 --shell=none './bin/ac_bench'

.PHONY: bench_acr, bacr
bench_acr bacr:
	$(CC) $(STD) $(release_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/readline/ac.c ./tests/ac_bench.c -o ./bin/ac_bench
	hyperfine --warmup 1000 --shell=none './bin/ac_bench'

.PHONY: bench_ac_tests, bact
bench_ac_tests bact:
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/readline/ac.c ./tests/ac_tests.c -o ./bin/ac_tests
	hyperfine --warmup 1000 --shell=none './bin/ac_tests'

.PHONY: test_parser, tp
test_parser tp:
	$(CC) $(STD) $(debug_flags) ./src/eskilib/etest.c ./src/arena.c ./src/var.c ./src/parser.c ./tests/parser_tests.c -o ./bin/parser_tests
	./bin/parser_tests

.PHONY: fuzz_parser, fp
fuzz_parser fp:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 $(STD) $(fuzz_flags) ./tests/parser_fuzzing.c ./src/arena.c ./src/var.c ./src/parser.c -o ./bin/parser_fuzz
	./bin/parser_fuzz PARSER_CORPUS/ -detect_leaks=0 -rss_limit_mb=4096

.PHONY: bench_parser, bp
bench_parser bp:
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/var.c ./src/parser.c ./tests/parser_bench.c -o ./bin/parser_bench
	hyperfine --warmup 1000 --shell=none './bin/parser_bench'

.PHONY: bench_parser_tests, bpt
bench_parser_tests bpt:
	$(CC) $(STD) $(debug_flags) -DNDEBUG ./src/eskilib/etest.c ./src/arena.c ./src/var.c ./src/parser.c ./tests/parser_tests.c -o ./bin/parser_tests
	hyperfine --warmup 1000 --shell=none './bin/parser_tests'

.PHONY: bench_parser_and_vm, bpv
bench_parser_and_vm vpb:
	hyperfine --warmup 100 --shell /bin/ncsh 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
	rm t.txt t2.txt

.PHONY: bash_bench_parser_and_vm, bbpv
bash_bench_parser_and_vm bbpv:
	hyperfine --warmup 100 --shell /bin/bash 'ls' 'ls | sort' 'ls > t.txt' 'ls | sort | wc -c' 'ls | sort | wc -c > t2.txt'
	rm t.txt t2.txt

.PHONY: test_z, tz
test_z tz:
	gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DZ_TEST ./src/arena.c ./src/eskilib/etest.c ./src/z/fzf.c ./src/z/z.c ./tests/z_tests.c -o ./bin/z_tests
	./bin/z_tests

.PHONY: fuzz_z, fz
fuzz_z fz:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/z_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_fuzz
	./bin/z_fuzz Z_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192

.PHONY: fuzz_z_add, fza
fuzz_z_add fza:
	chmod +x ./create_corpus_dirs.sh
	./create_corpus_dirs.sh
	clang-19 -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./src/arena.c ./tests/z_add_fuzzing.c ./src/z/fzf.c ./src/z/z.c -o ./bin/z_add_fuzz
	./bin/z_add_fuzz Z_ADD_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192

.PHONY: test_fzf, tf
test_fzf tf:
	$(CC) $(STD) -fsanitize=address,undefined,leak -g ./src/arena.c ./src/z/fzf.c ./tests/lib/examiner.c ./tests/fzf_tests.c -o ./bin/fzf_tests
	@LD_LIBRARY_PATH=/usr/local/lib:./bin/:${LD_LIBRARY_PATH} ./bin/fzf_tests

.PHONY: test_config, tc
test_config tc:
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/eskilib/etest.c ./src/eskilib/efile.c ./src/config.c ./tests/config_tests.c -o ./bin/config_tests
	./bin/config_tests

.PHONY: test_readline, tr
test_readline tr:
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/eskilib/etest.c ./src/eskilib/efile.c ./src/eskilib/emap.c ./src/readline/terminal.c ./src/readline/ac.c ./src/readline/history.c ./src/readline/ncreadline.c ./tests/ncreadline_tests.c -o ./bin/ncreadline_tests
	./bin/ncreadline_tests

.PHONY: test_arena, ta
test_arena ta:
	$(CC) $(STD) $(debug_flags) -DNCSH_HISTORY_TEST ./src/arena.c ./src/eskilib/etest.c ./tests/arena_tests.c -o ./bin/arena_tests
	./bin/arena_tests

.PHONY: test_emap, te
test_emap te:
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/eskilib/emap.c ./src/eskilib/etest.c ./tests/emap_tests.c -o ./bin/emap_tests
	./bin/emap_tests

.PHONY: test_estr, ts
test_estr ts:
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/eskilib/etest.c ./tests/estr_tests.c -o ./bin/estr_tests
	./bin/estr_tests

.PHONY: test_var, tv
test_var tv:
	$(CC) $(STD) $(debug_flags) ./src/arena.c ./src/var.c ./src/eskilib/etest.c ./tests/var_tests.c -o ./bin/var_tests
	./bin/var_tests

.PHONY: test_vm, tvm
test_vm tvm:
	$(CC) $(STD) $(debug_flags) -DNCSH_VM_TEST ./src/arena.c ./src/parser.c ./src/eskilib/efile.c ./src/eskilib/emap.c ./src/var.c ./src/readline/history.c ./src/z/fzf.c ./src/z/z.c ./src/config.c ./src/vm/vm.c ./src/vm/vm_tokenizer.c ./src/vm/vm_builtins.c ./src/eskilib/etest.c ./tests/vm_tests.c -o ./bin/vm_tests
	./bin/vm_tests

.PHONY: clang_format, cf
clang_format cf:
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;

.PHONY: clean, cl
clean cl:
	rm $(target) $(objects)
