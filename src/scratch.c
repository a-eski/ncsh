size_t alias_len = len - i - 1;
printf("val %s len %zu\n", val + i + 1, alias_len);
aliases[aliases_count].alias->length = alias_len;
aliases[aliases_count].alias->value = arena_malloc(arena, alias_len, char);
memcpy(aliases[aliases_count].alias->value, val + i + 1, alias_len);
aliases[aliases_count].alias->value[alias_len - 1] = '\0';
