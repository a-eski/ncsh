#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "lib/arena_test_helper.h"
#include "../src/eskilib/eskilib_test.h"
#include "../src/readline/autocompletions.h"
#include "../src/defines.h"

void autocompletions_bench(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    autocompletions_add("ls", sizeof("ls"), tree, &arena);
    autocompletions_add("ls | wc -c", sizeof("ls | wc -c"), tree, &arena);
    autocompletions_add("ls | sort", sizeof("ls | sort"), tree, &arena);
    autocompletions_add("ls | sort | wc -c", sizeof("ls | sort | wc -c"), tree, &arena);
    autocompletions_add("ls > t.txt", sizeof("ls > t.txt"), tree, &arena);
    autocompletions_add("cat t.txt", sizeof("cat t.txt"), tree, &arena);
    autocompletions_add("rm t.txt", sizeof("rm t.txt"), tree, &arena);
    autocompletions_add("ss", sizeof("ss"), tree, &arena);
    autocompletions_add("nvim", sizeof("nvim"), tree, &arena);
    autocompletions_add("nvim .", sizeof("nvim ."), tree, &arena);
    autocompletions_add("ls", sizeof("ls"), tree, &arena);
    autocompletions_add("ls", sizeof("ls"), tree, &arena);
    autocompletions_add("z ncsh", sizeof("z ncsh"), tree, &arena);
    autocompletions_add("make ud", sizeof("make ud"), tree, &arena);
    autocompletions_add("ls -a", sizeof("ls -a"), tree, &arena);
    autocompletions_add("echo hello", sizeof("echo hello"), tree, &arena);
    autocompletions_add("version", sizeof("version"), tree, &arena);
    autocompletions_add("history", sizeof("history"), tree, &arena);
    autocompletions_add("./bin/ncsh", sizeof("./bin/ncsh"), tree, &arena);
    autocompletions_add("z ncsh2", sizeof("z ncsh2"), tree, &arena);
    autocompletions_add("nvim src/readline/ncsh_readline.c", sizeof("nvim src/readline/ncsh_readline.c"), tree, &arena);
    autocompletions_add("nvim .", sizeof("nvim ."), tree, &arena);
    autocompletions_add("git diff", sizeof("git diff"), tree, &arena);
    autocompletions_add("git restore", sizeof("git restore"), tree, &arena);
    autocompletions_add("git restore src/ncsh_readline.c", sizeof("git restore src/ncsh_readline.c"), tree, &arena);
    autocompletions_add("git restore src/readline/ncsh_readline.c", sizeof("git restore src/readline/ncsh_readline.c"), tree, &arena);
    autocompletions_add("nvim src/ncsh_arena.c", sizeof("nvim src/ncsh_arena.c"), tree, &arena);
    autocompletions_add("z print", sizeof("z print"), tree, &arena);
    autocompletions_add("z rm /home/alex/readline", sizeof("z rm /home/alex/readline"), tree, &arena);
    autocompletions_add("z rm /home/alex/readline/usr/local", sizeof("z rm /home/alex/readline/usr/local"), tree, &arena);
    autocompletions_add("z rm /home/alex/ncsh/development/cash", sizeof("z rm /home/alex/ncsh/development/cash"), tree, &arena);
    autocompletions_add("z rm /home/alex/ncsh/development/curses", sizeof("z rm /home/alex/ncsh/development/curses"), tree, &arena);
    autocompletions_add("z rm /home/alex/../", sizeof("z rm /home/alex/../"), tree, &arena);
    autocompletions_add("z readline", sizeof("z readline"), tree, &arena);
    autocompletions_add("pwd", sizeof("pwd"), tree, &arena);
    autocompletions_add("z", sizeof("z"), tree, &arena);
    autocompletions_add("nvim makefile", sizeof("nvim makefile"), tree, &arena);
    autocompletions_add("git diff makefile", sizeof("git diff makefile"), tree, &arena);
    autocompletions_add("git restore makefile", sizeof("git restore makefile"), tree, &arena);
    autocompletions_add("git pull origin main", sizeof("git pull origin main"), tree, &arena);
    autocompletions_add("z eskilib", sizeof("z eskilib"), tree, &arena);
    autocompletions_add("git config pull.rebase true", sizeof("git config pull.rebase true"), tree, &arena);
    autocompletions_add("nvim src/z/z.c", sizeof("nvim src/z/z.c"), tree, &arena);
    autocompletions_add("nvim src/readline/ncsh_input.c", sizeof("nvim src/readline/ncsh_input.c"), tree, &arena);
    autocompletions_add("nvim src/ncsh.c", sizeof("nvim src/ncsh.c"), tree, &arena);
    autocompletions_add("echo 'hello'", sizeof("echo 'hello'"), tree, &arena);
    autocompletions_add("nvim src/vm", sizeof("nvim src/vm"), tree, &arena);
    autocompletions_add("ls | sort", sizeof("ls | sort"), tree, &arena);
    autocompletions_add("nvim src/ncsh_defines.h", sizeof("nvim src/ncsh_defines.h"), tree, &arena);
    autocompletions_add("ls -a --color", sizeof("ls -a --color"), tree, &arena);
    autocompletions_add("m l", sizeof("m l"), tree, &arena);
    autocompletions_add("m check", sizeof("m check"), tree, &arena);
    autocompletions_add("nvim src/z/fzf.c", sizeof("nvim src/z/fzf.c"), tree, &arena);
    autocompletions_add("nvim src/vm/ncsh_vm.c", sizeof("nvim src/vm/ncsh_vm.c"), tree, &arena);
    autocompletions_add("make", sizeof("make"), tree, &arena);
    autocompletions_add("sudo make install", sizeof("sudo make install"), tree, &arena);
    autocompletions_add("cd ncsh2", sizeof("cd ncsh2"), tree, &arena);
    autocompletions_add("make -B", sizeof("make -B"), tree, &arena);
    autocompletions_add("git branch", sizeof("git branch"), tree, &arena);
    autocompletions_add("git add .", sizeof("git add ."), tree, &arena);
    autocompletions_add("nvim .gitignore", sizeof("nvim .gitignore"), tree, &arena);
    autocompletions_add("git rm Z_CORPUS", sizeof("git rm Z_CORPUS"), tree, &arena);
    autocompletions_add("git rm -r Z_CORPUS", sizeof("git rm -r Z_CORPUS"), tree, &arena);
    autocompletions_add("git rm -r Z_ADD_CORPUS", sizeof("git rm -r Z_ADD_CORPUS"), tree, &arena);
    autocompletions_add("git push -u origin fuzz-testing-and-longjmp", sizeof("git push -u origin fuzz-testing-and-longjmp"), tree, &arena);
    autocompletions_add("cd ..", sizeof("cd .."), tree, &arena);
    autocompletions_add("git clone https://github.com/a-eski/ncsh.git ./ncsh3", sizeof("git clone https://github.com/a-eski/ncsh.git ./ncsh3"), tree, &arena);
    autocompletions_add("cd ncsh3", sizeof("cd ncsh3"), tree, &arena);
    autocompletions_add("z ncsh3", sizeof("z ncsh3"), tree, &arena);
    autocompletions_add("z '.config'", sizeof("z '.config'"), tree, &arena);
    autocompletions_add("cd nvim", sizeof("cd nvim"), tree, &arena);
    autocompletions_add("git stash", sizeof("git stash"), tree, &arena);
    autocompletions_add("z ..", sizeof("z .."), tree, &arena);
    autocompletions_add("cd ncsh", sizeof("cd ncsh"), tree, &arena);
    autocompletions_add("z rm /home/alex/ncsh2/ncsh3", sizeof("z rm /home/alex/ncsh2/ncsh3"), tree, &arena);
    autocompletions_add("m fz", sizeof("m fz"), tree, &arena);
    autocompletions_add("z rm /home/alex/ncsh2/ncsh2", sizeof("z rm /home/alex/ncsh2/ncsh2"), tree, &arena);
    autocompletions_add("make clean", sizeof("make clean"), tree, &arena);
    autocompletions_add("z src", sizeof("z src"), tree, &arena);
    autocompletions_add("vv", sizeof("vv"), tree, &arena);
    autocompletions_add("cd src", sizeof("cd src"), tree, &arena);
    autocompletions_add("z ncsh2/src", sizeof("z ncsh2/src"), tree, &arena);
    autocompletions_add("z rm /home/alex/ncsh3/ncsh2", sizeof("z rm /home/alex/ncsh3/ncsh2"), tree, &arena);
    autocompletions_add("z rm /home/alex/ncsh2/eskilib", sizeof("z rm /home/alex/ncsh2/eskilib"), tree, &arena);
    autocompletions_add("nvim ncsh", sizeof("nvim ncsh"), tree, &arena);
    autocompletions_add("nvim src/readline", sizeof("nvim src/readline"), tree, &arena);
    autocompletions_add("nvim src/readline/ncsh_terminal.c", sizeof("nvim src/readline/ncsh_terminal.c"), tree, &arena);
    autocompletions_add(":w", sizeof(":w"), tree, &arena);
    autocompletions_add("git restore src/readline/ncsh_terminal.c", sizeof("git restore src/readline/ncsh_terminal.c"), tree, &arena);
    autocompletions_add("clear", sizeof("clear"), tree, &arena);
    autocompletions_add("make l", sizeof("make l"), tree, &arena);
    autocompletions_add("nvim src", sizeof("nvim src"), tree, &arena);
    autocompletions_add("git switch -c sigwinch", sizeof("git switch -c sigwinch"), tree, &arena);
    autocompletions_add("make fuzz_parser", sizeof("make fuzz_parser"), tree, &arena);
    autocompletions_add("nvim ncsh_readline.c", sizeof("nvim ncsh_readline.c"), tree, &arena);
    autocompletions_add("history rm vim", sizeof("history rm vim"), tree, &arena);
    autocompletions_add("history rm 'vim .'", sizeof("history rm 'vim .'"), tree, &arena);
    autocompletions_add("nvim README.md", sizeof("nvim README.md"), tree, &arena);
    autocompletions_add("make fuzz_history", sizeof("make fuzz_history"), tree, &arena);
    autocompletions_add("/bin/bash", sizeof("/bin/bash"), tree, &arena);
    autocompletions_add("git push -u origin sigwinch", sizeof("git push -u origin sigwinch"), tree, &arena);
    autocompletions_add("nvim src/readline/ncsh_autocompletions.c", sizeof("nvim src/readline/ncsh_autocompletions.c"), tree, &arena);
    autocompletions_add("m ba", sizeof("m ba"), tree, &arena);
    autocompletions_add("make tac", sizeof("make tac"), tree, &arena);
    autocompletions_add("make ba", sizeof("make ba"), tree, &arena);
    autocompletions_add("git commit -m 'trie optimizations: switch back impl'", sizeof("git commit -m 'trie optimizations: switch back impl'"), tree, &arena);
    autocompletions_add("git push", sizeof("git push"), tree, &arena);
    autocompletions_add("git rm Z_CORPUS/.gitignore", sizeof("git rm Z_CORPUS/.gitignore"), tree, &arena);
    autocompletions_add("git rm Z_ADD_CORPUS", sizeof("git rm Z_ADD_CORPUS"), tree, &arena);
    autocompletions_add("git rm Z_ADD_CORPUS/.gitignore", sizeof("git rm Z_ADD_CORPUS/.gitignore"), tree, &arena);
    autocompletions_add("git rm NCSH_AUTOCOMPLETIONS_CORPUS/.gitignore", sizeof("git rm NCSH_AUTOCOMPLETIONS_CORPUS/.gitignore"), tree, &arena);
    autocompletions_add("git rm NCSH_AUTOCOMPLETIONS_CORPUS", sizeof("git rm NCSH_AUTOCOMPLETIONS_CORPUS"), tree, &arena);
    autocompletions_add("git rm NCSH_HISTORY_CORPUS/.gitignore", sizeof("git rm NCSH_HISTORY_CORPUS/.gitignore"), tree, &arena);
    autocompletions_add("git rm NCSH_PARSER_CORPUS/.gitignore", sizeof("git rm NCSH_PARSER_CORPUS/.gitignore"), tree, &arena);
    autocompletions_add("nvim", sizeof("nvim"), tree, &arena);
    autocompletions_add("nvim src/ncsh_noninteractive.c", sizeof("nvim src/ncsh_noninteractive.c"), tree, &arena);
    autocompletions_add("make bpv", sizeof("make bpv"), tree, &arena);
    autocompletions_add("make thta", sizeof("make thta"), tree, &arena);
    autocompletions_add("git restore *", sizeof("git restore *"), tree, &arena);
    autocompletions_add("git restore tests/ncsh_autocompletions_tests.c", sizeof("git restore tests/ncsh_autocompletions_tests.c"), tree, &arena);
    autocompletions_add("git restore src/readline/ncsh_history.h", sizeof("git restore src/readline/ncsh_history.h"), tree, &arena);
    autocompletions_add("nvim src/vm/vm.c", sizeof("nvim src/vm/vm.c"), tree, &arena);
    autocompletions_add("nvim src/readline/ncreadlin.c", sizeof("nvim src/readline/ncreadlin.c"), tree, &arena);
    autocompletions_add("nvim src/readline/ncreadline.c", sizeof("nvim src/readline/ncreadline.c"), tree, &arena);
    autocompletions_add("nvim tests", sizeof("nvim tests"), tree, &arena);
    autocompletions_add("make at", sizeof("make at"), tree, &arena);
    autocompletions_add("nvim acceptance_tests/tests", sizeof("nvim acceptance_tests/tests"), tree, &arena);
    autocompletions_add("nvim acceptance_tests/tests/common.rb", sizeof("nvim acceptance_tests/tests/common.rb"), tree, &arena);
    autocompletions_add("ls", sizeof("ls"), tree, &arena);
    autocompletions_add("ls", sizeof("ls"), tree, &arena);
    autocompletions_add("ls -a", sizeof("ls -a"), tree, &arena);
    autocompletions_add("nvim .", sizeof("nvim ."), tree, &arena);
    autocompletions_add("nvim .", sizeof("nvim ."), tree, &arena);
    autocompletions_add("nvim .", sizeof("nvim ."), tree, &arena);
    autocompletions_add("git diff", sizeof("git diff"), tree, &arena);
    autocompletions_add("nvim src/readline/readline.c", sizeof("nvim src/readline/readline.c"), tree, &arena);
    autocompletions_add("nvim .", sizeof("nvim ."), tree, &arena);
    autocompletions_add("z ncsh2", sizeof("z ncsh2"), tree, &arena);
    autocompletions_add("nvim src/readline/readline.c", sizeof("nvim src/readline/readline.c"), tree, &arena);
    autocompletions_add("nvim src", sizeof("nvim src"), tree, &arena);
    autocompletions_add("git stash pop", sizeof("git stash pop"), tree, &arena);
    autocompletions_add("nvim .", sizeof("nvim ."), tree, &arena);
    autocompletions_add("make ud", sizeof("make ud"), tree, &arena);
    autocompletions_add("./bin/ncsh", sizeof("./bin/ncsh"), tree, &arena);
    autocompletions_add("git diff", sizeof("git diff"), tree, &arena);
    autocompletions_add("nvim src/readline", sizeof("nvim src/readline"), tree, &arena);
    autocompletions_add("git stash pop", sizeof("git stash pop"), tree, &arena);
    autocompletions_add("nvim .", sizeof("nvim ."), tree, &arena);
    autocompletions_add("make l", sizeof("make l"), tree, &arena);
    autocompletions_add("ls -a --color", sizeof("ls -a --color"), tree, &arena);
    autocompletions_add("version", sizeof("version"), tree, &arena);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    uint_fast32_t match_count = autocompletions_get("l", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("ls", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("n", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("nv", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("nvi", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("nvim", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("v", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("ve", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("m", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("ma", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("mak", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("make", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("g", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("gi", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git ", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git d", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git di", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git dif", autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git diff", autocomplete, tree, scratch_arena);
    eskilib_assert(match_count);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main(void)
{
    autocompletions_bench();
}
/* Before change
    uint_fast32_t match_count = autocompletions_get("l", sizeof("l"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("ls", sizeof("ls"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("n", sizeof("n"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("nv", sizeof("nv"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("nvi", sizeof("nvi"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("nvim", sizeof("nvim"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("v", sizeof("v"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("ve", sizeof("ve"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("m", sizeof("m"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("ma", sizeof("ma"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("mak", sizeof("mak"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("make", sizeof("make"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("g", sizeof("g"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("gi", sizeof("gi"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git", sizeof("git"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git ", sizeof("git "), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git d", sizeof("git d"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git di", sizeof("git di"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git dif", sizeof("git dif"), autocomplete, tree, scratch_arena);
    match_count = autocompletions_get("git diff", sizeof("git diff"), autocomplete, tree, scratch_arena);
*/
