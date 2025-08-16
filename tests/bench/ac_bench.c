#include "../../src/defines.h" // used for macro NCSH_MAX_AUTOCOMPLETION_MATCHES
#include "../etest.h"
#include "../../src/io/ac.h"
#include "../lib/arena_test_helper.h"

void ac_bench()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", sizeof("ls"), tree, &arena);
    ac_add("ls | wc -c", sizeof("ls | wc -c"), tree, &arena);
    ac_add("ls | sort", sizeof("ls | sort"), tree, &arena);
    ac_add("ls | sort | wc -c", sizeof("ls | sort | wc -c"), tree, &arena);
    ac_add("ls > t.txt", sizeof("ls > t.txt"), tree, &arena);
    ac_add("cat t.txt", sizeof("cat t.txt"), tree, &arena);
    ac_add("rm t.txt", sizeof("rm t.txt"), tree, &arena);
    ac_add("ss", sizeof("ss"), tree, &arena);
    ac_add("nvim", sizeof("nvim"), tree, &arena);
    ac_add("nvim .", sizeof("nvim ."), tree, &arena);
    ac_add("ls", sizeof("ls"), tree, &arena);
    ac_add("ls", sizeof("ls"), tree, &arena);
    ac_add("z ncsh", sizeof("z ncsh"), tree, &arena);
    ac_add("make ud", sizeof("make ud"), tree, &arena);
    ac_add("ls -a", sizeof("ls -a"), tree, &arena);
    ac_add("echo hello", sizeof("echo hello"), tree, &arena);
    ac_add("version", sizeof("version"), tree, &arena);
    ac_add("history", sizeof("history"), tree, &arena);
    ac_add("./bin/ncsh", sizeof("./bin/ncsh"), tree, &arena);
    ac_add("z ncsh2", sizeof("z ncsh2"), tree, &arena);
    ac_add("nvim src/io/io.c", sizeof("nvim src/io/io.c"), tree, &arena);
    ac_add("nvim .", sizeof("nvim ."), tree, &arena);
    ac_add("git diff", sizeof("git diff"), tree, &arena);
    ac_add("git restore", sizeof("git restore"), tree, &arena);
    ac_add("git restore src/io.c", sizeof("git restore src/io.c"), tree, &arena);
    ac_add("git restore src/io/io.c", sizeof("git restore src/io/io.c"), tree,
           &arena);
    ac_add("nvim src/arena.c", sizeof("nvim src/arena.c"), tree, &arena);
    ac_add("z print", sizeof("z print"), tree, &arena);
    ac_add("z rm /home/alex/io", sizeof("z rm /home/alex/io"), tree, &arena);
    ac_add("z rm /home/alex/io/usr/local", sizeof("z rm /home/alex/io/usr/local"), tree, &arena);
    ac_add("z rm /home/alex/ncsh/development/cash", sizeof("z rm /home/alex/ncsh/development/cash"), tree, &arena);
    ac_add("z rm /home/alex/ncsh/development/curses", sizeof("z rm /home/alex/ncsh/development/curses"), tree, &arena);
    ac_add("z rm /home/alex/../", sizeof("z rm /home/alex/../"), tree, &arena);
    ac_add("z io", sizeof("z io"), tree, &arena);
    ac_add("pwd", sizeof("pwd"), tree, &arena);
    ac_add("z", sizeof("z"), tree, &arena);
    ac_add("nvim makefile", sizeof("nvim makefile"), tree, &arena);
    ac_add("git diff makefile", sizeof("git diff makefile"), tree, &arena);
    ac_add("git restore makefile", sizeof("git restore makefile"), tree, &arena);
    ac_add("git pull origin main", sizeof("git pull origin main"), tree, &arena);
    ac_add("z eskilib", sizeof("z eskilib"), tree, &arena);
    ac_add("git config pull.rebase true", sizeof("git config pull.rebase true"), tree, &arena);
    ac_add("nvim src/z/z.c", sizeof("nvim src/z/z.c"), tree, &arena);
    ac_add("nvim src/io/input.c", sizeof("nvim src/io/input.c"), tree, &arena);
    ac_add("nvim src/ncsh.c", sizeof("nvim src/ncsh.c"), tree, &arena);
    ac_add("echo 'hello'", sizeof("echo 'hello'"), tree, &arena);
    ac_add("nvim src/vm", sizeof("nvim src/vm"), tree, &arena);
    ac_add("ls | sort", sizeof("ls | sort"), tree, &arena);
    ac_add("nvim src/defines.h", sizeof("nvim src/defines.h"), tree, &arena);
    ac_add("ls -a --color", sizeof("ls -a --color"), tree, &arena);
    ac_add("m l", sizeof("m l"), tree, &arena);
    ac_add("m check", sizeof("m check"), tree, &arena);
    ac_add("nvim src/z/fzf.c", sizeof("nvim src/z/fzf.c"), tree, &arena);
    ac_add("nvim src/vm/vm.c", sizeof("nvim src/vm/vm.c"), tree, &arena);
    ac_add("make", sizeof("make"), tree, &arena);
    ac_add("sudo make install", sizeof("sudo make install"), tree, &arena);
    ac_add("cd ncsh2", sizeof("cd ncsh2"), tree, &arena);
    ac_add("make -B", sizeof("make -B"), tree, &arena);
    ac_add("git branch", sizeof("git branch"), tree, &arena);
    ac_add("git add .", sizeof("git add ."), tree, &arena);
    ac_add("nvim .gitignore", sizeof("nvim .gitignore"), tree, &arena);
    ac_add("git rm Z_CORPUS", sizeof("git rm Z_CORPUS"), tree, &arena);
    ac_add("git rm -r Z_CORPUS", sizeof("git rm -r Z_CORPUS"), tree, &arena);
    ac_add("git rm -r Z_ADD_CORPUS", sizeof("git rm -r Z_ADD_CORPUS"), tree, &arena);
    ac_add("git push -u origin fuzz-testing-and-longjmp", sizeof("git push -u origin fuzz-testing-and-longjmp"), tree,
           &arena);
    ac_add("cd ..", sizeof("cd .."), tree, &arena);
    ac_add("git clone https://github.com/a-eski/ncsh.git ./ncsh3",
           sizeof("git clone https://github.com/a-eski/ncsh.git ./ncsh3"), tree, &arena);
    ac_add("cd ncsh3", sizeof("cd ncsh3"), tree, &arena);
    ac_add("z ncsh3", sizeof("z ncsh3"), tree, &arena);
    ac_add("z '.config'", sizeof("z '.config'"), tree, &arena);
    ac_add("cd nvim", sizeof("cd nvim"), tree, &arena);
    ac_add("git stash", sizeof("git stash"), tree, &arena);
    ac_add("z ..", sizeof("z .."), tree, &arena);
    ac_add("cd ncsh", sizeof("cd ncsh"), tree, &arena);
    ac_add("z rm /home/alex/ncsh2/ncsh3", sizeof("z rm /home/alex/ncsh2/ncsh3"), tree, &arena);
    ac_add("m fz", sizeof("m fz"), tree, &arena);
    ac_add("z rm /home/alex/ncsh2/ncsh2", sizeof("z rm /home/alex/ncsh2/ncsh2"), tree, &arena);
    ac_add("make clean", sizeof("make clean"), tree, &arena);
    ac_add("z src", sizeof("z src"), tree, &arena);
    ac_add("vv", sizeof("vv"), tree, &arena);
    ac_add("cd src", sizeof("cd src"), tree, &arena);
    ac_add("z ncsh2/src", sizeof("z ncsh2/src"), tree, &arena);
    ac_add("z rm /home/alex/ncsh3/ncsh2", sizeof("z rm /home/alex/ncsh3/ncsh2"), tree, &arena);
    ac_add("z rm /home/alex/ncsh2/eskilib", sizeof("z rm /home/alex/ncsh2/eskilib"), tree, &arena);
    ac_add("nvim ncsh", sizeof("nvim ncsh"), tree, &arena);
    ac_add("nvim src/io", sizeof("nvim src/io"), tree, &arena);
    ac_add("nvim src/io/terminal.c", sizeof("nvim src/io/terminal.c"), tree, &arena);
    ac_add(":w", sizeof(":w"), tree, &arena);
    ac_add("git restore src/io/terminal.c", sizeof("git restore src/io/terminal.c"), tree,
           &arena);
    ac_add("clear", sizeof("clear"), tree, &arena);
    ac_add("make l", sizeof("make l"), tree, &arena);
    ac_add("nvim src", sizeof("nvim src"), tree, &arena);
    ac_add("git switch -c sigwinch", sizeof("git switch -c sigwinch"), tree, &arena);
    ac_add("make fuzz_parser", sizeof("make fuzz_parser"), tree, &arena);
    ac_add("nvim io.c", sizeof("nvim io.c"), tree, &arena);
    ac_add("history rm vim", sizeof("history rm vim"), tree, &arena);
    ac_add("history rm 'vim .'", sizeof("history rm 'vim .'"), tree, &arena);
    ac_add("nvim README.md", sizeof("nvim README.md"), tree, &arena);
    ac_add("make fuzz_history", sizeof("make fuzz_history"), tree, &arena);
    ac_add("/bin/bash", sizeof("/bin/bash"), tree, &arena);
    ac_add("git push -u origin sigwinch", sizeof("git push -u origin sigwinch"), tree, &arena);
    ac_add("nvim src/io/autocompletions.c", sizeof("nvim src/io/autocompletions.c"), tree,
           &arena);
    ac_add("m ba", sizeof("m ba"), tree, &arena);
    ac_add("make tac", sizeof("make tac"), tree, &arena);
    ac_add("make ba", sizeof("make ba"), tree, &arena);
    ac_add("git commit -m 'trie optimizations: switch back impl'",
           sizeof("git commit -m 'trie optimizations: switch back impl'"), tree, &arena);
    ac_add("git push", sizeof("git push"), tree, &arena);
    ac_add("git rm Z_CORPUS/.gitignore", sizeof("git rm Z_CORPUS/.gitignore"), tree, &arena);
    ac_add("git rm Z_ADD_CORPUS", sizeof("git rm Z_ADD_CORPUS"), tree, &arena);
    ac_add("git rm Z_ADD_CORPUS/.gitignore", sizeof("git rm Z_ADD_CORPUS/.gitignore"), tree, &arena);
    ac_add("git rm NCSH_AUTOCOMPLETIONS_CORPUS/.gitignore", sizeof("git rm NCSH_ac_CORPUS/.gitignore"), tree, &arena);
    ac_add("git rm NCSH_AUTOCOMPLETIONS_CORPUS", sizeof("git rm NCSH_AUTOCOMPLETIONS_CORPUS"), tree, &arena);
    ac_add("git rm NCSH_HISTORY_CORPUS/.gitignore", sizeof("git rm NCSH_HISTORY_CORPUS/.gitignore"), tree, &arena);
    ac_add("git rm NCSH_PARSER_CORPUS/.gitignore", sizeof("git rm NCSH_PARSER_CORPUS/.gitignore"), tree, &arena);
    ac_add("nvim", sizeof("nvim"), tree, &arena);
    ac_add("nvim src/noninteractive.c", sizeof("nvim src/noninteractive.c"), tree, &arena);
    ac_add("make bpv", sizeof("make bpv"), tree, &arena);
    ac_add("make thta", sizeof("make thta"), tree, &arena);
    ac_add("git restore *", sizeof("git restore *"), tree, &arena);
    ac_add("git restore tests/autocompletions_tests.c", sizeof("git restore tests/ac_tests.c"), tree, &arena);
    ac_add("git restore src/io/history.h", sizeof("git restore src/io/history.h"), tree, &arena);
    ac_add("nvim src/vm/vm.c", sizeof("nvim src/vm/vm.c"), tree, &arena);
    ac_add("nvim src/io/ncreadlin.c", sizeof("nvim src/io/ncreadlin.c"), tree, &arena);
    ac_add("nvim src/io/ncio.c", sizeof("nvim src/io/ncio.c"), tree, &arena);
    ac_add("nvim tests", sizeof("nvim tests"), tree, &arena);
    ac_add("make at", sizeof("make at"), tree, &arena);
    ac_add("nvim acceptance_tests/tests", sizeof("nvim acceptance_tests/tests"), tree, &arena);
    ac_add("nvim acceptance_tests/tests/common.rb", sizeof("nvim acceptance_tests/tests/common.rb"), tree, &arena);
    ac_add("ls", sizeof("ls"), tree, &arena);
    ac_add("ls", sizeof("ls"), tree, &arena);
    ac_add("ls -a", sizeof("ls -a"), tree, &arena);
    ac_add("nvim .", sizeof("nvim ."), tree, &arena);
    ac_add("nvim .", sizeof("nvim ."), tree, &arena);
    ac_add("nvim .", sizeof("nvim ."), tree, &arena);
    ac_add("git diff", sizeof("git diff"), tree, &arena);
    ac_add("nvim src/io/io.c", sizeof("nvim src/io/io.c"), tree, &arena);
    ac_add("nvim .", sizeof("nvim ."), tree, &arena);
    ac_add("z ncsh2", sizeof("z ncsh2"), tree, &arena);
    ac_add("nvim src/io/io.c", sizeof("nvim src/io/io.c"), tree, &arena);
    ac_add("nvim src", sizeof("nvim src"), tree, &arena);
    ac_add("git stash pop", sizeof("git stash pop"), tree, &arena);
    ac_add("nvim .", sizeof("nvim ."), tree, &arena);
    ac_add("make ud", sizeof("make ud"), tree, &arena);
    ac_add("./bin/ncsh", sizeof("./bin/ncsh"), tree, &arena);
    ac_add("git diff", sizeof("git diff"), tree, &arena);
    ac_add("nvim src/io", sizeof("nvim src/io"), tree, &arena);
    ac_add("git stash pop", sizeof("git stash pop"), tree, &arena);
    ac_add("nvim .", sizeof("nvim ."), tree, &arena);
    ac_add("make l", sizeof("make l"), tree, &arena);
    ac_add("ls -a --color", sizeof("ls -a --color"), tree, &arena);
    ac_add("version", sizeof("version"), tree, &arena);

    Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    uint8_t match_count = ac_get("l", autocomplete, tree, scratch_arena);
    match_count = ac_get("ls", autocomplete, tree, scratch_arena);
    match_count = ac_get("n", autocomplete, tree, scratch_arena);
    match_count = ac_get("nv", autocomplete, tree, scratch_arena);
    match_count = ac_get("nvi", autocomplete, tree, scratch_arena);
    match_count = ac_get("nvim", autocomplete, tree, scratch_arena);
    match_count = ac_get("v", autocomplete, tree, scratch_arena);
    match_count = ac_get("ve", autocomplete, tree, scratch_arena);
    match_count = ac_get("m", autocomplete, tree, scratch_arena);
    match_count = ac_get("ma", autocomplete, tree, scratch_arena);
    match_count = ac_get("mak", autocomplete, tree, scratch_arena);
    match_count = ac_get("make", autocomplete, tree, scratch_arena);
    match_count = ac_get("g", autocomplete, tree, scratch_arena);
    match_count = ac_get("gi", autocomplete, tree, scratch_arena);
    match_count = ac_get("git", autocomplete, tree, scratch_arena);
    match_count = ac_get("git ", autocomplete, tree, scratch_arena);
    match_count = ac_get("git d", autocomplete, tree, scratch_arena);
    match_count = ac_get("git di", autocomplete, tree, scratch_arena);
    match_count = ac_get("git dif", autocomplete, tree, scratch_arena);
    match_count = ac_get("git diff", autocomplete, tree, scratch_arena);
    eassert(match_count);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    ac_bench();
}
