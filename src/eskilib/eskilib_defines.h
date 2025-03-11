/* Copyright eskilib by Alex Eski 2025 */

#ifndef ESKILIB_DEFINES_H_
#define ESKILIB_DEFINES_H_

/* INFO: eskilib_nodiscard: an attribute qualifier wrapper for [[nodiscard]].
 * If not using c2x/c23, does nothing, otherwise use [[nodiscard]] attribute.
 * The nodiscard attribute is helpful with refactors and preventing bugs from not checking return values.
 */

/* NOTE: IF YOU SEE AN ERROR BELOW:
 * ignore the error function-like macro '__has_c_attribute' is not defined
 * it only shows up in header files when using this attribute
 */

#if __has_c_attribute(nodiscard)
#if __STDC_VERSION__ >= 202000
#define eskilib_nodiscard [[nodiscard]]
#else
#define eskilib_nodiscard
#endif
#else
#define eskilib_nodiscard
#endif

#endif // !ESKILIB_DEFINES_H_
