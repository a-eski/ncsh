#include <stdint.h>
#include <stdio.h>

constexpr size_t NCSH_LETTERS = 96;
// #define NCSH_LETTERS 96 // ascii printable characters 32-127

typedef struct Autocompletion_Node_ {
    uint16_t weight;
    bool is_end_of_a_word;
    struct Autocompletion_Node_* nodes[NCSH_LETTERS];
} Autocompletion_Node;

static inline int char_to_index(char character)
{
    return (int)character - ' ';
}

Autocompletion_Node* ac_find(char* restrict p, Autocompletion_Node* restrict t)
{
    while (p) {
        if (!t)
            return NULL;

        t = t->nodes[char_to_index(*p)];
        ++p;
    }

    return t;
}

int main()
{
    printf("struct size: %zu\n", sizeof(Autocompletion_Node));

    printf("is_end_of_word size: %zu\n", sizeof(bool));
    printf("weight size: %zu\n", sizeof(uint16_t));
    printf("nodes one node size: %zu\n", sizeof(Autocompletion_Node));
    printf("nodes total size: %zu\n", sizeof(Autocompletion_Node) * NCSH_LETTERS);
}
