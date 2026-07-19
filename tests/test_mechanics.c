#include "lcca_common.h"
#include "lcca_mechanics.h"
#include <math.h>
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

/* Epsilon for lcca_approximate_k rate comparisons.
 * The approximation is linear with rate ~12.3685/year. Over a one-year
 * interval, the formula and a correct implementation should agree to within
 * 0.01; 0.05 is a comfortable margin that still catches a wrong coefficient
 * (e.g. 12.0 vs 12.3685 produces a 0.37 error over one year). */
#define TEST_APPROX_K_EPSILON (0.05)

#define IS_CLOSE_APPROX_K(val, expected)                                       \
    ((((val) - (expected)) < TEST_APPROX_K_EPSILON) &&                         \
     (((expected) - (val)) < TEST_APPROX_K_EPSILON))

/* Rounds a k approximation to the nearest integer, mirroring the rounding
 * convention applied in lcca_convert_gregorian_to_lunar:
 *     floor(lcca_approximate_k(gregorian, midnight) + 0.5)
 * Note: floor() is required (not a cast) for correct behavior on negative
 * values. */
#define ROUND_K(val) ((lcca_i32)floor((val) + 0.5))

/* Epsilon for winter solstice JD comparisons, in days.
 * The Meeus Chapter 27 seasonal-point formula is accurate to approximately
 * 45 minutes (0.031 days) against high-precision ephemerides. 0.05 days
 * (~72 minutes) absorbs formula error, Delta T (~0.001 days for modern
 * dates, negligible relative to epsilon), and published UTC oracle precision
 * (~1 minute). A wrong polynomial coefficient produces hour-scale or larger
 * errors, far exceeding this threshold. Once oracle values are verified
 * against the TypeScript reference implementation, tighten to ~1e-6 days
 * (floating-point agreement between two implementations of the same formula).
 */
#define TEST_WINTER_SOLSTICE_JD_EPSILON (0.05)

#define IS_EQUAL_WINTER_SOLSTICE_JD(val, expected)                             \
    ((((val) - (expected)) < TEST_WINTER_SOLSTICE_JD_EPSILON) &&               \
     (((expected) - (val)) < TEST_WINTER_SOLSTICE_JD_EPSILON))

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
 * lcca_approximate_k
 * ========================================================================= */

/**
 * @brief Tests that lcca_approximate_k rounds to the correct integer k at
 * the three known New Moon anchor dates (k = -1, 0, +1).
 *
 * Oracle dates are the same as in test_lcca_get_new_moon_jd_td_nominal:
 *   k = -1: Dec 7,  1999 (New Moon)
 *   k =  0: Jan 6,  2000 (New Moon; the definition of k = 0)
 *   k = +1: Feb 5,  2000 (New Moon)
 *
 * The assertion tested is ROUND_K(lcca_approximate_k(date, midnight)) == k,
 * which matches the rounding applied in lcca_convert_gregorian_to_lunar.
 *
 * All three oracles were verified analytically against (Y - 2000) * 12.3685:
 *   Dec 7, 1999: k_approx ≈ -0.847 -> ROUND_K = -1
 *   Jan 6, 2000: k_approx ≈  0.169 -> ROUND_K =  0
 *   Feb 5, 2000: k_approx ≈  1.182 -> ROUND_K = +1
 * Each is comfortably within ±0.35 of its target integer, so none of these
 * is near a rounding boundary.
 *
 * UTC (time_zone = 0.0) is used throughout. A ±24-hour timezone shift
 * affects k by at most 24/24/365 * 12.3685 ≈ 0.034, which cannot push
 * any of these values past a rounding boundary.
 *
 * @todo Add more anchor cases covering dates from different years (including
 * years containing lunar leap months) after verifying expected ROUND_K values
 * against the TypeScript reference implementation.
 */
