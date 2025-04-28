#include "arena.h"
#include "eskilib/eresult.h"
#include <assert.h>
#include <string.h>
#include "args.h"

struct Arg* arg_alloc(uint8_t op, size_t len, char* val, struct Arena* const restrict arena)
{
    struct Arg* arg = arena_malloc(arena, 1, struct Arg);
    arg->val = arena_malloc(arena, len, char);
    memcpy(arg->val, val, len);
    arg->op = op;
    arg->len = len;
    return arg;
}

enum eresult arg_set_after(struct Arg* currentNode, struct Arg* nodeToSetAfter)
{
	assert(currentNode);
	assert(nodeToSetAfter);
	if (!currentNode || ! nodeToSetAfter) {
		return E_FAILURE_NULL_REFERENCE;
	}

	struct Arg* temporaryNode = currentNode->next;
	currentNode->next = nodeToSetAfter;
	nodeToSetAfter->next = temporaryNode;

	return E_SUCCESS;
}
