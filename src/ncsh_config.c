#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "eskilib/eskilib_colors.h"
#include "ncsh_defines.h"
#include "ncsh_config.h"
#include "eskilib/eskilib_result.h"

uint_fast32_t ncsh_config(char* out) {
	const char* const out_original_ptr = out;
	char* home = getenv("XDG_CONFIG_HOME");
	bool set_config = false;
	if (!home) {
		home = getenv("HOME");
		if (!home) { // neither HOME or XDG_CONFIG_HOME are available
			out[0] = 0;
			return 0;
		}
		set_config = true;
	}

	uint_fast32_t home_length = strlen(home);
	uint_fast32_t length = home_length;

	/* first +1 is "/", second is terminating null */
	if (home_length + 1 + NCSH_CONFIG_LENGTH + NCSH_LENGTH + 1 > NCSH_MAX_INPUT) {
		out[0] = 0;
		return 0;
	}

	memcpy(out, home, home_length);
	out += home_length;
	*out = '/';
	++length;
	++out;
	if (set_config) {
		memcpy(out, ".config/", NCSH_CONFIG_LENGTH);
		out += NCSH_CONFIG_LENGTH;
		*out = '\0';
		length += NCSH_CONFIG_LENGTH;
	}
	memcpy(out, NCSH, NCSH_LENGTH);
	out += NCSH_LENGTH;
	*out = '\0';
	length += NCSH_LENGTH;

	#ifdef NCSH_DEBUG
	printf("mkdir %s\n", out_original_ptr);
	#endif /* ifdef NCSH_DEBUG */
	mkdir(out_original_ptr, 0755);

	return length;
}

void ncsh_configure(struct eskilib_String config_location) {
	if (config_location.length == 0 || config_location.value == NULL) {
		return;
	}
}

enum eskilib_Result ncsh_config_init(struct ncsh_Config* config) {
	config->config_location.value = malloc(NCSH_MAX_INPUT);
	if (config->config_location.value == NULL) {
		perror(RED "ncsh: Error when allocating memory for config" RESET);
		fflush(stderr);
		return E_FAILURE_MALLOC;
	}
	config->config_location.length = ncsh_config(config->config_location.value);
	#ifdef  NCSH_DEBUG
	ncsh_debug_config(config_location);
	#endif /* ifdef  NCSH_DEBUG */

	ncsh_configure(config->config_location);

	return E_SUCCESS;
}

void ncsh_config_free(struct ncsh_Config* config) {
	free(config->config_location.value);
}
