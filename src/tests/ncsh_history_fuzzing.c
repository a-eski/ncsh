#include <stddef.h>
#include <stdint.h>

#include "../ncsh_history.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
	struct ncsh_History history = {};
	ncsh_history_malloc(&history);
	ncsh_history_load(&history);
	ncsh_history_add((char*)Data, Size, &history);
	ncsh_history_free(&history);
	return 0;
}

