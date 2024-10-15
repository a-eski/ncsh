#ifndef ncsh_tokenize_h
#define ncsh_tokenize_h

#include "ncsh_args.h"
#include <stdint.h>

struct ncsh_Tokens {
	enum ncsh_Ops* ops;
	char*** tokens;
};

struct ncsh_Tokens ncsh_tokens_malloc(void);

void ncsh_tokens_free(struct ncsh_Tokens tokens);

void ncsh_tokens_values_free(struct ncsh_Tokens tokens);

struct ncsh_Tokens ncsh_tokenize(struct ncsh_Args args, struct ncsh_Tokens tokens);

#endif // !ncsh_tokenize_h

