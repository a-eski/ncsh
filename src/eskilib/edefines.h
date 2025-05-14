/* Copyright eskilib by Alex Eski 2025 */

/* clang-format off */

#pragma once

/* INFO: enodiscard: an attribute qualifier wrapper for [[nodiscard]].
 * If not using c2x/c23, does nothing, otherwise use [[nodiscard]] attribute.
 * The nodiscard attribute is helpful with refactors and preventing bugs from not checking return values.
 */

/* NOTE: IF YOU SEE AN ERROR BELOW:
 * ignore the error function-like macro '__has_c_attribute' is not defined
 * it only shows up in header files when using this attribute
 */

#if __has_c_attribute(nodiscard)
#    if __STDC_VERSION__ >= 202000
#        define enodiscard [[nodiscard]]
#    else
#        define enodiscard
#    endif
#else
#    define enodiscard
#endif

/* INFO: econstexpr: a qualifier wrapper for constexpr.
 * If not using c2x/c23, does nothing, otherwise use constexpr.
 * constexpr ensures compile time constant values.
 */

#if __STDC_VERSION__ == 202311L
#   define econstexpr constexpr
#else
#    define econstexpr
#endif

#ifndef rst
#   define rst restrict
#endif // !rst

/* clang-format on */
