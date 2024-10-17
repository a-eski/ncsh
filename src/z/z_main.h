#ifndef z_main_h
#define z_main_h

#include <stdint.h>

uint_fast32_t z_start (void);

struct eskilib_String* z_process (const struct eskilib_String* target);

uint_fast32_t z_finish (void);

#endif // !z_main_h

