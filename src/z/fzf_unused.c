#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "../ncsh_arena.h"

char *str_replace(char *orig,
                  size_t orig_len,
                  char *rep,
                  char *with,
                  struct ncsh_Arena* const arena) {
  if (!orig || !rep || !with) {
    return NULL;
  }

  char *result;
  char *ins;
  char *tmp;

  size_t len_rep = strlen(rep);
  size_t len_front = 0;
  size_t len_with = strlen(with);
  size_t count = 0;

  if (len_rep == 0) {
    return NULL;
  }

  ins = orig;
  for (; (tmp = strstr(ins, rep)); ++count) {
    ins = tmp + len_rep;
  }

  tmp = result = (char *)malloc(orig_len + (len_with - len_rep) * count + 1);
  if (!result) {
    return NULL;
  }

  while (count--) {
    ins = strstr(orig, rep);
    len_front = (size_t)(ins - orig);
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep;
    orig_len -= len_front + len_rep;
  }
  strncpy(tmp, orig, orig_len);
  tmp[orig_len] = 0;
  return result;
}

