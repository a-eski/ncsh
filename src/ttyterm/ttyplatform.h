/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */
/* ttyplatform.h: platform layer for the ttyterm library */

#ifndef TTYPLATFORM_GUARD_H_
#define TTYPLATFORM_GUARD_H_

// Definitions needed for all unix like envirnoments. Includes unistd in unix like environments
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

#   include <unistd.h>

#endif /* ifdef  defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64) || defined(__CYGWIN__) */


#if __STDC_VERSION__ >= 202311L /* C23 */
#   define _MAYBE_UNUSED_ [[maybe_unused]]
#else
#   if defined(__GNUC__) || defined(__clang__)
#       define _MAYBE_UNUSED_ __attribute__((unused))
#   else
#       define _MAYBE_UNUSED_
#   endif /* defined(__GNUC__) || defined(__clang__) */
#endif /* C23 */



// Definitions needed for pre-C23 and cross-compilation from Windows to other platforms. Also needed for Apple and the BSDs.
#if __STDC_VERSION__ < 202311L || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) && (!defined(__MINGW32__) && !defined(__MINGW64__))

#   include <stdbool.h>

#   if defined(__GNUC__) || defined(__clang__)
#       ifndef unreachable
#           define unreachable() __builtin_unreachable()
#       endif
#   elif defined(_MSY_VER)
#       ifndef unreachable
#           define unreachable() __assume(0)
#       endif
#   else
#       ifndef unreachable
#           define unreachable() assert(0 && "unreachable");
#       endif /* ifndef unreachable */
#   endif

#endif



// Definitions needed for Cygwin
#if defined(__CYGWIN__)
#   define strdup __builtin_strdup
#endif /* defined(__CYGWIN__) */



// Definitions needed for MSYS2 Environments
#if defined(__MINGW64__)
#   define SIZE_T_FMT "%llu"
#elif defined(__MINGW32__)
#   define SIZE_T_FMT "%u"
#else
#   define SIZE_T_FMT "%zu"
#endif /* defined(__MINGW64__) */



#endif /* TTYPLATFORM_GUARD_H_ */
