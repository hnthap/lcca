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