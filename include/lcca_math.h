/**
 * @file lcca_math.h
 * @brief Lightweight mathematical utilities for angular computations.
 *
 * This header provides small mathematical helper functions used throughout the
 * LCCA library. The utilities focus on angle normalization and trigonometric
 * operations expressed in degrees rather than radians, matching the
 * conventions used by the library's astronomical algorithms.
 *
 * The functions are implemented as `static inline` to enable efficient
 * compilation without introducing additional translation units or function
 * call overhead.
 *
 * ## Provided functionality
 *
 * - Mathematical constant π (`LCCA_PI`)
 * - Angle normalization to the interval [0°, 360°)
 * - Trigonometric functions accepting degree arguments
 *
 * ## Design rationale
 *
 * The ISO C standard library defines trigonometric functions using radians.
 * Since astronomical formulae in the LCCA library are primarily expressed in
 * degrees, these wrappers eliminate repetitive degree-to-radian conversions
 * throughout the codebase while improving readability and reducing the
 * likelihood of unit-conversion errors.
 *
 * Angle normalization follows the mathematical modulo operation, ensuring that
 * equivalent angles (for example, -30°, 330°, and 690°) normalize to the same
 * canonical representation.
 *
 * ## Thread safety
 *
 * All functions are pure computations with no side effects or mutable global
 * state. They are fully reentrant and thread-safe.
 *
 * ## Memory management
 *
 * This header performs no dynamic memory allocation and maintains no
 * persistent state.
 *
 * ## Dependencies
 *
 * The implementations are thin wrappers around the ISO C `<math.h>` library
 * and require an implementation providing the standard floating-point
 * mathematical functions.
 */

#ifndef LCCA_MATH_H
#define LCCA_MATH_H

#include "lcca_numeric.h"
#include <math.h>


#define LCCA_PI (3.14159265358979323846)

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
