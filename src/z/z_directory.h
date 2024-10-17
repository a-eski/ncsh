#ifndef z_directory_h
#define z_directory_h

#include <time.h>

#include "../eskilib/eskilib_string.h"

#define Z_SECOND 1
#define Z_MINUTE 60 * Z_SECOND
#define Z_HOUR 60 * Z_MINUTE
#define Z_DAY 24 * Z_HOUR
#define Z_WEEK 7 * Z_DAY
#define Z_MONTH 30 * Z_DAY

typedef double rank;
typedef clock_t epoch;

struct z_Directory {
	struct eskilib_String path;
	rank rank;
	epoch last_accessed;
};

rank z_score(struct z_Directory* directory, epoch now);

#endif // !z_directory_h
