    struct Autocompletion_Node* prev = tree;
    size_t prev_i = 0;
    struct Autocompletion_Node* node = tree->nodes[0];
    size_t i = 0;

    while (true) {
	if (i == NCSH_LETTERS) {
            node = prev;
	    i = prev_i;
	    --ac_str_pos;
	    continue;
	}

	if (!node) {
	    ++i;
            ++node;
	    continue;
        }

        if (!matches[ac_match_pos].value) {
            matches[ac_match_pos].value = arena_malloc(scratch, NCSH_MAX_INPUT, char);

            if (ac_match_pos) {
                memcpy(matches[ac_match_pos].value, matches[ac_match_pos - 1].value, ac_str_pos);
            }
        }

        matches[ac_match_pos].value[ac_str_pos] = index_to_char(i);
        ++ac_str_pos;

        if (node->is_end_of_a_word) {
            matches[ac_match_pos].weight = node->weight;
            ++ac_match_pos;
        }

	prev = node;
	prev_i = i;
	i = 0;
	node = node->nodes[0];
    }
