//for testing only

#include "z_main.h"
#include "../eskilib/eskilib_string.h"

int main(void) {
	z_start();

	struct eskilib_String input = { .length = 5, .value = "ncsh" };
	struct eskilib_String* output = z_process(&input);

	z_finish();
	return 0;
}
