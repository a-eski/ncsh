#include "../../src/io/ac.h"
#include "../../src/io/history.h"
#include "../../src/conf.h"
#include <stdio.h>
#include <stdlib.h>

void ac_export_dot(Autocompletion_Node* restrict tree, char* restrict file_path);

char* arena_init(Arena* arena, Arena* temp)
{
    constexpr int arena_capacity = 1 << 24;
    constexpr int scratch_arena_capacity = 1 << 20;
    constexpr int total_capacity = arena_capacity + scratch_arena_capacity;

    char* memory = malloc(total_capacity);
    if (!memory) {
        return NULL;
    }

    arena->start = memory;
    arena->end = memory + (arena_capacity);

    char* scratch_memory_start = memory + (arena_capacity + 1);
    temp->start = scratch_memory_start;
    temp->end = scratch_memory_start + (scratch_arena_capacity);
    return memory;
}

int main()
{
    Arena a = {};
    Arena ta = {};
    char* mem = arena_init(&a, &ta);
    Config conf = {};
    enum eresult r = config_init(&conf, &a, ta);
    if (r != E_SUCCESS) {
        perror("ncsh: Error when setting up configurations");
        return EXIT_FAILURE;
    }

    History history = {};
    r = history_init(conf.config_location, &history, &a);
    if (history_init(conf.config_location, &history, &a) != E_SUCCESS) {
        perror("ncsh: Error when setting up history");
        return EXIT_FAILURE;
    }

    Autocompletion_Node ac_tree = {};
    ac_add_multiple(history.entries, history.count, &ac_tree, &a);

    ac_export_dot(&ac_tree, "./trie");

    free(mem);
}
