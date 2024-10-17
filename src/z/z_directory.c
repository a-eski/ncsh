#include "z_directory.h"

rank z_score(struct z_Directory* directory, epoch now) {
	epoch duration = now - directory->last_accessed;

	if (duration < Z_HOUR)
		return directory->rank * 4.0;
	else if (duration < Z_DAY)
		return directory->rank * 2.0;
	else if (duration < Z_WEEK)
		return directory->rank * 0.5;
	else
		return directory->rank * 0.25;
}

