#ifndef TERMINFO_GUARD_H_
#define TERMINFO_GUARD_H_

#include "lib/unibilium.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

unibi_term* terminfo_from_builtin(const char *term, char **termname);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !TERMINFO_GUARD_H_
