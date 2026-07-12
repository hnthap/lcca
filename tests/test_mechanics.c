#include "lcca_common.h"
#include "lcca_mechanics.h"
#include <stdbool.h>
#include <stdio.h>

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

/* Epsilon for sun longitude comparisons. Set to 0.01 degrees, which is the
 * documented accuracy of the Meeus Chapter 25 simplified algorithm relative
 * to high-precision ephemerides. Two implementations of the same formula
 * should agree to well within this bound. */
#define TEST_SUN_LONGITUDE_EPSILON (0.01)

#define IS_EQUAL_SUN_LONG(val, expected)                                       \
    ((((val) - (expected)) < TEST_SUN_LONGITUDE_EPSILON) &&                    \
     (((expected) - (val)) < TEST_SUN_LONGITUDE_EPSILON))

/* Epsilon for new moon JD comparisons, in days.
 * The full Meeus Chapter 47 formula is accurate to within ~1-2 minutes.
 * 0.001 day ≈ 86 seconds — wide enough to absorb the ΔT (UT→TD)
 * conversion (~64s in 2000) and minute-level precision of reference
 * sources, while tight enough to catch formula errors (which produce
 * discrepancies of hours, not seconds). */
#define TEST_NEW_MOON_JD_EPSILON (0.001)

#define IS_EQUAL_NEW_MOON_JD(val, expected)                                    \
    ((((val) - (expected)) < TEST_NEW_MOON_JD_EPSILON) &&                      \
     (((expected) - (val)) < TEST_NEW_MOON_JD_EPSILON))

/* =========================================================================
 * lcca_get_sun_true_longitude
 * ========================================================================= */

/**
 * @brief Tests lcca_get_sun_true_longitude with a precise oracle at the
 * J2000.0 epoch, and an approximate range check at the Meeus Chapter 25
 * worked-example date.
 *
 * Test Case 1 — J2000.0 epoch (JD 2451545.0):
 * At T = 0, the Meeus formula reduces to its constant terms with no
 * accumulated time error. The oracle value of 280.38 degrees was derived
 * analytically from the formula with T = 0:
 *   L0 = 280.46646
 *   M  = 357.52911
 *   C  = 1.914602 * sin(M) + 0.019993 * sin(2M) + 0.000289 * sin(3M)
 *      ≈ -0.08427
 *   L_true = 280.46646 + (-0.08427) = 280.38219
 * The Sun is in Sagittarius/Capricorn in early January, consistent with
 * ~280 degrees of ecliptic longitude. This case uses
 * TEST_SUN_LONGITUDE_EPSILON.
 *
 * Test Case 2 — Meeus Chapter 25 worked-example date, April 12.0, 1992 TD
 * (JD 2448724.5):
 * This is the date used in Example 25.a of Meeus (2nd ed., p. 165). The Sun
 * is in early Aries in April, approximately 22 days past the vernal equinox
 * (0 degrees), so the expected range is [19, 26] degrees. A precise oracle
 * is not given here because hand-computation of intermediate trigonometric
 * terms accumulates ~0.5 degree of error.
 *
 * @todo The user should verify the precise
 * expected value against the TypeScript reference implementation and tighten
 * this assertion to use IS_EQUAL_SUN_LONG once confirmed.
 */
