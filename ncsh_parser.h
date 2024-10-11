#ifndef ncsh_parser_h
#define ncsh_parser_h

#include <stdint.h>

#include "ncsh_args.h"

struct ncsh_Args ncsh_parse(char line[], uint_fast32_t length, struct ncsh_Args args);

struct ncsh_Args ncsh_parse_and_tokenize(char line[], uint_fast32_t length, struct ncsh_Args args);

#endif // !ncsh_parser_h

