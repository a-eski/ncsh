void debug_stmts(Statements* restrict stmts)
{
    printf("stmts->count %zu\n", stmts->count);
    printf("stmts->cap %zu\n", stmts->cap);
    printf("stmts->type %u\n", stmts->type);

    for (size_t i = 0; i < stmts->count; ++i) {
        printf("stmts->statements[%zu].type %d\n", i, stmts->statements[i].type);
        printf("stmts->statements[%zu].count %zu\n", i, stmts->statements[i].count);
        printf("stmts->statements[%zu].commands->strs[0].value %s\n", i, stmts->statements[i].commands->strs[0].value);
    }
}
