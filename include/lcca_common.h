/**
 * @file lcca_common.h
 * @brief Common utilities, assertions, and shared infrastructure for the LCCA
 *        library.
 *
 * This header provides low-level facilities shared throughout the LCCA
 * library. Its primary purpose is to define the library's assertion mechanism
 * together with supporting infrastructure that remains lightweight,
 * deterministic, and suitable for both development and production builds.
 *
 * The assertion facility is intentionally implemented using a fixed-signature
 * debugging hook instead of variadic interfaces within library code. This
 * design improves static analyzability and supports compliance with coding
 * standards such as the Jet Propulsion Laboratory (JPL) C Coding Standard,
 * particularly Rule 20 regarding variadic functions.
 *
 * ## Provided functionality
 *
 * - Fixed-signature assertion failure reporting
 * - Dynamic assertion macro (`lcca_c_assert`)
 * - Common definitions shared across library components
 *
 * ## Assertion behavior
 *
 * The `lcca_c_assert()` macro evaluates a boolean expression.
 *
 * - If the expression evaluates to true, the macro evaluates to `true`.
 * - If the expression evaluates to false:
 *   - the configured debugging hook is invoked,
 *   - diagnostic information identifying the source file, line number, and
 *     failed expression is emitted,
 *   - the macro evaluates to `false`.
 *
 * Unlike the standard C `assert()` macro, this implementation does not abort
 * program execution. Instead, it provides a lightweight runtime diagnostic
 * mechanism that allows callers to determine how assertion failures should be
 * handled.
 *
 * ## Development implementation
 *
 * This header includes a default debugging implementation,
 * `lcca_tst_debugging()`, intended primarily for development and testing. The
 * default implementation prints assertion diagnostics to the standard error
 * stream using a fixed-parameter wrapper around `fprintf()`.
 *
 * Library users targeting embedded or safety-critical environments may replace
 * this implementation with an alternative reporting mechanism if desired.
 *
 * ## Thread safety
 *
 * The assertion macro itself maintains no internal state. Thread safety depends
 * solely on the behavior of the configured debugging implementation and the
 * underlying C runtime.
 *
 * ## Memory management
 *
 * This header performs no dynamic memory allocation and maintains no persistent
 * state.
 */

#ifndef LCCA_COMMON_H
#define LCCA_COMMON_H

#include "lcca_numeric.h"
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * A development-environment implementation of tst_debugging
 * that prints the formatted assertion failure to standard error
 * using a fixed-parameter signature to comply with JPL Rule 20.
 *
 * @param[in] file The name of the source file where the assertion failed.
 *
 * @param[in] line The line number where the assertion failed.
 *
 * @param[in] expr The stringified expression that evaluated to false.
 *
 * @returns (Does not return)
 */
static inline void lcca_tst_debugging(const char *file, lcca_i32 line,
                                      const char *expr) {
    /**
     * Note: While fprintf itself is variadic, this wrapper enforces
     * a strict, statically verifiable signature for our application code.
     */
    (void)fprintf(stderr, "%s,%d: assertion '%s' failed\n", file, (int)line,
                  expr);
}

#ifdef __cplusplus
}
#endif

/**
 * @brief Dynamic assertion macro.
 * Evaluates the expression. If false, triggers the fixed-signature
 * debugging hook and evaluates to false.
 */
#define lcca_c_assert(e)                                                       \
    ((e) ? (true)                                                              \
         : (lcca_tst_debugging(__FILE__, (lcca_i32)__LINE__, #e), false))

#endif /* LCCA_COMMON_H */
