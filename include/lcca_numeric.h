/**
 * @file lcca_numeric.h
 * @brief Fixed-width numeric type definitions used throughout the LCCA library.
 *
 * This header defines the fundamental scalar types used by the LCCA library.
 * Rather than exposing implementation-defined C primitive types directly, the
 * library adopts fixed-width integer and floating-point aliases built upon the
 * ISO C standard headers `<stdint.h>` and `<stdbool.h>`.
 *
 * Using library-defined numeric types provides a stable public interface,
 * improves portability across platforms, and makes data sizes explicit in API
 * declarations and documentation.
 *
 * ## Provided types
 *
 * The following aliases are defined:
 *
 * - `lcca_bool` — Boolean value
 * - `lcca_i8`   — Signed 8-bit integer
 * - `lcca_i16`  — Signed 16-bit integer
 * - `lcca_i32`  — Signed 32-bit integer
 * - `lcca_i64`  — Signed 64-bit integer
 * - `lcca_f32`  — IEEE 754 single-precision floating-point value
 * - `lcca_f64`  — IEEE 754 double-precision floating-point value
 *
 * These types are used consistently throughout the library to avoid ambiguity
 * arising from implementation-defined primitive types such as `int`, `long`,
 * or `double`.
 *
 * ## Compile-time verification
 *
 * The header performs compile-time validation of the floating-point type sizes
 * expected by the library. Compilation fails if:
 *
 * - `lcca_f32` is not 32 bits (4 bytes), or
 * - `lcca_f64` is not 64 bits (8 bytes).
 *
 * These checks ensure that the numerical algorithms used throughout the
 * library execute with the precision assumed during implementation and
 * verification.
 *
 * Integer widths are guaranteed by the ISO C fixed-width integer types defined
 * in `<stdint.h>`.
 *
 * ## Portability
 *
 * This header requires a C implementation providing:
 *
 * - `<stdbool.h>`
 * - `<stdint.h>`
 *
 * conforming to ISO C99 or later.
 *
 * ## Thread safety
 *
 * This header defines only type aliases and compile-time assertions. It
 * contains no executable code, mutable state, or runtime initialization and
 * is therefore inherently thread-safe.
 *
 * ## Memory management
 *
 * This header performs no dynamic memory allocation and requires no runtime
 * initialization or cleanup.
 */

#ifndef LCCA_NUMERIC_H
#define LCCA_NUMERIC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Boolean type.
 */
typedef bool lcca_bool;

/**
 * @brief 8-byte integer type.
 */
typedef int8_t lcca_i8;

/**
 * @brief 16-byte integer type.
 */
typedef int16_t lcca_i16;

/**
 * @brief 32-byte integer type.
 */
typedef int32_t lcca_i32;

/**
 * @brief 64-byte integer type.
 */
typedef int64_t lcca_i64;

/**
 * @brief 32-byte floating-point value type.
 */
typedef float lcca_f32;

/**
 * @brief 64-byte floating-point value type.
 */
typedef double lcca_f64;

typedef char assert_float32_size[sizeof(lcca_f32) == 4 ? 1 : -1];
typedef char assert_float64_size[sizeof(lcca_f64) == 8 ? 1 : -1];

#ifdef __cplusplus
}
#endif

#endif /* LCCA_NUMERIC_H */