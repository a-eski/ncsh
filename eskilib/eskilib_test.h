#ifndef eskilib_test_h
#define eskilib_test_h

#include <stdio.h>
#include <stdbool.h>

#define eskilib_assert(condition) if (!(condition)) { printf("%s condition failed on line %d\n", __func__, __LINE__); eskilib_test_failed(); }

extern void eskilib_test_run(const char* function_name, void (*function)(void));

//do not need to use this manually, used with eskilib_assert to mark test failed when assertion does not pass.
extern void eskilib_test_failed();

#endif /* !eskilib_test_h */

