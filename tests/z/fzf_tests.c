/* For license see fzf_LICENSE.*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

#include "../../src/z/fzf.h"
#include "../lib/arena_test_helper.h"
#include "../lib/examiner.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ScoreMatch = 16,
    ScoreGapStart = -3,
    ScoreGapExtension = -1,
    BonusBoundary = ScoreMatch / 2,
    BonusNonWord = ScoreMatch / 2,
    BonusCamel123 = BonusBoundary + ScoreGapExtension,
    BonusConsecutive = -(ScoreGapStart + ScoreGapExtension),
    BonusFirstCharMultiplier = 2,
} score_t;

fzf_position_t* fzf_pos_array(size_t len, Arena* scratch_arena);
fzf_position_t* fzf_get_positions(const char* text, fzf_pattern_t* pattern, fzf_slab_t* slab, Arena* scratch_arena);

#define call_alg(alg, case, txt, pat, assert_block)                                                                    \
    SCRATCH_ARENA_TEST_SETUP;                                                                                          \
    {                                                                                                                  \
        fzf_position_t* pos = fzf_pos_array(0, &scratch_arena);                                                        \
        fzf_result_t res = alg(case, txt, pat, pos, NULL, &scratch_arena);                                             \
        assert_block;                                                                                                  \
    }                                                                                                                  \
    {                                                                                                                  \
        fzf_position_t* pos = fzf_pos_array(0, &scratch_arena);                                                        \
        fzf_slab_t* slab = fzf_make_default_slab(&scratch_arena);                                                      \
        fzf_result_t res = alg(case, txt, pat, pos, slab, &scratch_arena);                                             \
        assert_block;                                                                                                  \
    }                                                                                                                  \
    SCRATCH_ARENA_TEST_TEARDOWN;

static int8_t max_i8(int8_t a, int8_t b)
{
    return a > b ? a : b;
}

#define MATCH_WRAPPER(nn, og)                                                                                          \
    fzf_result_t nn(bool case_sensitive, const char* text, const char* pattern, fzf_position_t* pos, fzf_slab_t* slab, \
                    Arena* scratch_arena)                                                                              \
    {                                                                                                                  \
        fzf_string_t input = {.data = text, .size = strlen(text)};                                                     \
        fzf_string_t pattern_wrap = {.data = pattern, .size = strlen(pattern)};                                        \
        return og(case_sensitive, &input, &pattern_wrap, pos, slab, scratch_arena);                                    \
    }

MATCH_WRAPPER(fuzzy_match_v2, fzf_fuzzy_match_v2);
MATCH_WRAPPER(fuzzy_match_v1, fzf_fuzzy_match_v1);
MATCH_WRAPPER(exact_match_naive, fzf_exact_match_naive);
MATCH_WRAPPER(prefix_match, fzf_prefix_match);
MATCH_WRAPPER(suffix_match, fzf_suffix_match);
MATCH_WRAPPER(equal_match, fzf_equal_match);

// TODO(conni2461): Implement normalize and test it here
TEST(FuzzyMatchV2, case1)
{
    call_alg(fuzzy_match_v2, true, "So Danco Samba", "So", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(2, res.end);
        ASSERT_EQ(56, res.score);

        ASSERT_EQ(2, pos->size);
        ASSERT_EQ(1, pos->data[0]);
        ASSERT_EQ(0, pos->data[1]);
    });
}

TEST(FuzzyMatchV2, case2)
{
    call_alg(fuzzy_match_v2, false, "So Danco Samba", "sodc", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(7, res.end);
        ASSERT_EQ(89, res.score);

        ASSERT_EQ(4, pos->size);
        ASSERT_EQ(6, pos->data[0]);
        ASSERT_EQ(3, pos->data[1]);
        ASSERT_EQ(1, pos->data[2]);
        ASSERT_EQ(0, pos->data[3]);
    });
}

TEST(FuzzyMatchV2, case3)
{
    call_alg(fuzzy_match_v2, false, "Danco", "danco", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(5, res.end);
        ASSERT_EQ(128, res.score);

        ASSERT_EQ(5, pos->size);
        ASSERT_EQ(4, pos->data[0]);
        ASSERT_EQ(3, pos->data[1]);
        ASSERT_EQ(2, pos->data[2]);
        ASSERT_EQ(1, pos->data[3]);
        ASSERT_EQ(0, pos->data[4]);
    });
}

TEST(FuzzyMatchV2, case4)
{
    call_alg(fuzzy_match_v2, false, "fooBarbaz1", "obz", {
        ASSERT_EQ(2, res.start);
        ASSERT_EQ(9, res.end);
        int expected_score = ScoreMatch * 3 + BonusCamel123 + ScoreGapStart + ScoreGapExtension * 3;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case5)
{
    call_alg(fuzzy_match_v2, false, "foo bar baz", "fbb", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(9, res.end);
        int expected_score = ScoreMatch * 3 + BonusBoundary * BonusFirstCharMultiplier + BonusBoundary * 2 +
                             2 * ScoreGapStart + 4 * ScoreGapExtension;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case6)
{
    call_alg(fuzzy_match_v2, false, "/AutomatorDocument.icns", "rdoc", {
        ASSERT_EQ(9, res.start);
        ASSERT_EQ(13, res.end);
        int expected_score = ScoreMatch * 4 + BonusCamel123 + BonusConsecutive * 2;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case7)
{
    call_alg(fuzzy_match_v2, false, "/man1/zshcompctl.1", "zshc", {
        ASSERT_EQ(6, res.start);
        ASSERT_EQ(10, res.end);
        int expected_score = ScoreMatch * 4 + BonusBoundary * BonusFirstCharMultiplier + BonusBoundary * 3;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case8)
{
    call_alg(fuzzy_match_v2, false, "/.oh-my-zsh/cache", "zshc", {
        ASSERT_EQ(8, res.start);
        ASSERT_EQ(13, res.end);
        int expected_score =
            ScoreMatch * 4 + BonusBoundary * BonusFirstCharMultiplier + BonusBoundary * 3 + ScoreGapStart;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case9)
{
    call_alg(fuzzy_match_v2, false, "ab0123 456", "12356", {
        ASSERT_EQ(3, res.start);
        ASSERT_EQ(10, res.end);
        int expected_score = ScoreMatch * 5 + BonusConsecutive * 3 + ScoreGapStart + ScoreGapExtension;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case10)
{
    call_alg(fuzzy_match_v2, false, "abc123 456", "12356", {
        ASSERT_EQ(3, res.start);
        ASSERT_EQ(10, res.end);
        int expected_score = ScoreMatch * 5 + BonusCamel123 * BonusFirstCharMultiplier + BonusCamel123 * 2 +
                             BonusConsecutive + ScoreGapStart + ScoreGapExtension;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case11)
{
    call_alg(fuzzy_match_v2, false, "foo/bar/baz", "fbb", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(9, res.end);
        int expected_score = ScoreMatch * 3 + BonusBoundary * BonusFirstCharMultiplier + BonusBoundary * 2 +
                             2 * ScoreGapStart + 4 * ScoreGapExtension;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case12)
{
    call_alg(fuzzy_match_v2, false, "fooBarBaz", "fbb", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(7, res.end);
        int expected_score = ScoreMatch * 3 + BonusBoundary * BonusFirstCharMultiplier + BonusCamel123 * 2 +
                             2 * ScoreGapStart + 2 * ScoreGapExtension;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case13)
{
    call_alg(fuzzy_match_v2, false, "foo barbaz", "fbb", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(8, res.end);
        int expected_score = ScoreMatch * 3 + BonusBoundary * BonusFirstCharMultiplier + BonusBoundary +
                             ScoreGapStart * 2 + ScoreGapExtension * 3;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case14)
{
    call_alg(fuzzy_match_v2, false, "fooBar Baz", "foob", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(4, res.end);
        int expected_score = ScoreMatch * 4 + BonusBoundary * BonusFirstCharMultiplier + BonusBoundary * 3;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case15)
{
    call_alg(fuzzy_match_v2, false, "xFoo-Bar Baz", "foo-b", {
        ASSERT_EQ(1, res.start);
        ASSERT_EQ(6, res.end);
        int expected_score = ScoreMatch * 5 + BonusCamel123 * BonusFirstCharMultiplier + BonusCamel123 * 2 +
                             BonusNonWord + BonusBoundary;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case16)
{
    call_alg(fuzzy_match_v2, true, "fooBarbaz", "oBz", {
        ASSERT_EQ(2, res.start);
        ASSERT_EQ(9, res.end);
        int expected_score = ScoreMatch * 3 + BonusCamel123 + ScoreGapStart + ScoreGapExtension * 3;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case17)
{
    call_alg(fuzzy_match_v2, true, "Foo/Bar/Baz", "FBB", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(9, res.end);
        int expected_score =
            ScoreMatch * 3 + BonusBoundary * (BonusFirstCharMultiplier + 2) + ScoreGapStart * 2 + ScoreGapExtension * 4;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case18)
{
    call_alg(fuzzy_match_v2, true, "FooBarBaz", "FBB", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(7, res.end);
        int expected_score = ScoreMatch * 3 + BonusBoundary * BonusFirstCharMultiplier + BonusCamel123 * 2 +
                             ScoreGapStart * 2 + ScoreGapExtension * 2;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case19)
{
    call_alg(fuzzy_match_v2, true, "FooBar Baz", "FooB", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(4, res.end);
        int expected_score = ScoreMatch * 4 + BonusBoundary * BonusFirstCharMultiplier + BonusBoundary * 2 +
                             max_i8(BonusCamel123, BonusBoundary);
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case20)
{
    call_alg(fuzzy_match_v2, true, "foo-bar", "o-ba", {
        ASSERT_EQ(2, res.start);
        ASSERT_EQ(6, res.end);
        int expected_score = ScoreMatch * 4 + BonusBoundary * 3;
        ASSERT_EQ(expected_score, res.score);
    });
}

TEST(FuzzyMatchV2, case21)
{
    call_alg(fuzzy_match_v2, true, "fooBarbaz", "oBZ", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(FuzzyMatchV2, case22)
{
    call_alg(fuzzy_match_v2, true, "Foo Bar Baz", "fbb", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(FuzzyMatchV2, case23)
{
    call_alg(fuzzy_match_v2, true, "fooBarbaz", "fooBarbazz", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(FuzzyMatchV1, case1)
{
    call_alg(fuzzy_match_v1, true, "So Danco Samba", "So", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(2, res.end);
        ASSERT_EQ(56, res.score);

        ASSERT_EQ(2, pos->size);
        ASSERT_EQ(0, pos->data[0]);
        ASSERT_EQ(1, pos->data[1]);
    });
}

TEST(FuzzyMatchV1, case2)
{
    call_alg(fuzzy_match_v1, false, "So Danco Samba", "sodc", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(7, res.end);
        ASSERT_EQ(89, res.score);

        ASSERT_EQ(4, pos->size);
        ASSERT_EQ(0, pos->data[0]);
        ASSERT_EQ(1, pos->data[1]);
        ASSERT_EQ(3, pos->data[2]);
        ASSERT_EQ(6, pos->data[3]);
    });
}

TEST(FuzzyMatchV1, case3)
{
    call_alg(fuzzy_match_v1, false, "Danco", "danco", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(5, res.end);
        ASSERT_EQ(128, res.score);

        ASSERT_EQ(5, pos->size);
        ASSERT_EQ(0, pos->data[0]);
        ASSERT_EQ(1, pos->data[1]);
        ASSERT_EQ(2, pos->data[2]);
        ASSERT_EQ(3, pos->data[3]);
        ASSERT_EQ(4, pos->data[4]);
    });
}

TEST(ExactMatch, case1)
{
    call_alg(exact_match_naive, true, "So Danco Samba", "So", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(2, res.end);
        ASSERT_EQ(56, res.score);
    });
}

TEST(ExactMatch, case2)
{
    call_alg(exact_match_naive, false, "So Danco Samba", "sodc", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(ExactMatch, case3)
{
    call_alg(exact_match_naive, false, "Danco", "danco", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(5, res.end);
        ASSERT_EQ(128, res.score);
    });
}

TEST(PrefixMatch, case1)
{
    call_alg(prefix_match, true, "So Danco Samba", "So", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(2, res.end);
        ASSERT_EQ(56, res.score);
    });
}

TEST(PrefixMatch, case2)
{
    call_alg(prefix_match, false, "So Danco Samba", "sodc", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(PrefixMatch, case3)
{
    call_alg(prefix_match, false, "Danco", "danco", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(5, res.end);
        ASSERT_EQ(128, res.score);
    });
}

TEST(SuffixMatch, case1)
{
    call_alg(suffix_match, true, "So Danco Samba", "So", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(SuffixMatch, case2)
{
    call_alg(suffix_match, false, "So Danco Samba", "sodc", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(SuffixMatch, case3)
{
    call_alg(suffix_match, false, "Danco", "danco", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(5, res.end);
        ASSERT_EQ(128, res.score);
    });
}

TEST(EqualMatch, case1)
{
    call_alg(equal_match, true, "So Danco Samba", "So", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(EqualMatch, case2)
{
    call_alg(equal_match, false, "So Danco Samba", "sodc", {
        ASSERT_EQ(-1, res.start);
        ASSERT_EQ(-1, res.end);
        ASSERT_EQ(0, res.score);
    });
}

TEST(EqualMatch, case3)
{
    call_alg(equal_match, false, "Danco", "danco", {
        ASSERT_EQ(0, res.start);
        ASSERT_EQ(5, res.end);
        ASSERT_EQ(128, res.score);
    });
}

TEST(PatternParsing, empty)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat = fzf_parse_pattern("", strlen(""), &scratch_arena);
    ASSERT_EQ(0, pat->size);
    ASSERT_EQ(0, pat->cap);
    ASSERT_FALSE(pat->only_inv);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, simple)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat = fzf_parse_pattern("lua", strlen("lua"), &scratch_arena);
    ASSERT_EQ(1, pat->size);
    ASSERT_EQ(1, pat->cap);
    ASSERT_FALSE(pat->only_inv);

    ASSERT_EQ(1, pat->ptr[0]->size);
    ASSERT_EQ(1, pat->ptr[0]->cap);

    ASSERT_EQ((void*)fzf_fuzzy_match_v2, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ("lua", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[0]->ptr[0].case_sensitive);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, withEscapedSpace)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat = fzf_parse_pattern("file\\ ", strlen("file\\ "), &scratch_arena);
    ASSERT_EQ(1, pat->size);
    ASSERT_EQ(1, pat->cap);
    ASSERT_FALSE(pat->only_inv);

    ASSERT_EQ(1, pat->ptr[0]->size);
    ASSERT_EQ(1, pat->ptr[0]->cap);

    ASSERT_EQ((void*)fzf_fuzzy_match_v2, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ("file ", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[0]->ptr[0].case_sensitive);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, withComplexEscapedSpace)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat = fzf_parse_pattern("file\\ with\\ space", strlen("file\\ with\\ space"), &scratch_arena);
    ASSERT_EQ(1, pat->size);
    ASSERT_EQ(1, pat->cap);
    ASSERT_FALSE(pat->only_inv);

    ASSERT_EQ(1, pat->ptr[0]->size);
    ASSERT_EQ(1, pat->ptr[0]->cap);

    ASSERT_EQ((void*)fzf_fuzzy_match_v2, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ("file with space", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[0]->ptr[0].case_sensitive);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, withEscapedSpaceAndNormalSpace)
{
    SCRATCH_ARENA_TEST_SETUP;
    const char str[] = "file\\  new";
    fzf_pattern_t* pat = fzf_parse_pattern((char*)str, sizeof(str) - 1, &scratch_arena);
    ASSERT_EQ(2, pat->size);
    ASSERT_EQ(2, pat->cap);
    ASSERT_FALSE(pat->only_inv);

    ASSERT_EQ(1, pat->ptr[0]->size);
    ASSERT_EQ(1, pat->ptr[0]->cap);
    ASSERT_EQ(1, pat->ptr[1]->size);
    ASSERT_EQ(1, pat->ptr[1]->cap);

    ASSERT_EQ((void*)fzf_fuzzy_match_v2, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ("file ", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[0]->ptr[0].case_sensitive);

    ASSERT_EQ((void*)fzf_fuzzy_match_v2, pat->ptr[1]->ptr[0].fn);
    ASSERT_EQ("new", ((fzf_string_t*)(pat->ptr[1]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[1]->ptr[0].case_sensitive);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, invert)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat = fzf_parse_pattern("!Lua", strlen("!Lua"), &scratch_arena);
    ASSERT_EQ(1, pat->size);
    ASSERT_EQ(1, pat->cap);
    ASSERT_TRUE(pat->only_inv);

    ASSERT_EQ(1, pat->ptr[0]->size);
    ASSERT_EQ(1, pat->ptr[0]->cap);

    ASSERT_EQ((void*)fzf_exact_match_naive, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ("Lua", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_TRUE(pat->ptr[0]->ptr[0].case_sensitive);
    ASSERT_TRUE(pat->ptr[0]->ptr[0].inv);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, invertMultiple)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat = fzf_parse_pattern("!fzf !test", strlen("!fzf !test"), &scratch_arena);
    ASSERT_EQ(2, pat->size);
    ASSERT_EQ(2, pat->cap);
    ASSERT_TRUE(pat->only_inv);

    ASSERT_EQ(1, pat->ptr[0]->size);
    ASSERT_EQ(1, pat->ptr[0]->cap);
    ASSERT_EQ(1, pat->ptr[1]->size);
    ASSERT_EQ(1, pat->ptr[1]->cap);

    ASSERT_EQ((void*)fzf_exact_match_naive, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ("fzf", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[0]->ptr[0].case_sensitive);
    ASSERT_TRUE(pat->ptr[0]->ptr[0].inv);

    ASSERT_EQ((void*)fzf_exact_match_naive, pat->ptr[1]->ptr[0].fn);
    ASSERT_EQ("test", ((fzf_string_t*)(pat->ptr[1]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[1]->ptr[0].case_sensitive);
    ASSERT_TRUE(pat->ptr[1]->ptr[0].inv);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, smartCase)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat = fzf_parse_pattern("Lua", strlen("Lua"), &scratch_arena);
    ASSERT_EQ(1, pat->size);
    ASSERT_EQ(1, pat->cap);
    ASSERT_FALSE(pat->only_inv);

    ASSERT_EQ(1, pat->ptr[0]->size);
    ASSERT_EQ(1, pat->ptr[0]->cap);

    ASSERT_EQ((void*)fzf_fuzzy_match_v2, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ("Lua", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_TRUE(pat->ptr[0]->ptr[0].case_sensitive);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, simpleOr)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat = fzf_parse_pattern("'src | ^Lua", strlen("'src | ^Lua"), &scratch_arena);
    ASSERT_EQ(1, pat->size);
    ASSERT_EQ(1, pat->cap);
    ASSERT_FALSE(pat->only_inv);

    ASSERT_EQ(2, pat->ptr[0]->size);
    ASSERT_EQ(2, pat->ptr[0]->cap);

    ASSERT_EQ((void*)fzf_exact_match_naive, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ("src", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[0]->ptr[0].case_sensitive);

    ASSERT_EQ((void*)fzf_prefix_match, pat->ptr[0]->ptr[1].fn);
    ASSERT_EQ("Lua", ((fzf_string_t*)(pat->ptr[0]->ptr[1].text))->data);
    ASSERT_TRUE(pat->ptr[0]->ptr[1].case_sensitive);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PatternParsing, complexAnd)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_pattern_t* pat =
        fzf_parse_pattern(".lua$ 'previewer !'term !asdf", strlen(".lua$ 'previewer !'term !asdf"), &scratch_arena);
    ASSERT_EQ(4, pat->size);
    ASSERT_EQ(4, pat->cap);
    ASSERT_FALSE(pat->only_inv);

    ASSERT_EQ(1, pat->ptr[0]->size);
    ASSERT_EQ(1, pat->ptr[0]->cap);
    ASSERT_EQ(1, pat->ptr[1]->size);
    ASSERT_EQ(1, pat->ptr[1]->cap);
    ASSERT_EQ(1, pat->ptr[2]->size);
    ASSERT_EQ(1, pat->ptr[2]->cap);
    ASSERT_EQ(1, pat->ptr[3]->size);
    ASSERT_EQ(1, pat->ptr[3]->cap);

    ASSERT_EQ((void*)fzf_suffix_match, pat->ptr[0]->ptr[0].fn);
    ASSERT_EQ(".lua", ((fzf_string_t*)(pat->ptr[0]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[0]->ptr[0].case_sensitive);

    ASSERT_EQ((void*)fzf_exact_match_naive, pat->ptr[1]->ptr[0].fn);
    ASSERT_EQ("previewer", ((fzf_string_t*)(pat->ptr[1]->ptr[0].text))->data);
    ASSERT_EQ(0, pat->ptr[1]->ptr[0].case_sensitive);

    ASSERT_EQ((void*)fzf_fuzzy_match_v2, pat->ptr[2]->ptr[0].fn);
    ASSERT_EQ("term", ((fzf_string_t*)(pat->ptr[2]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[2]->ptr[0].case_sensitive);
    ASSERT_TRUE(pat->ptr[2]->ptr[0].inv);

    ASSERT_EQ((void*)fzf_exact_match_naive, pat->ptr[3]->ptr[0].fn);
    ASSERT_EQ("asdf", ((fzf_string_t*)(pat->ptr[3]->ptr[0].text))->data);
    ASSERT_FALSE(pat->ptr[3]->ptr[0].case_sensitive);
    ASSERT_TRUE(pat->ptr[3]->ptr[0].inv);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

static void score_wrapper(char* pattern, char** input, int* expected)
{

    SCRATCH_ARENA_TEST_SETUP;
    fzf_slab_t* slab = fzf_make_default_slab(&scratch_arena);
    fzf_pattern_t* pat = fzf_parse_pattern(pattern, strlen(pattern), &scratch_arena);
    for (size_t i = 0; input[i] != NULL; ++i) {
        ASSERT_EQ(expected[i], fzf_get_score(input[i], strlen(input[i]), pat, slab, &scratch_arena));
    }
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(ScoreIntegration, simple)
{
    char* input[] = {"fzf", "main.c", "src/fzf", "fz/noooo", NULL};
    int expected[] = {0, 1, 0, 1};
    score_wrapper("!fzf", input, expected);
}

TEST(ScoreIntegration, invertAnd)
{
    char* input[] = {"src/fzf.c", "README.md", "lua/asdf", "test/test.c", NULL};
    int expected[] = {0, 1, 1, 0};
    score_wrapper("!fzf !test", input, expected);
}

TEST(ScoreIntegration, withEscapedSpace)
{
    char* input[] = {"file ", "file lua", "lua", NULL};
    int expected[] = {0, 200, 0};
    score_wrapper("file\\ lua", input, expected);
}

TEST(ScoreIntegration, onlyEscapedSpace)
{
    char* input[] = {"file with space", "file lua", "lua", "src", "test", NULL};
    int expected[] = {32, 32, 0, 0, 0};
    score_wrapper("\\ ", input, expected);
}

TEST(ScoreIntegration, simpleOr)
{
    char* input[] = {"src/fzf.h", "README.md", "build/fzf", "lua/fzf_lib.lua", "Lua/fzf_lib.lua", NULL};
    int expected[] = {80, 0, 0, 0, 80};
    score_wrapper("'src | ^Lua", input, expected);
}

TEST(ScoreIntegration, complexTerm)
{
    char* input[] = {"lua/random_previewer",  "README.md",           "previewers/utils.lua",
                     "previewers/buffer.lua", "previewers/term.lua", NULL};
    int expected[] = {0, 0, 328, 328, 0};
    score_wrapper(".lua$ 'previewer !'term", input, expected);
}

static void pos_wrapper(char* pattern, char** input, int** expected)
{
    SCRATCH_ARENA_TEST_SETUP;
    fzf_slab_t* slab = fzf_make_default_slab(&scratch_arena);
    fzf_pattern_t* pat = fzf_parse_pattern(pattern, strlen(pattern), &scratch_arena);
    for (size_t i = 0; input[i] != NULL; ++i) {
        fzf_position_t* pos = fzf_get_positions(input[i], pat, slab, &scratch_arena);
        if (!pos) {
            ASSERT_EQ((void*)pos, expected[i]);
            continue;
        }

        // Verify that the size is correct
        if (expected[i]) {
            ASSERT_EQ(-1, expected[i][pos->size]);
        }
        else {
            ASSERT_EQ(0, pos->size);
        }
        ASSERT_EQ_MEM(expected[i], pos->data, pos->size * sizeof(pos->data[0]));
    }
    SCRATCH_ARENA_TEST_TEARDOWN;
}

TEST(PosIntegration, simple)
{
    char* input[] = {"src/fzf.c", "src/fzf.h", "lua/fzf_lib.lua", "lua/telescope/_extensions/fzf.lua",
                     "README.md", NULL};
    int match1[] = {6, 5, 4, -1};
    int match2[] = {6, 5, 4, -1};
    int match3[] = {6, 5, 4, -1};
    int match4[] = {28, 27, 26, -1};
    int* expected[] = {match1, match2, match3, match4, NULL};
    pos_wrapper("fzf", input, expected);
}

TEST(PosIntegration, invert)
{
    char* input[] = {"fzf", "main.c", "src/fzf", "fz/noooo", NULL};
    int* expected[] = {NULL, NULL, NULL, NULL, NULL};
    pos_wrapper("!fzf", input, expected);
}

TEST(PosIntegration, andWithSecondInvert)
{
    char* input[] = {"src/fzf.c", "lua/fzf_lib.lua", "build/libfzf", NULL};
    int match1[] = {6, 5, 4, -1};
    int* expected[] = {match1, NULL, NULL};
    pos_wrapper("fzf !lib", input, expected);
}

TEST(PosIntegration, andAllInvert)
{
    char* input[] = {"src/fzf.c", "README.md", "lua/asdf", "test/test.c", NULL};
    int* expected[] = {NULL, NULL, NULL, NULL};
    pos_wrapper("!fzf !test", input, expected);
}

TEST(PosIntegration, withEscapedSpace)
{
    char* input[] = {"file ", "file lua", "lua", NULL};
    int match1[] = {7, 6, 5, 4, 3, 2, 1, 0, -1};
    int* expected[] = {NULL, match1, NULL};
    pos_wrapper("file\\ lua", input, expected);
}

TEST(PosIntegration, onlyEscapedSpace)
{
    char* input[] = {"file with space", "lul lua", "lua", "src", "test", NULL};
    int match1[] = {4, -1};
    int match2[] = {3, -1};
    int* expected[] = {match1, match2, NULL, NULL, NULL};
    pos_wrapper("\\ ", input, expected);
}

TEST(PosIntegration, simpleOr)
{
    char* input[] = {"src/fzf.h", "README.md", "build/fzf", "lua/fzf_lib.lua", "Lua/fzf_lib.lua", NULL};
    int match1[] = {0, 1, 2, -1};
    int match2[] = {0, 1, 2, -1};
    int* expected[] = {match1, NULL, NULL, NULL, match2};
    pos_wrapper("'src | ^Lua", input, expected);
}

TEST(PosIntegration, orMemLeak)
{
    char* input[] = {"src/fzf.h", NULL};
    int match1[] = {2, 1, 0, -1};
    int* expected[] = {match1};
    pos_wrapper("src | src", input, expected);
}

TEST(PosIntegration, complexTerm)
{
    char* input[] = {"lua/random_previewer",  "README.md",           "previewers/utils.lua",
                     "previewers/buffer.lua", "previewers/term.lua", NULL};
    int match1[] = {16, 17, 18, 19, 0, 1, 2, 3, 4, 5, 6, 7, 8, -1};
    int match2[] = {17, 18, 19, 20, 0, 1, 2, 3, 4, 5, 6, 7, 8, -1};
    int* expected[] = {NULL, NULL, match1, match2, NULL};
    pos_wrapper(".lua$ 'previewer !'term", input, expected);
}

int main(int argc, char** argv)
{
    exam_init(argc, argv);
    return exam_run();
}

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
