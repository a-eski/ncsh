extern void alias_tests();
extern void arena_tests();
extern void config_tests();
extern void env_tests();
extern void str_tests();
extern void vm_next_tests();
extern void vm_tests();
extern void expansions_tests();
extern void lexer_tests();
extern void parser_tests();
extern void vars_tests();
extern void ac_tests();
extern void hashset_tests();
extern void history_tests();
extern void prompt_tests();
extern void z_tests();
extern int fzf_tests(int, char**);

int main(int argc, char** argv)
{
    alias_tests();
    arena_tests();
    config_tests();
    config_tests();
    env_tests();
    str_tests();
    vm_next_tests();
    vm_tests();
    ac_tests();
    hashset_tests();
    history_tests();
    prompt_tests();
    z_tests();
    fzf_tests(argc, argv);
}
