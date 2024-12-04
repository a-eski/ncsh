#include <time.h>
#include <stdint.h>

#include "../eskilib/eskilib_string.h"
#include "z.h"

double z_score(struct z_Directory* directory, time_t now);

struct z_Directory* find_match(char* target, size_t target_length, uint32_t number_of_entries, struct z_Directory* dirs);

enum z_Result add_new_to_database(char* path, size_t path_length, const char* cwd, struct z_Database* db);

void z_free(struct z_Database* db);

