#include <time.h>
#include <stdint.h>

#include "../eskilib/eskilib_string.h"
#include "z.h"

double z_score(struct z_Directory* directory, time_t now);

struct z_Directory* z_find_match(char* target, size_t target_length, const char* cwd, size_t cwd_length, struct z_Database* db);

enum z_Result z_add_new_to_database(char* path, size_t path_length, const char* cwd, size_t cwd_length, struct z_Database* db);

void z_free(struct z_Database* db);

