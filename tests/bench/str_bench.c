#include "../lib/arena_test_helper.h"
#include "../../src/eskilib/str.h"

// Compared these 2 implementations
// First was generally faster
/*enodiscard static inline Str* estrdup(Str* v, Arena* restrict a)
{
    Str* rv = arena_malloc(a, 1, Str);
    rv->value = arena_malloc(a, v->length, char);
    memcpy(rv->value, v->value, v->length - 1);
    rv->length = v->length;

    assert(rv->value[rv->length - 1] == '\0');

    return rv;
}

enodiscard static inline Str* estrdup(Str* v, Arena* restrict a)
{
    Str* rv = arena_malloc(a, 1, Str);
    rv->value = arena_malloc(a, v->length, char);
    for (size_t i = 0; i < v->length - 1; ++i) {
        rv->value[i] = v->value[i];
    }
    rv->length = v->length;

    assert(rv->value[rv->length - 1] == '\0');

    return rv;
}*/


void estrdup_bench()
{
    ARENA_TEST_SETUP;

    auto data = Str_New_Literal("string to be duplicated");

    [[maybe_unused]] auto result = estrdup(&data, &a);

    ARENA_TEST_TEARDOWN;
}

// Compared these 2 implementations
// First was generally faster
/*enodiscard static inline size_t estridx(Str* v, char c)
{
    assert(v); assert(v->value);

    size_t idx;
    for (idx = 0; idx < v->length; ++idx) {
        if (v->value[idx] == c)
            break;
    }
    return idx;
}

enodiscard static inline size_t estridx(Str* v, char c)
{
    assert(v); assert(v->value);
    static char terminated_c[2] = {0};
    terminated_c[0] = c;

    return strcspn(v->value, terminated_c);
}*/
void estridx_bench()
{
    auto data = Str_New_Literal("string=toBeSplit");

    [[maybe_unused]] auto result = estridx(&data, '=');
}

// 483.5 - 488.6
// 481.2 - 485.5
void estrtrim_bench()
{
    auto data = Str_New_Literal("stringtoBeTrimmed  ");

    estrtrim(&data);
}

int main()
{
    // estrdup_bench();
    // estridx_bench();
    estrtrim_bench();
}
