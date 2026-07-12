#include "lcca_common.h"
#include "lcca_math.h"
#include <stdbool.h>
#include <stdio.h>

/* * Utility macro for the test harness.
 * Since lcca_c_assert logs to stderr and returns false without aborting,
 * we use this macro to track test failures safely and exit the test suite
 * gracefully if necessary.
 */
#define RUN_TEST(test_func)                                                    \
    do {                                                                       \
        printf("Running %s...\n", #test_func);                                 \
        if (test_func()) {                                                     \
            printf("  [PASS]\n");                                              \
        } else {                                                               \
            printf("  [FAIL]\n");                                              \
            tests_passed = false;                                              \
        }                                                                      \
    } while (0)

/* Epsilon for pure math comparisons. Tighter than TEST_JD_EPSILON because
 * lcca_math functions involve no astronomical approximation — only
 * floating-point arithmetic against known analytical values. */
#define TEST_MATH_EPSILON (1e-9)

/* Macro to safely compare lcca_f64 values within the math tolerance bracket */
#define IS_EQUAL_MATH(val, expected)                                           \
    ((((val) - (expected)) < TEST_MATH_EPSILON) &&                             \
     (((expected) - (val)) < TEST_MATH_EPSILON))

/* =========================================================================
 * lcca_normalize_degrees
 * ========================================================================= */

/**
 * @brief Tests lcca_normalize_degrees with values already within [0, 360).
 *
 * Verifies that in-range values are returned unchanged: the lower bound
 * (inclusive), the midpoint, and a value just below the exclusive upper bound.
 */
static lcca_bool test_lcca_normalize_degrees_nominal(void) {
    lcca_bool passed = true;

    /* Test Case: Lower bound (inclusive) */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(0.0), 0.0))) {
        passed = false;
    }

    /* Test Case: Quarter */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(90.0), 90.0))) {
        passed = false;
    }

    /* Test Case: Midpoint */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(180.0), 180.0))) {
        passed = false;
    }

    /* Test Case: Just below upper bound (exclusive) */
    if (!lcca_c_assert(
            IS_EQUAL_MATH(lcca_normalize_degrees(359.999), 359.999))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_normalize_degrees with values that require wrapping.
 *
 * Covers positive overflow (including exact multiples of 360), negative
 * values, and combined negative overflow. These exercise the double-fmod
 * implementation's handling of both the positive and negative domains.
 */
static lcca_bool test_lcca_normalize_degrees_wrap(void) {
    lcca_bool passed = true;

    /* Test Case: Exact upper bound wraps to 0 */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(360.0), 0.0))) {
        passed = false;
    }

    /* Test Case: Exact double multiple of 360 wraps to 0 */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(720.0), 0.0))) {
        passed = false;
    }

    /* Test Case: Positive overflow with fractional part */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(361.5), 1.5))) {
        passed = false;
    }

    /* Test Case: Small negative */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(-1.0), 359.0))) {
        passed = false;
    }

    /* Test Case: Negative midpoint */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(-180.0), 180.0))) {
        passed = false;
    }

    /* Test Case: Exact negative multiple of 360 wraps to 0 */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(-360.0), 0.0))) {
        passed = false;
    }

    /* Test Case: Negative overflow with fractional part */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_normalize_degrees(-361.5), 358.5))) {
        passed = false;
    }

    return passed;
}

/* =========================================================================
 * lcca_sin_degrees
 * ========================================================================= */

/**
 * @brief Tests lcca_sin_degrees against known exact analytical values.
 *
 * Covers the standard cardinal angles (0, 90, 180, 270, 360 degrees) whose
 * sine values are analytically exact (0, 1, 0, -1, 0), a 30-degree angle
 * (sine = 0.5 exactly), a negative input, and a value above 360 degrees.
 *
 * Note: sin(180) and sin(360) are not exactly 0 in floating point due to
 * the finite precision of LCCA_PI (3.1415926535898). The residual error
 * (~1e-13) is well within TEST_MATH_EPSILON and the comparison is correct.
 */
static lcca_bool test_lcca_sin_degrees_nominal(void) {
    lcca_bool passed = true;

    /* Test Case: sin(0°) = 0 */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_sin_degrees(0.0), 0.0))) {
        passed = false;
    }

    /* Test Case: sin(30°) = 0.5 */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_sin_degrees(30.0), 0.5))) {
        passed = false;
    }

    /* Test Case: sin(90°) = 1 */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_sin_degrees(90.0), 1.0))) {
        passed = false;
    }

    /* Test Case: sin(180°) ≈ 0 (floating-point residual ~1e-13) */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_sin_degrees(180.0), 0.0))) {
        passed = false;
    }

    /* Test Case: sin(270°) = -1 */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_sin_degrees(270.0), -1.0))) {
        passed = false;
    }

    /* Test Case: sin(360°) ≈ 0 (floating-point residual ~1e-13) */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_sin_degrees(360.0), 0.0))) {
        passed = false;
    }

    /* Test Case: Negative input — sin(-90°) = -1 */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_sin_degrees(-90.0), -1.0))) {
        passed = false;
    }

    /* Test Case: Input above 360° — sin(450°) = sin(90°) = 1 */
    /* Confirms the function does not assume bounded input */
    if (!lcca_c_assert(IS_EQUAL_MATH(lcca_sin_degrees(450.0), 1.0))) {
        passed = false;
    }

    return passed;
}

/* =========================================================================
 * Entry point
 * ========================================================================= */

int main(void) {
    lcca_bool tests_passed = true;

    printf("=== Starting test_math ===\n");

    /* Execute lcca_normalize_degrees test suite */
    RUN_TEST(test_lcca_normalize_degrees_nominal);
    RUN_TEST(test_lcca_normalize_degrees_wrap);

    /* Execute lcca_sin_degrees test suite */
    RUN_TEST(test_lcca_sin_degrees_nominal);

    printf("=== Test Suite Finished ===\n");

    return tests_passed ? 0 : 1;
}
