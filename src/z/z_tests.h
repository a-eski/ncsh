#include <time.h>
#include <stdint.h>

#include "../eskilib/eskilib_string.h"
#include "z.h"

double z_score(struct z_Directory* directory, time_t now);

struct z_Directory* find_match(char* target, size_t target_length, uint32_t number_of_entries, struct z_Directory* dirs);

enum z_Result write_to_database_file(uint32_t number_of_entries, struct z_Directory* dirs);

enum z_Result add_to_database(char* path, size_t path_length, struct z_Database* db);
