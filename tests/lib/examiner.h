/* for license see examiner_LICENSE.md */

#ifndef EXAMINER_H
#define EXAMINER_H

#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    void (*fn)();
    const char* scope;
    const char* name;
    bool pending;
} exam_test_t;

typedef struct {
    exam_test_t* tests;
    size_t len;
    size_t cap;
    const char* name;
    void (*before)();
    void (*after)();
} exam_scope_t;

typedef struct {
    exam_scope_t* scope;
    size_t len;
    size_t cap;
} exam_test_table_t;

typedef struct {
    const char** data;
    size_t len;
    size_t cap;
} exam_filter_t;

typedef struct {
    exam_test_table_t tbl;
    exam_filter_t filter;
    const char* (*color)(int32_t);
    int32_t longest_name_len;
    int32_t repeat;
    bool list;
    bool shuffle;
    bool shortd;
    bool die_on_fail;
} exam_env_t;

void exam_init(int32_t argc, char** argv);
int32_t exam_run();
void _exam_register_test(const char* scope, const char* name, void (*fn)(), bool pending);
void _exam_register_each(const char* scope, void (*fn)(), bool before);

#define __REGISTER_TEST(SCOPE, NAME, PENDING)                                                                          \
    void examtest_##SCOPE##_##NAME();                                                                                  \
    void examtest_##SCOPE##_##NAME##_register() __attribute__((constructor));                                          \
    void examtest_##SCOPE##_##NAME##_register()                                                                        \
    {                                                                                                                  \
        _exam_register_test(#SCOPE, #NAME, &examtest_##SCOPE##_##NAME, PENDING);                                       \
    }                                                                                                                  \
    void examtest_##SCOPE##_##NAME()

#define TEST(SCOPE, NAME) __REGISTER_TEST(SCOPE, NAME, false)
#define PENDING(SCOPE, NAME) __REGISTER_TEST(SCOPE, NAME, true)

#define BEFORE_EACH(SCOPE)                                                                                             \
    void examtest_before_each_##SCOPE();                                                                               \
    void examtest_before_each_##SCOPE##_register() __attribute__((constructor));                                       \
    void examtest_before_each_##SCOPE##_register()                                                                     \
    {                                                                                                                  \
        _exam_register_each(#SCOPE, &examtest_before_each_##SCOPE, true);                                              \
    }                                                                                                                  \
    void examtest_before_each_##SCOPE()
#define AFTER_EACH(SCOPE)                                                                                              \
    void examtest_after_each_##SCOPE();                                                                                \
    void examtest_after_each_##SCOPE##_register() __attribute__((constructor));                                        \
    void examtest_after_each_##SCOPE##_register()                                                                      \
    {                                                                                                                  \
        _exam_register_each(#SCOPE, &examtest_after_each_##SCOPE, false);                                              \
    }                                                                                                                  \
    void examtest_after_each_##SCOPE()

/////////////
// ASSERTS //
/////////////
void _exam_assert_true(bool value, const char* file, int32_t line);
void _exam_assert_false(bool value, const char* file, int32_t line);

#define ASSERT_TRUE(x) _exam_assert_true(x, __FILE__, __LINE__);
#define ASSERT_FALSE(x) _exam_assert_false(x, __FILE__, __LINE__);

void _exam_assert_equal_double(double expected, double result, const char* file, int32_t line);
void _exam_assert_equal_float(float expected, float result, const char* file, int32_t line);
void _exam_assert_equal_int(int32_t expected, int32_t result, const char* file, int32_t line);
void _exam_assert_equal_uint(uint32_t expected, uint32_t result, const char* file, int32_t line);
void _exam_assert_equal_long(long int expected, long int result, const char* file, int32_t line);
void _exam_assert_equal_ulong(unsigned long int expected, unsigned long int result, const char* file, int32_t line);
void _exam_assert_equal_long_long(long long expected, long long result, const char* file, int32_t line);
void _exam_assert_equal_ulong_long(unsigned long long expected, unsigned long long result, const char* file,
                                   int32_t line);
void _exam_assert_equal_char(char expected, char result, const char* file, int32_t line);
void _exam_assert_equal_str(const char* expected, const char* result, const char* file, int32_t line);
void _exam_assert_equal_ptr(void* expected, void* result, const char* file, int32_t line);
void _exam_assert_equal_mem(void* expected, void* result, size_t len, const char* file, int32_t line);

void _exam_assert_not_equal_double(double expected, double result, const char* file, int32_t line);
void _exam_assert_not_equal_float(float expected, float result, const char* file, int32_t line);
void _exam_assert_not_equal_int(int32_t expected, int32_t result, const char* file, int32_t line);
void _exam_assert_not_equal_uint(uint32_t expected, uint32_t result, const char* file, int32_t line);
void _exam_assert_not_equal_long(long int expected, long int result, const char* file, int32_t line);
void _exam_assert_not_equal_ulong(unsigned long int expected, unsigned long int result, const char* file, int32_t line);
void _exam_assert_not_equal_long_long(long long expected, long long result, const char* file, int32_t line);
void _exam_assert_not_equal_ulong_long(unsigned long long expected, unsigned long long result, const char* file,
                                       int32_t line);
void _exam_assert_not_equal_char(char expected, char result, const char* file, int32_t line);
void _exam_assert_not_equal_str(const char* expected, const char* result, const char* file, int32_t line);
void _exam_assert_not_equal_ptr(void* expected, void* result, const char* file, int32_t line);
void _exam_assert_not_equal_mem(void* expected, void* result, size_t len, const char* file, int32_t line);

#define __ISTYPE(x, t) __builtin_types_compatible_p(typeof(x), t)

// clang-format off
#define ASSERT_EQ(expected, result)                                            \
  (                                                                            \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, double), _exam_assert_equal_double,                   \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, float), _exam_assert_equal_float,                     \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, int32_t), _exam_assert_equal_int,                     \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, uint32_t), _exam_assert_equal_uint,                   \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, long int), _exam_assert_equal_long,                   \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, unsigned long int), _exam_assert_equal_ulong,         \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, long long), _exam_assert_equal_long_long,             \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, unsigned long long), _exam_assert_equal_ulong_long,   \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, char), _exam_assert_equal_char,                       \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, char[]), _exam_assert_equal_str,                      \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, char*), _exam_assert_equal_str,                       \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, void*), _exam_assert_equal_ptr,                       \
    (void)0))))))))))))                                                        \
  (expected, result, __FILE__, __LINE__))