static lcca_bool test_lcca_approximate_k_rounding(void) {
    lcca_bool passed = true;
    const lcca_time midnight = lcca_new_midnight_time();
    lcca_gregorian_date date;
    lcca_f64 result;

    /* k = -1: Dec 7, 1999 New Moon */
    date.year = 1999;
    date.month = 12;
    date.day = 7;
    date.time_zone = 0.0;
    result = lcca_approximate_k(date, midnight);
    if (!lcca_c_assert(ROUND_K(result) == -1)) {
        passed = false;
    }

    /* k = 0: Jan 6, 2000 New Moon (definition of k = 0) */
    date.year = 2000;
    date.month = 1;
    date.day = 6;
    date.time_zone = 0.0;
    result = lcca_approximate_k(date, midnight);
    if (!lcca_c_assert(ROUND_K(result) == 0)) {
        passed = false;
    }

    /* k = +1: Feb 5, 2000 New Moon */
    date.year = 2000;
    date.month = 2;
    date.day = 5;
    date.time_zone = 0.0;
    result = lcca_approximate_k(date, midnight);
    if (!lcca_c_assert(ROUND_K(result) == 1)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests monotonicity, annual rate, synodic-month rate, and
 * time-of-day sensitivity of lcca_approximate_k.
 *
 * Five structural invariants are verified:
 *
 * 1. Monotonicity (nearby): the result strictly increases across the three
 *    anchor dates Dec 7 1999 -> Jan 6 2000 -> Feb 5 2000. A sign error in
 *    the year-to-k coefficient would invert this.
 *
 * 2. Monotonicity (wide): the result is also strictly increasing from
 *    Jan 1 1900 -> Jan 1 2000 -> Jan 1 2100, exercising large negative
 *    and positive k values.
 *
 * 3. Rate over one synodic month: k(Feb 5, 2000) - k(Jan 6, 2000).
 *    These dates are 30 days apart; the expected difference is
 *    30 / 29.5306 ≈ 1.016. The range (0.9, 1.1) is used, which is wide
 *    enough to tolerate minor decimal-year implementation differences but
 *    catches a grossly wrong coefficient (e.g. 10.0/year would give ≈ 0.82).
 *    Values are reused from the monotonicity test above.
 *
 * 4. Rate over one calendar year: k(Jan 1, 2001) - k(Jan 1, 2000).
 *    The Meeus formula's explicit rate is 12.3685 lunations/year. Jan 1 is
 *    chosen for both endpoints to eliminate day-of-year fraction arithmetic
 *    from the oracle derivation. IS_CLOSE_APPROX_K with epsilon 0.05 is
 *    used, which catches a wrong coefficient (12.0 instead of 12.3685
 *    produces an error of 0.3685, well above the threshold).
 *
 * 5. Time-of-day sensitivity: for the same date, noon must give a strictly
 *    higher result than midnight. The expected delta is ~0.5/29.53 ≈ 0.017
 *    k units per 12-hour difference. A bug that ignores the time parameter
 *    entirely would produce equal results, failing this check.
 *
 * @todo Once reference values are available from the TypeScript implementation
 *       or JPL Horizons, add precise IS_CLOSE_APPROX_K assertions for the
 *       synodic-month and annual-rate cases to replace the range checks.
 */
static lcca_bool test_lcca_approximate_k_monotonicity_and_rate(void) {
    lcca_bool passed = true;
    const lcca_time midnight = lcca_new_midnight_time();
    lcca_gregorian_date d_dec, d_jan6, d_feb;
    lcca_f64 k_dec, k_jan6, k_feb;

    /* --- Monotonicity (nearby): three anchor dates --- */
    d_dec.year = 1999;
    d_dec.month = 12;
    d_dec.day = 7;
    d_dec.time_zone = 0.0;
    d_jan6.year = 2000;
    d_jan6.month = 1;
    d_jan6.day = 6;
    d_jan6.time_zone = 0.0;
    d_feb.year = 2000;
    d_feb.month = 2;
    d_feb.day = 5;
    d_feb.time_zone = 0.0;
    k_dec = lcca_approximate_k(d_dec, midnight);
    k_jan6 = lcca_approximate_k(d_jan6, midnight);
    k_feb = lcca_approximate_k(d_feb, midnight);
    if (!lcca_c_assert(k_dec < k_jan6)) {
        passed = false;
    }
    if (!lcca_c_assert(k_jan6 < k_feb)) {
        passed = false;
    }

    /* --- Monotonicity (wide): Jan 1, 1900 / 2000 / 2100 --- */
    {
        lcca_gregorian_date d_1900, d_2000, d_2100;
        d_1900.year = 1900;
        d_1900.month = 1;
        d_1900.day = 1;
        d_1900.time_zone = 0.0;
        d_2000.year = 2000;
        d_2000.month = 1;
        d_2000.day = 1;
        d_2000.time_zone = 0.0;
        d_2100.year = 2100;
        d_2100.month = 1;
        d_2100.day = 1;
        d_2100.time_zone = 0.0;
        if (!lcca_c_assert(lcca_approximate_k(d_1900, midnight) <
                           lcca_approximate_k(d_2000, midnight))) {
            passed = false;
        }
        if (!lcca_c_assert(lcca_approximate_k(d_2000, midnight) <
                           lcca_approximate_k(d_2100, midnight))) {
            passed = false;
        }
    }

    /* --- Rate over one synodic month: k(Feb 5) - k(Jan 6) --- */
    /* 30 days / 29.5306 days/month ≈ 1.016; acceptable range (0.9, 1.1) */
    {
        const lcca_f64 diff = k_feb - k_jan6;
        if (!lcca_c_assert(diff > 0.9 && diff < 1.1)) {
            passed = false;
        }
    }

    /* --- Rate over one calendar year: k(Jan 1, 2001) - k(Jan 1, 2000) --- */
    /* Expected: ~12.3685 (Meeus formula rate). Jan 1 chosen to eliminate
     * day-of-year fraction ambiguity from the oracle derivation. */
    {
        lcca_gregorian_date y2000, y2001;
        y2000.year = 2000;
        y2000.month = 1;
        y2000.day = 1;
        y2000.time_zone = 0.0;
        y2001.year = 2001;
        y2001.month = 1;
        y2001.day = 1;
        y2001.time_zone = 0.0;
        {
            const lcca_f64 annual_rate = lcca_approximate_k(y2001, midnight) -
                                         lcca_approximate_k(y2000, midnight);
            if (!lcca_c_assert(IS_CLOSE_APPROX_K(annual_rate, 12.3685))) {
                passed = false;
            }
        }
    }

    /* --- Time-of-day sensitivity --- */
    /* Noon must give a strictly higher result than midnight on the same date.
     * Expected delta ≈ 0.5/29.53 ≈ 0.017 k units per 12-hour difference. */
    {
        lcca_gregorian_date d;
        lcca_time noon;
        d.year = 2000;
        d.month = 6;
        d.day = 15;
        d.time_zone = 0.0;
        noon.hours = 12;
        noon.minutes = 0;
        noon.seconds = 0.0;
        if (!lcca_c_assert(lcca_approximate_k(d, midnight) <
                           lcca_approximate_k(d, noon))) {
            passed = false;
        }
    }

    return passed;
}

/* =========================================================================
 * lcca_get_winter_solstice_jd_td
 * ========================================================================= */

/**
 * @brief Tests lcca_get_winter_solstice_jd_td against oracle values for
 * AD 2000 and AD 2020 derived from published UTC solstice times.
 *
 * Oracles (UTC times converted to JD(UT); function returns JD(TD)):
 *
 *   year 2000: Dec 21, 13:37 UTC
 *              JD(UT) = 2451899.5 + (13 + 37/60) / 24
 *                     = 2451899.5 + 0.5674 = 2451900.067
 *
 *   year 2020: Dec 21, 10:02 UTC
 *              JD(UT) = 2459204.5 + (10 + 2/60) / 24
 *                     = 2459204.5 + 0.4181 = 2459204.918
 *
 * JD arithmetic for year 2000:
 *   Jan 1.0, 2000 midnight = JD 2451544.5 (J2000.0 − 0.5 days)
 *   Dec 21 = day 356 in 2000 (leap year):
 *     31+29+31+30+31+30+31+31+30+31+30+21 = 356
 *   Dec 21.0, 2000 midnight = JD 2451544.5 + 355 = 2451899.5
 *
 * JD arithmetic for year 2020:
 *   Leap years 2000-2019: 2000, 2004, 2008, 2012, 2016 = 5 years
 *   Jan 1.0, 2020 midnight = JD 2451544.5 + (5×366 + 15×365)
 *                          = JD 2451544.5 + 7305 = 2458849.5
 *   Dec 21 = day 356 in 2020 (leap year, same structure as 2000)
 *   Dec 21.0, 2020 midnight = JD 2458849.5 + 355 = 2459204.5
 *
 * The function returns JD(TD). Delta T in 2000 ≈ 63.8 s ≈ 0.000739 days;
 * in 2020 ≈ 69.9 s ≈ 0.000809 days. Both are far below
 * TEST_WINTER_SOLSTICE_JD_EPSILON and are not corrected in the oracles.
 *
 * @todo Verify both UTC oracle times against the TypeScript reference
 *       implementation or JPL Horizons (Sun ecliptic longitude = 270°
 *       crossing). Tighten epsilon to ~1e-6 once confirmed.
 *
 * @todo Add oracle cases for pre-2000 years (e.g. 1900, 1950) after
 *       sourcing verified historical solstice times, to exercise the Meeus
 *       Chapter 27 polynomial outside its well-tested modern range.
 */
static lcca_bool test_lcca_get_winter_solstice_jd_td_nominal(void) {
    lcca_bool passed = true;
    lcca_f64 result;

    /* Year 2000: Dec 21, 13:37 UTC -> JD(UT) = 2451900.067 */
    result = lcca_get_winter_solstice_jd_td(2000);
    if (!lcca_c_assert(IS_EQUAL_WINTER_SOLSTICE_JD(result, 2451900.067))) {
        passed = false;
    }
    if (!lcca_c_assert(result > 0.0)) {
        passed = false;
    }

    /* Year 2020: Dec 21, 10:02 UTC -> JD(UT) = 2459204.918 */
    result = lcca_get_winter_solstice_jd_td(2020);
    if (!lcca_c_assert(IS_EQUAL_WINTER_SOLSTICE_JD(result, 2459204.918))) {
        passed = false;
    }
    if (!lcca_c_assert(result > 0.0)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests monotonicity and annual rate of lcca_get_winter_solstice_jd_td.
 *
 * Five invariants verified:
 *
 * 1. Monotonicity (nearby): consecutive years 1999, 2000, 2001 are strictly
 *    increasing. A sign error on the leading year coefficient would invert
 *    the order or collapse all results to a constant.
 *
 * 2. Monotonicity (wide): years 1900, 2000, 2100 are strictly increasing.
 *    Exercises the polynomial's behavior across a 200-year span within the
 *    Meeus Chapter 27 intended domain (1000-3000).
 *
 * 3. Rate (1 year): jd(2001) - jd(2000) in [365.0, 366.0] days. The mean
 *    tropical year is 365.2422 days; year-to-year variation due to orbital
 *    perturbations is typically under 0.5 days. A wrong leading coefficient
 *    (e.g. 300 instead of ~365) would produce a result far outside this
 *    range. Values are reused from the nearby monotonicity check above.
 *
 * 4. Rate (4 years): jd(2004) - jd(2000) in [1459.0, 1463.0] days.
 *    Expected ≈ 4 × 365.2422 = 1460.97. The ±2-day margin absorbs
 *    short-term perturbations while rejecting grossly wrong coefficients.
 *
 * 5. Rate (100 years): jd(2100) - jd(2000) in [36519.0, 36529.0] days.
 *    Expected ≈ 100 × 365.2422 = 36524.2. The ±5-day margin tolerates
 *    polynomial fit residuals over a century-long span while catching any
 *    order-of-magnitude coefficient error.
 */
static lcca_bool
test_lcca_get_winter_solstice_jd_td_monotonicity_and_rate(void) {
    lcca_bool passed = true;

    /* --- Monotonicity (nearby) and 1-year rate: 1999, 2000, 2001 --- */
    {
        const lcca_f64 jd_1999 = lcca_get_winter_solstice_jd_td(1999);
        const lcca_f64 jd_2000 = lcca_get_winter_solstice_jd_td(2000);
        const lcca_f64 jd_2001 = lcca_get_winter_solstice_jd_td(2001);

        if (!lcca_c_assert(jd_1999 < jd_2000)) {
            passed = false;
        }
        if (!lcca_c_assert(jd_2000 < jd_2001)) {
            passed = false;
        }

        /* Rate (1 year): 2001 - 2000; expected ~365.2422, range [365.0, 366.0]
         */
        {
            const lcca_f64 diff_1yr = jd_2001 - jd_2000;
            if (!lcca_c_assert(diff_1yr >= 365.0 && diff_1yr <= 366.0)) {
                passed = false;
            }
        }
    }

    /* --- Monotonicity (wide), 4-year rate, and 100-year rate --- */
    {
        const lcca_f64 jd_1900 = lcca_get_winter_solstice_jd_td(1900);
        const lcca_f64 jd_2000 = lcca_get_winter_solstice_jd_td(2000);
        const lcca_f64 jd_2004 = lcca_get_winter_solstice_jd_td(2004);
        const lcca_f64 jd_2100 = lcca_get_winter_solstice_jd_td(2100);

        /* Monotonicity: 1900, 2000, 2100 */
        if (!lcca_c_assert(jd_1900 < jd_2000)) {
            passed = false;
        }
        if (!lcca_c_assert(jd_2000 < jd_2100)) {
            passed = false;
        }

        /* Rate (4 years): 2004 - 2000; expected ~1460.97, range [1459.0,
         * 1463.0] */
        {
            const lcca_f64 diff_4yr = jd_2004 - jd_2000;
            if (!lcca_c_assert(diff_4yr >= 1459.0 && diff_4yr <= 1463.0)) {
                passed = false;
            }
        }

        /* Rate (100 years): 2100 - 2000; expected ~36524.2, range [36519.0,
         * 36529.0] */
        {
            const lcca_f64 diff_100yr = jd_2100 - jd_2000;
            if (!lcca_c_assert(diff_100yr >= 36519.0 &&
                               diff_100yr <= 36529.0)) {
                passed = false;
            }
        }
    }

    return passed;
}

/**
 * @brief Tests that the Sun's ecliptic longitude at the computed winter
 * solstice JD is approximately 270 degrees.
 *
 * The winter solstice is defined as the moment the Sun's ecliptic longitude
 * reaches 270 degrees. This test composes lcca_get_winter_solstice_jd_td
 * with lcca_get_sun_true_longitude to verify internal consistency between
 * the two functions. Both accept and return JD(TD), so no Delta T conversion
 * is needed.
 *
 * The tolerance is [268.0, 272.0] (±2 degrees) rather than
 * TEST_SUN_LONGITUDE_EPSILON because lcca_get_winter_solstice_jd_td (Meeus
 * Ch. 27) and lcca_get_sun_true_longitude (Meeus Ch. 24) use different
 * polynomial fits to the same physical model. Inter-chapter discrepancy can
 * reach ~1-2 degrees in simplified implementations. A wrong-season error
 * (returning the summer solstice JD, which would yield ~90 degrees, or the
 * autumnal equinox at ~180 degrees) would produce a discrepancy far outside
 * this tolerance.
 *
 * @todo Once both functions are confirmed against the TypeScript reference,
 *       tighten the tolerance here. If both use internally consistent Meeus
 *       formulas, the actual discrepancy should be well under 0.1 degrees.
 */
static lcca_bool
test_lcca_get_winter_solstice_jd_td_sun_longitude_consistency(void) {
    lcca_bool passed = true;
    lcca_f64 sun_lon;

    /* Year 2000: sun longitude at computed solstice JD should be ~270 degrees
     */
    sun_lon = lcca_get_sun_true_longitude(lcca_get_winter_solstice_jd_td(2000));
    if (!lcca_c_assert(sun_lon >= 268.0 && sun_lon <= 272.0)) {
        passed = false;
    }

    /* Year 2019: same consistency check at a different date */
    sun_lon = lcca_get_sun_true_longitude(lcca_get_winter_solstice_jd_td(2019));
    if (!lcca_c_assert(sun_lon >= 268.0 && sun_lon <= 272.0)) {
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

    /* Execute lcca_approximate_k test suite */
    RUN_TEST(test_lcca_approximate_k_rounding);
    RUN_TEST(test_lcca_approximate_k_monotonicity_and_rate);

    /* Execute lcca_get_winter_solstice_jd_td test suite */
    RUN_TEST(test_lcca_get_winter_solstice_jd_td_nominal);
    RUN_TEST(test_lcca_get_winter_solstice_jd_td_monotonicity_and_rate);
    RUN_TEST(test_lcca_get_winter_solstice_jd_td_sun_longitude_consistency);

    printf("=== Test Suite Finished ===\n");

    return tests_passed ? 0 : 1;
}
