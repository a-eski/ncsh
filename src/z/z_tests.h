#include "time.h"
#include "../eskilib/eskilib_string.h"
#include <stdint.h>

double z_score(struct z_Directory* directory, time_t now);

struct z_Directory* find_match(struct eskilib_String target, uint32_t number_of_entries, struct z_Directory* dirs);

void write_to_database_file(uint32_t number_of_entries, struct z_Directory* dirs);

uint32_t read_from_database_file(struct z_Directory* dirs);

uint32_t add_to_database(struct eskilib_String path, uint32_t number_of_entries, struct z_Directory* dirs);
