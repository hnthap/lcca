#ifndef LCCA_COMMON_H
#define LCCA_COMMON_H

#include <stdio.h>
#include "lcca_numeric.h"

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
static inline void lcca_tst_debugging(const char *file, lcca_i32 line, const char *expr) {
    /**
     * Note: While fprintf itself is variadic, this wrapper enforces 
     * a strict, statically verifiable signature for our application code. 
     */
    (void)fprintf(stderr, "%s,%d: assertion '%s' failed\n", file, (int)line, expr);
}

#ifdef __cplusplus
}
#endif

/**
 * @brief Dynamic assertion macro.
 * Evaluates the expression. If false, triggers the fixed-signature 
 * debugging hook and evaluates to false.
 */
#define lcca_c_assert(e) ((e) ? (true) : \
    (lcca_tst_debugging(__FILE__, (lcca_i32)__LINE__, #e), false))

#endif /* LCCA_COMMON_H */
