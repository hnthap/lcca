#ifndef LCCA_MATH_H
#define LCCA_MATH_H

#include <math.h>
#include "lcca_numeric.h"

#define LCCA_PI (3.1415926535898)

#ifdef __cplusplus
extern "C" { 
#endif

/**
 * @brief Normalizes degrees in the range from 0 (inclusive) to 360 (exclusive).
 *
 * @post This function has no side effects.
 *
 * @param[in] degrees Degrees
 *
 * @returns Normalized degrees
 */
static inline lcca_f64 lcca_normalize_degrees(lcca_f64 degrees) {
    return fmod(fmod(degrees, 360.0) + 360.0, 360.0);
}

/**
 * @brief Calculates the sine of specified degrees.
 *
 * @post This function has no side effects.
 *
 * @param[in] degrees Degrees
 *
 * @returns Sine of the degrees
 */
static inline lcca_f64 lcca_sin_degrees(lcca_f64 degrees) {
    return sin(degrees * LCCA_PI / 180.0);
}

#ifdef __cplusplus
}
#endif

#endif /* LCCA_MATH_H */