static lcca_bool test_lcca_get_sun_true_longitude_nominal(void) {
    lcca_bool passed = true;
    lcca_f64 result;

    /* Test Case 1: J2000.0 epoch — JD 2451545.0 (January 1.5, 2000 TD) */
    /* Oracle: 280.38 degrees, derived analytically at T = 0 */
    result = lcca_get_sun_true_longitude(2451545.0);
    if (!lcca_c_assert(IS_EQUAL_SUN_LONG(result, 280.38))) {
        passed = false;
    }
    if (!lcca_c_assert(result >= 0.0 && result < 360.0)) {
        passed = false;
    }

    /* Test Case 2: Meeus Example 25.a date — JD 2448724.5 (April 12.0, 1992 TD)
     */
    /* Wide range check only: Sun in early Aries, expected approximately 22 */
    /* degrees (~22 days past the vernal equinox at 0 degrees) */
    result = lcca_get_sun_true_longitude(2448724.5);
    if (!lcca_c_assert(result >= 19.0 && result < 26.0)) {
        passed = false;
    }
    if (!lcca_c_assert(result >= 0.0 && result < 360.0)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests that lcca_get_sun_true_longitude always returns a normalized
 * value in [0, 360) even when the un-normalized intermediate result L0 + C
 * is far outside that range.
 *
 * At T = +1.0 (January 1.5, 2101 TD, JD 2488070.0), L0 accumulates to
 * approximately 36281 degrees — exactly 100 full rotations plus ~281 degrees.
 * A correct implementation normalizes this before returning. A naive
 * implementation that omits normalization would return a value near 36281,
 * which this test would catch.
 *
 * @todo The range [0, 360) is the only assertion here; a precise expected value
 * is not given because hand-computation confidence decreases at T far from 0.
 * The user should add a precise IS_EQUAL_SUN_LONG assertion after verifying
 * the value against the TypeScript reference implementation.
 */
static lcca_bool test_lcca_get_sun_true_longitude_normalization(void) {
    lcca_bool passed = true;
    lcca_f64 result;

    /* Test Case: T = +1.0 (JD 2488070.0, January 1.5, 2101 TD) */
    /* Un-normalized L0 ≈ 36281 degrees (100 full rotations plus ~281 degrees)
     */
    result = lcca_get_sun_true_longitude(2488070.0);
    if (!lcca_c_assert(result >= 0.0 && result < 360.0)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_get_new_moon_jd_td for k = -1, 0, and +1 against
 * oracle values derived from published UTC new moon times.
 *
 * Oracle sources (UTC times converted to JD; function returns JD(TD)):
 *   k=-1  Dec 7,  1999, 22:32 UTC -> JD(UT) ~2451520.439   (TheSkyLive 1999)
 *   k=0   Jan 6,  2000, 18:14 UTC -> JD(UT) ~2451550.260   (hermetic.ch;
 * multiple) k=1   Feb 5,  2000, 13:03 UTC -> JD(UT) ~2451580.044
 * (thelunaologist.com)
 *
 * The function returns JD(TD). UTC oracle values were not adjusted for
 * DeltaT (~0.00074 day in 2000) because that offset is within epsilon.
 *
 * @todo Verify all three oracle values against the TypeScript reference
 *       implementation or JPL Horizons (target 301; search elongation = 0
 *       near each date) to ensure the 1st-edition formula coefficients
 *       (Chapter 47) match. Tighten epsilon to 0.0002 (~17s) if confirmed.
 *
 * @todo Add test cases for large |k| (e.g. k = -500, k = 500) once
 *       reference values are available, to verify non-degradation at
 *       historical and future dates.
 */
static lcca_bool test_lcca_get_new_moon_jd_td_nominal(void) {
    lcca_bool passed = true;
    lcca_f64 result;

    /* k=-1: December 7, 1999 new moon */
    result = lcca_get_new_moon_jd_td(-1);
    if (!lcca_c_assert(IS_EQUAL_NEW_MOON_JD(result, 2451520.439))) {
        passed = false;
    }

    /* k=0: January 6, 2000 new moon — defining epoch of the numbering */
    result = lcca_get_new_moon_jd_td(0);
    if (!lcca_c_assert(IS_EQUAL_NEW_MOON_JD(result, 2451550.260))) {
        passed = false;
    }

    /* k=1: February 5, 2000 new moon */
    result = lcca_get_new_moon_jd_td(1);
    if (!lcca_c_assert(IS_EQUAL_NEW_MOON_JD(result, 2451580.044))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests structural invariants of lcca_get_new_moon_jd_td that hold
 * regardless of which edition's correction coefficients are used.
 *
 * Three invariants are verified for k = -1, 0, +1:
 *
 * 1. Monotonicity: output is strictly increasing with k. A bug in the
 *    sign of k's coefficient would break this.
 *
 * 2. Synodic month spacing: the JD difference between consecutive k values
 *    must lie within [29.27, 29.83] days — the observed astronomical range
 *    of synodic month lengths. The mean is 29.5306 days. A wrong coefficient
 *    or missing correction term would push the result outside this range.
 *
 * 3. Positive JD: all outputs must be positive. Even k=-1 (December 1999)
 *    is well above JD 0, so a negative result would indicate a sign error.
 */
static lcca_bool test_lcca_get_new_moon_jd_td_periodicity(void) {
    lcca_bool passed = true;

    lcca_f64 jd_m1 = lcca_get_new_moon_jd_td(-1);
    lcca_f64 jd_0 = lcca_get_new_moon_jd_td(0);
    lcca_f64 jd_p1 = lcca_get_new_moon_jd_td(1);

    lcca_f64 span_lower = jd_0 - jd_m1;
    lcca_f64 span_upper = jd_p1 - jd_0;

    /* Monotonicity */
    if (!lcca_c_assert(jd_m1 < jd_0)) {
        passed = false;
    }
    if (!lcca_c_assert(jd_0 < jd_p1)) {
        passed = false;
    }

    /* Synodic month bounds */
    if (!lcca_c_assert(span_lower >= 29.27 && span_lower <= 29.83)) {
        passed = false;
    }
    if (!lcca_c_assert(span_upper >= 29.27 && span_upper <= 29.83)) {
        passed = false;
    }

    /* Positive JD */
    if (!lcca_c_assert(jd_m1 > 0.0)) {
        passed = false;
    }
    if (!lcca_c_assert(jd_0 > 0.0)) {
        passed = false;
    }
    if (!lcca_c_assert(jd_p1 > 0.0)) {
        passed = false;
    }

    return passed;
}

/* =========================================================================
 * Entry point
 * ========================================================================= */

int main(void) {
    lcca_bool tests_passed = true;

    printf("=== Starting test_mechanics ===\n");

    /* Execute lcca_get_sun_true_longitude test suite */
    RUN_TEST(test_lcca_get_sun_true_longitude_nominal);
    RUN_TEST(test_lcca_get_sun_true_longitude_normalization);

    /* Execute lcca_get_new_moon_jd_td test suite */
    RUN_TEST(test_lcca_get_new_moon_jd_td_nominal);
    RUN_TEST(test_lcca_get_new_moon_jd_td_periodicity);

    printf("=== Test Suite Finished ===\n");

    return tests_passed ? 0 : 1;
}
