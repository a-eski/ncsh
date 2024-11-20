// old autocomplete implementation, may be useful at some point
/*void ncsh_autocomplete(char* buffer, uint_fast32_t buf_position, char* current_autocompletion, struct ncsh_Autocompletion_Node* autocompletions_tree) {
	struct ncsh_Autocompletion autocompletion_matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
	uint_fast32_t autocompletions_matches_count = ncsh_autocompletions_get(buffer, buf_position + 1, autocompletion_matches, autocompletions_tree);

	if (autocompletions_matches_count == 0) {
		current_autocompletion[0] = '\0';
		return;
	}

	putchar('\n');
	for (uint_fast8_t i = 0; i < autocompletions_matches_count; ++i)
		printf("match[%d] (weight: %d) %s\n", i, autocompletion_matches[i].weight, autocompletion_matches[i].value);

	struct ncsh_Coordinates position = ncsh_terminal_position();
	if (position.x == 0 && position.y == 0)
		return;

	eskilib_string_copy(current_autocompletion, autocompletion_matches[0].value, NCSH_MAX_INPUT);

	for (uint_fast32_t i = 0; i < autocompletions_matches_count; ++i)
		free(autocompletion_matches[i].value);

	printf(WHITE_DIM "%s" RESET, current_autocompletion);
	ncsh_terminal_move(position.x, position.y);
	fflush(stdout);
}*/