#define ASSERT_NE(expected, result)                                            \
  (                                                                            \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, double), _exam_assert_not_equal_double,               \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, float), _exam_assert_not_equal_float,                 \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, int32_t), _exam_assert_not_equal_int,                 \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, uint32_t), _exam_assert_not_equal_uint,               \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, long int), _exam_assert_not_equal_long,               \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, unsigned long int), _exam_assert_not_equal_ulong,     \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, long long), _exam_assert_not_equal_long_long,         \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, unsigned long long),                                  \
               _exam_assert_not_equal_ulong_long,                              \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, char), _exam_assert_not_equal_char,                   \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, char[]), _exam_assert_not_equal_str,                  \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, char*), _exam_assert_not_equal_str,                   \
    __builtin_choose_expr(                                                     \
      __ISTYPE(expected, void*), _exam_assert_not_equal_ptr,                   \
    (void)0))))))))))))                                                        \
  (expected, result, __FILE__, __LINE__))
// clang-format on

#define ASSERT_EQ_MEM(expected, result, len) _exam_assert_equal_mem(expected, result, len, __FILE__, __LINE__)

#define ASSERT_NE_MEM(expected, result, len) _exam_assert_not_equal_mem(expected, result, len, __FILE__, __LINE__)

#endif // EXAMINER_H
