#ifndef z_main_h
#define z_main_h

#include <stdint.h>

#include "z_database.h"

enum z_Database_Result z_start (struct z_Database* database);

struct eskilib_String z_process (const struct eskilib_String target, struct z_Database* database);

enum z_Database_Result z_finish (struct z_Database* database);

#endif // !z_main_h

