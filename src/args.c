/* Copyright ncsh (C) by Alex Eski 2025 */
/* args.c: a linked list for storing tokens outputted by the parser */

#include <assert.h>
#include <string.h>

#include "arena.h"
#include "args.h"

struct Args* args_alloc(struct Arena* const restrict arena)
{
    struct Args* args = arena_malloc(arena, 1, struct Args);
    args->count = 0;
    // allocate the first head as empty with no op (0 is OP_NONE)
    args->head = arg_alloc(0, 1, "\0", arena);
    return args;
}

struct Arg* arg_alloc(uint8_t op, size_t len, char* val, struct Arena* const restrict arena)
{
    struct Arg* arg = arena_malloc(arena, 1, struct Arg);
    arg->val = arena_malloc(arena, len, char);
    memcpy(arg->val, val, len);
    arg->op = op;
    arg->len = len;
    arg->next = NULL;
    return arg;
}

bool arg_set_after(struct Arg* current, struct Arg* nodeToSetAfter)
{
	assert(current);
	assert(nodeToSetAfter);
	if (!current || !nodeToSetAfter) {
		return false;
	}

	struct Arg* temporaryNode = current->next;
	current->next = nodeToSetAfter;
	nodeToSetAfter->next = temporaryNode;

	return true;
}

bool arg_set_last(struct Args* args, struct Arg* nodeToSetLast)
{
	assert(args && args->head);
	if (!args) {
		return false;
	}

	assert(nodeToSetLast);
	if (!nodeToSetLast) {
		return false;
	}

	struct Arg* current = args->head;

	while (current->next) {
		current = current->next;
	}

	current->next = nodeToSetLast;
	nodeToSetLast->next = NULL;
    ++args->count;

	return true;
}
