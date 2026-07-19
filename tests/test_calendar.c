#include "lcca_calendar.h"
#include "lcca_common.h"
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

/* Standard tolerance for Julian Date comparisons (roughly 0.086 seconds of
 * precision) */
#define TEST_JD_EPSILON (0.001)

/* Macro to safely compare lcca_f64 values within a tolerance bracket */
#define IS_EQUAL_F64(val, expected)                                            \
    ((((val) - (expected)) < TEST_JD_EPSILON) &&                               \
     (((expected) - (val)) < TEST_JD_EPSILON))

/* Dedicated epsilon for Delta T verification */
#define TEST_DELTA_T_EPSILON (0.01)

/* Macro for Delta T comparison */
#define IS_EQUAL_DELTA_T(val, expected)                                        \
    ((((val) - (expected)) < TEST_DELTA_T_EPSILON) &&                          \
     (((expected) - (val)) < TEST_DELTA_T_EPSILON))

/**
 * @brief Tests the exact initialization values of lcca_new_midnight_time.
 * * @returns true if all fields are exactly zero, false otherwise.
 */
static lcca_bool test_lcca_new_midnight_time_values(void) {
    lcca_bool passed = true;

    /* 1. Execution */
    lcca_time midnight = lcca_new_midnight_time();

    /* 2. Verification */
    if (!lcca_c_assert(midnight.hours == 0)) {
        passed = false;
    }
    if (!lcca_c_assert(midnight.minutes == 0)) {
        passed = false;
    }

    /* Floating point literal comparison is safe here because it is assigned
     * exactly to 0 */
    if (!lcca_c_assert(midnight.seconds == 0.0)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests that the generated midnight time is considered a valid time of
 * day.
 * * @returns true if the structural validator accepts it, false otherwise.
 */
static lcca_bool test_lcca_new_midnight_time_validity(void) {
    lcca_bool passed = true;

    /* 1. Execution */
    lcca_time midnight = lcca_new_midnight_time();

    /* 2. Verification */
    /* Assumes lcca_is_valid_time_of_day is implemented and accessible */
    if (!lcca_c_assert(lcca_is_valid_time_of_day(midnight) == true)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_is_valid_time_of_day with valid, in-bounds time values.
 * * Verifies that standard times, including lower bounds and upper extremes
 * just before the rollover, are accepted.
 */
static lcca_bool test_lcca_is_valid_time_of_day_nominal(void) {
    lcca_bool passed = true;
    lcca_time time_test;

    /* Test Case: Absolute Minimum (Midnight) */
    time_test.hours = 0;
    time_test.minutes = 0;
    time_test.seconds = 0.0;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == true)) {
        passed = false;
    }

    /* Test Case: Middle of the day */
    time_test.hours = 12;
    time_test.minutes = 30;
    time_test.seconds = 30.5;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == true)) {
        passed = false;
    }

    /* Test Case: Absolute Maximum (Just before next midnight) */
    time_test.hours = 23;
    time_test.minutes = 59;
    time_test.seconds = 59.999;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == true)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_is_valid_time_of_day with invalid, out-of-bounds time
 * values.
 * * Verifies that the function correctly rejects negative values and values
 * that meet or exceed the exclusive upper bounds.
 */
static lcca_bool test_lcca_is_valid_time_of_day_off_nominal(void) {
    lcca_bool passed = true;
    lcca_time time_test;

    /* Test Case: Hours out of bounds (Upper) */
    time_test.hours = 24;
    time_test.minutes = 0;
    time_test.seconds = 0.0;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == false)) {
        passed = false;
    }

    /* Test Case: Hours out of bounds (Lower) */
    time_test.hours = -1;
    time_test.minutes = 0;
    time_test.seconds = 0.0;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == false)) {
        passed = false;
    }

    /* Test Case: Minutes out of bounds (Upper) */
    time_test.hours = 12;
    time_test.minutes = 60;
    time_test.seconds = 0.0;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == false)) {
        passed = false;
    }

    /* Test Case: Minutes out of bounds (Lower) */
    time_test.hours = 12;
    time_test.minutes = -1;
    time_test.seconds = 0.0;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == false)) {
        passed = false;
    }

    /* Test Case: Seconds out of bounds (Upper) */
    time_test.hours = 12;
    time_test.minutes = 30;
    time_test.seconds = 60.0;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == false)) {
        passed = false;
    }

    /* Test Case: Seconds out of bounds (Lower) */
    time_test.hours = 12;
    time_test.minutes = 30;
    time_test.seconds = -0.001;
    if (!lcca_c_assert(lcca_is_valid_time_of_day(time_test) == false)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_is_valid_gregorian_date with nominal, historically valid
 * dates.
 */
static lcca_bool test_lcca_is_valid_gregorian_date_nominal(void) {
    lcca_bool passed = true;
    lcca_gregorian_date date_test;

    /* Test Case: Standard 31-day month */
    date_test.time_zone = 0.0;
    date_test.year = 2023;
    date_test.month = 10;
    date_test.day = 15;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == true)) {
        passed = false;
    }

    /* Test Case: 400-year Leap Year (Feb 29) */
    date_test.time_zone = -5.0;
    date_test.year = 2000;
    date_test.month = 2;
    date_test.day = 29;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == true)) {
        passed = false;
    }

    /* Test Case: Standard 4-year Leap Year (Feb 29) */
    date_test.time_zone = 7.0;
    date_test.year = 2004;
    date_test.month = 2;
    date_test.day = 29;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == true)) {
        passed = false;
    }

    /* Test Case: Astronomical Year 0 (1 BC), which is a leap year */
    date_test.time_zone = 0.0;
    date_test.year = 0;
    date_test.month = 2;
    date_test.day = 29;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == true)) {
        passed = false;
    }

    /* Test Case: 30-day month upper bound */
    date_test.time_zone = 14.0;
    date_test.year = 1999;
    date_test.month = 4;
    date_test.day = 30;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == true)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_is_valid_gregorian_date with out-of-bounds configurations.
 */
static lcca_bool test_lcca_is_valid_gregorian_date_off_nominal(void) {
    lcca_bool passed = true;
    lcca_gregorian_date date_test;

    /* Test Case: Invalid 100-year Century Leap Year (e.g., 1900) */
    date_test.time_zone = 0.0;
    date_test.year = 1900;
    date_test.month = 2;
    date_test.day = 29;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == false)) {
        passed = false;
    }

    /* Test Case: 31st day in a 30-day month */
    date_test.time_zone = 0.0;
    date_test.year = 2023;
    date_test.month = 11;
    date_test.day = 31;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == false)) {
        passed = false;
    }

    /* Test Case: Month out of bounds (Upper) */
    date_test.time_zone = 0.0;
    date_test.year = 2023;
    date_test.month = 13;
    date_test.day = 1;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == false)) {
        passed = false;
    }

    /* Test Case: Month out of bounds (Lower) */
    date_test.time_zone = 0.0;
    date_test.year = 2023;
    date_test.month = 0;
    date_test.day = 1;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == false)) {
        passed = false;
    }

    /* Test Case: Day out of bounds (Lower) */
    date_test.time_zone = 0.0;
    date_test.year = 2023;
    date_test.month = 1;
    date_test.day = 0;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == false)) {
        passed = false;
    }

    /* Test Case: Time zone out of bounds */
    date_test.time_zone = 25.0;
    date_test.year = 2023;
    date_test.month = 1;
    date_test.day = 1;
    if (!lcca_c_assert(lcca_is_valid_gregorian_date(date_test) == false)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_get_gregorian_month_size with valid, in-bounds dates.
 * * Verifies standard months, non-leap year Februarys, standard leap year
 * Februarys, and 400-year cycle leap year Februarys.
 */
static lcca_bool test_lcca_get_gregorian_month_size_nominal(void) {
    lcca_bool passed = true;
    lcca_gregorian_date date_test;
    lcca_i8 size;

    /* Test Case: 31-day month (January) */
    date_test.time_zone = 0.0;
    date_test.year = 2023;
    date_test.month = 1;
    date_test.day =
        1; /* Day is irrelevant for this function, but initialized */
    size = lcca_get_gregorian_month_size(date_test);
    if (!lcca_c_assert(size == 31)) {
        passed = false;
    }

    /* Test Case: 30-day month (April) */
    date_test.year = 2023;
    date_test.month = 4;
    size = lcca_get_gregorian_month_size(date_test);
    if (!lcca_c_assert(size == 30)) {
        passed = false;
    }

    /* Test Case: February, standard non-leap year */
    date_test.year = 2023;
    date_test.month = 2;
    size = lcca_get_gregorian_month_size(date_test);
    if (!lcca_c_assert(size == 28)) {
        passed = false;
    }

    /* Test Case: February, 100-year century non-leap year */
    date_test.year = 1900;
    date_test.month = 2;
    size = lcca_get_gregorian_month_size(date_test);
    if (!lcca_c_assert(size == 28)) {
        passed = false;
    }

    /* Test Case: February, standard 4-year leap year */
    date_test.year = 2024;
    date_test.month = 2;
    size = lcca_get_gregorian_month_size(date_test);
    if (!lcca_c_assert(size == 29)) {
        passed = false;
    }

    /* Test Case: February, 400-year century leap year */
    date_test.year = 2000;
    date_test.month = 2;
    size = lcca_get_gregorian_month_size(date_test);
    if (!lcca_c_assert(size == 29)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_get_gregorian_month_size with a precondition violation.
 * * Since lcca_c_assert returns false and delegates logging without aborting,
 * a robust implementation must return a safe fail-state value (like -1).
 */
static lcca_bool test_lcca_get_gregorian_month_size_violation(void) {
    lcca_bool passed = true;
    lcca_gregorian_date date_test;
    lcca_i8 size;

    /* Test Case: Invalid month (triggers precondition assertion) */
    date_test.time_zone = 0.0;
    date_test.year = 2023;
    date_test.month = 13;
    date_test.day = 1;

    size = lcca_get_gregorian_month_size(date_test);

    if (!lcca_c_assert(size == -1)) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_convert_gregorian_to_jd_ut with recognized astronomical
 * epochs.
 *
 * Relies entirely on the caller respecting the precondition of valid inputs.
 */
static lcca_bool test_lcca_convert_gregorian_to_jd_ut_nominal(void) {
    lcca_bool passed = true;
    lcca_gregorian_date date_test;
    lcca_time time_test;
    lcca_f64 jd_result;

    /* Test Case 1: Standard J2000.0 Epoch (January 1.5, 2000, UT) */
    /* Source: Meeus (1991) p. 62 */
    date_test.time_zone = 0.0;
    date_test.year = 2000;
    date_test.month = 1;
    date_test.day = 1;

    time_test.hours = 12;
    time_test.minutes = 0;
    time_test.seconds = 0.0;

    jd_result = lcca_convert_gregorian_to_jd_ut(date_test, time_test);
    if (!lcca_c_assert(IS_EQUAL_F64(jd_result, 2451545.0))) {
        passed = false;
    }

    /* Test Case 2: Launch of Sputnik 1 (October 4, 1957, at 19:28:34 UT) */
    date_test.time_zone = 0.0;
    date_test.year = 1957;
    date_test.month = 10;
    date_test.day = 4;

    time_test.hours = 19;
    time_test.minutes = 28;
    time_test.seconds = 34.0;

    jd_result = lcca_convert_gregorian_to_jd_ut(date_test, time_test);
    if (!lcca_c_assert(IS_EQUAL_F64(jd_result, 2436116.3115046))) {
        passed = false;
    }

    /* Test Case 3: Time zone handling (J2000 Epoch stated in +05:00 local time)
       January 1, 2000, 17:00:00 in +05:00 is exactly 12:00:00 UT. */
    date_test.time_zone = 5.0;
    date_test.year = 2000;
    date_test.month = 1;
    date_test.day = 1;

    time_test.hours = 17;
    time_test.minutes = 0;
    time_test.seconds = 0.0;

    jd_result = lcca_convert_gregorian_to_jd_ut(date_test, time_test);
    if (!lcca_c_assert(IS_EQUAL_F64(jd_result, 2451545.0))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_convert_jd_ut_to_gregorian with recognized astronomical
 * epochs and time zone shifts. Relies on the caller respecting the positive JD
 * precondition.
 */
static lcca_bool test_lcca_convert_jd_ut_to_gregorian_nominal(void) {
    lcca_bool passed = true;
    lcca_gregorian_date date_result;

    /* Test Case 1: Standard J2000.0 Epoch */
    /* JD 2451545.0 corresponds to January 1.5, 2000, UT (noon). */
    date_result = lcca_convert_jd_ut_to_gregorian(2451545.0, 0.0);

    if (!lcca_c_assert(date_result.year == 2000)) {
        passed = false;
    }
    if (!lcca_c_assert(date_result.month == 1)) {
        passed = false;
    }
    if (!lcca_c_assert(date_result.day == 1)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(date_result.time_zone, 0.0))) {
        passed = false;
    }

    /* Test Case 2: Launch of Sputnik 1 */
    /* JD 2436116.3115046 corresponds to October 4, 1957, UT. */
    date_result = lcca_convert_jd_ut_to_gregorian(2436116.3115046, 0.0);

    if (!lcca_c_assert(date_result.year == 1957)) {
        passed = false;
    }
    if (!lcca_c_assert(date_result.month == 10)) {
        passed = false;
    }
    if (!lcca_c_assert(date_result.day == 4)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(date_result.time_zone, 0.0))) {
        passed = false;
    }

    /* Test Case 3: Time Zone Shift across midnight */
    /* JD 2451544.5 is exactly midnight January 1, 2000 UT. */
    /* In a -5.0 time zone (e.g., EST), this shifts back to 19:00 on December
     * 31, 1999. */
    date_result = lcca_convert_jd_ut_to_gregorian(2451544.5, -5.0);

    if (!lcca_c_assert(date_result.year == 1999)) {
        passed = false;
    }
    if (!lcca_c_assert(date_result.month == 12)) {
        passed = false;
    }
    if (!lcca_c_assert(date_result.day == 31)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(date_result.time_zone, -5.0))) {
        passed = false;
    }

    /* Test Case 4: Time Zone Shift crossing into a leap day */
    /* JD 2453065.5 is midnight March 1, 2004 UT (a leap year). */
    /* Shifting backward by 4 hours (-4.0) should land on February 29, 2004. */
    date_result = lcca_convert_jd_ut_to_gregorian(2453065.5, -4.0);

    if (!lcca_c_assert(date_result.year == 2004)) {
        passed = false;
    }
    if (!lcca_c_assert(date_result.month == 2)) {
        passed = false;
    }
    if (!lcca_c_assert(date_result.day == 29)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(date_result.time_zone, -4.0))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_convert_jd_ut_to_time_of_day with recognized astronomical
 * times and time zone shifts. Relies on the caller respecting the positive JD
 * precondition.
 */
static lcca_bool test_lcca_convert_jd_ut_to_time_of_day_nominal(void) {
    lcca_bool passed = true;
    lcca_time time_result;

    /* Test Case 1: Standard J2000.0 Epoch */
    /* JD 2451545.0 corresponds to January 1.5, 2000, UT (exactly 12:00:00.0).
     */
    time_result = lcca_convert_jd_ut_to_time_of_day(2451545.0, 0.0);

    if (!lcca_c_assert(time_result.hours == 12)) {
        passed = false;
    }
    if (!lcca_c_assert(time_result.minutes == 0)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(time_result.seconds, 0.0))) {
        passed = false;
    }

    /* Test Case 2: Launch of Sputnik 1 */
    /* JD 2436116.3115046296 */
    time_result = lcca_convert_jd_ut_to_time_of_day(2436116.3115046296, 0.0);

    if (!lcca_c_assert(time_result.hours == 19)) {
        passed = false;
    }
    if (!lcca_c_assert(time_result.minutes == 28)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(time_result.seconds, 34.0))) {
        passed = false;
    }

    /* Test Case 3: Time Zone Shift backward crossing midnight */
    /* JD 2451544.5 is exactly midnight January 1, 2000 UT (00:00:00.0). */
    /* In a -5.0 time zone (e.g., EST), this shifts backward to 19:00:00.0 of
     * the previous day. */
    time_result = lcca_convert_jd_ut_to_time_of_day(2451544.5, -5.0);

    if (!lcca_c_assert(time_result.hours == 19)) {
        passed = false;
    }
    if (!lcca_c_assert(time_result.minutes == 0)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(time_result.seconds, 0.0))) {
        passed = false;
    }

    /* Test Case 4: Time Zone Shift forward crossing midnight */
    /* JD 2451545.0 is noon UT. Shifting +14.0 hours puts local time at
     * 02:00:00.0 the next day. */
    time_result = lcca_convert_jd_ut_to_time_of_day(2451545.0, 14.0);

    if (!lcca_c_assert(time_result.hours == 2)) {
        passed = false;
    }
    if (!lcca_c_assert(time_result.minutes == 0)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(time_result.seconds, 0.0))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_get_delta_t_seconds with the dedicated epsilon.
 */
static lcca_bool test_lcca_get_delta_t_seconds_nominal(void) {
    lcca_bool passed = true;
    lcca_f64 delta_t;

    /* Benchmark for J2000.0 (JD 2451545.0) using the accepted polynomial */
    delta_t = lcca_get_delta_t_seconds(2451545.0);

    if (!lcca_c_assert(IS_EQUAL_DELTA_T(delta_t, 102.32))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_get_delta_t_seconds_td.
 * Uses historically validated values for Delta T with the dedicated epsilon.
 */
static lcca_bool test_lcca_get_delta_t_seconds_td_nominal(void) {
    lcca_bool passed = true;
    lcca_f64 delta_t;

    /* Test Case: Delta T for J2000.0 (JD 2451545.0) */
    /* Benchmarked against the polynomial expansion being utilized. */
    delta_t = lcca_get_delta_t_seconds_td(2451545.0);

    if (!lcca_c_assert(IS_EQUAL_DELTA_T(delta_t, 102.32))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_convert_gregorian_to_lunar with valid non-leap Gregorian
 * dates. Reference values are confirmed outputs of the TypeScript
 * implementation.
 */
static lcca_bool test_lcca_convert_gregorian_to_lunar_nominal(void) {
    lcca_bool passed = true;
    lcca_gregorian_date date_test;
    lcca_lunar_date result;

    /* Test Case 1: July 19th, 2024, +07:00 */
    /* Expected: year 2024, month 6 (not leap), day 14, monthSize 29 */
    date_test.time_zone = 7.0;
    date_test.year = 2024;
    date_test.month = 7;
    date_test.day = 19;
    result = lcca_convert_gregorian_to_lunar(date_test);
    if (!lcca_c_assert(result.year == 2024)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month == 6)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month_size == 29)) {
        passed = false;
    }
    if (!lcca_c_assert(result.day == 14)) {
        passed = false;
    }
    if (!lcca_c_assert(result.leap == false)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(result.time_zone, 7.0))) {
        passed = false;
    }

    /* Test Case 2: December 31st, 2010, +07:00 */
    /* Expected: year 2010, month 11 (not leap), day 26, monthSize 29 */
    /* Exercises a Gregorian year-end date mapping to a mid-lunar-year month */
    date_test.time_zone = 7.0;
    date_test.year = 2010;
    date_test.month = 12;
    date_test.day = 31;
    result = lcca_convert_gregorian_to_lunar(date_test);
    if (!lcca_c_assert(result.year == 2010)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month == 11)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month_size == 29)) {
        passed = false;
    }
    if (!lcca_c_assert(result.day == 26)) {
        passed = false;
    }
    if (!lcca_c_assert(result.leap == false)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(result.time_zone, 7.0))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_convert_gregorian_to_lunar specifically for leap month
 * detection. Both dates fall in the same Gregorian year (2025), which contains
 * a leap month 6. The first date falls within that leap month; the second
 * does not. This verifies that the leap flag is per-month, not per-year.
 */
static lcca_bool test_lcca_convert_gregorian_to_lunar_leap_month(void) {
    lcca_bool passed = true;
    lcca_gregorian_date date_test;
    lcca_lunar_date result;

    /* Test Case 1: August 11th, 2025, +07:00 */
    /* Expected: year 2025, month 6 (leap), day 18, monthSize 29 */
    date_test.time_zone = 7.0;
    date_test.year = 2025;
    date_test.month = 8;
    date_test.day = 11;
    result = lcca_convert_gregorian_to_lunar(date_test);
    if (!lcca_c_assert(result.year == 2025)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month == 6)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month_size == 29)) {
        passed = false;
    }
    if (!lcca_c_assert(result.day == 18)) {
        passed = false;
    }
    if (!lcca_c_assert(result.leap == true)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(result.time_zone, 7.0))) {
        passed = false;
    }

    /* Test Case 2: August 31st, 2025, +08:00 */
    /* Expected: year 2025, month 7 (not leap), day 9, monthSize 30 */
    /* Same Gregorian year as above; confirms leap is per-month, not per-year */
    /* Also the only case across all g2l tests with monthSize 30 */
    date_test.time_zone = 8.0;
    date_test.year = 2025;
    date_test.month = 8;
    date_test.day = 31;
    result = lcca_convert_gregorian_to_lunar(date_test);
    if (!lcca_c_assert(result.year == 2025)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month == 7)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month_size == 30)) {
        passed = false;
    }
    if (!lcca_c_assert(result.day == 9)) {
        passed = false;
    }
    if (!lcca_c_assert(result.leap == false)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(result.time_zone, 8.0))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_convert_lunar_to_gregorian with non-leap inputs using a
 * roundtrip approach.
 *
 * Direct construction of lcca_lunar_date is not feasible here because the k
 * field (lunation number) must be valid and is not independently derivable
 * by a caller. Roundtrip testing (g2l then l2g) provides guaranteed-valid
 * input structs and verifies the inverse relationship. Oracle values correspond
 * to those used in test_lcca_convert_gregorian_to_lunar_nominal.
 */
static lcca_bool test_lcca_convert_lunar_to_gregorian_nominal(void) {
    lcca_bool passed = true;
    lcca_gregorian_date gregorian_input;
    lcca_lunar_date lunar;
    lcca_gregorian_date result;

    /* Test Case 1: July 19th, 2024, +07:00 */
    gregorian_input.time_zone = 7.0;
    gregorian_input.year = 2024;
    gregorian_input.month = 7;
    gregorian_input.day = 19;
    lunar = lcca_convert_gregorian_to_lunar(gregorian_input);
    result = lcca_convert_lunar_to_gregorian(lunar);
    if (!lcca_c_assert(result.year == 2024)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month == 7)) {
        passed = false;
    }
    if (!lcca_c_assert(result.day == 19)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(result.time_zone, 7.0))) {
        passed = false;
    }

    /* Test Case 2: December 31st, 2010, +07:00 */
    gregorian_input.time_zone = 7.0;
    gregorian_input.year = 2010;
    gregorian_input.month = 12;
    gregorian_input.day = 31;
    lunar = lcca_convert_gregorian_to_lunar(gregorian_input);
    result = lcca_convert_lunar_to_gregorian(lunar);
    if (!lcca_c_assert(result.year == 2010)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month == 12)) {
        passed = false;
    }
    if (!lcca_c_assert(result.day == 31)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(result.time_zone, 7.0))) {
        passed = false;
    }

    return passed;
}

/**
 * @brief Tests lcca_convert_lunar_to_gregorian for leap month inputs using
 * a roundtrip approach.
 *
 * Both cases use Gregorian dates from 2025, which contains a leap month 6.
 * The first input falls within that leap month (leap=true); the second does
 * not (leap=false, month 7). This mirrors the structure of
 * test_lcca_convert_gregorian_to_lunar_leap_month and verifies that l2g
 * correctly distinguishes a leap month 6 from the following month 7 in the
 * same lunar year.
 */
static lcca_bool test_lcca_convert_lunar_to_gregorian_leap_month(void) {
    lcca_bool passed = true;
    lcca_gregorian_date gregorian_input;
    lcca_lunar_date lunar;
    lcca_gregorian_date result;

    /* Test Case 1: August 11th, 2025, +07:00 */
    /* Lunar intermediate: year 2025, month 6 (leap), day 18 */
    gregorian_input.time_zone = 7.0;
    gregorian_input.year = 2025;
    gregorian_input.month = 8;
    gregorian_input.day = 11;
    lunar = lcca_convert_gregorian_to_lunar(gregorian_input);
    result = lcca_convert_lunar_to_gregorian(lunar);
    if (!lcca_c_assert(result.year == 2025)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month == 8)) {
        passed = false;
    }
    if (!lcca_c_assert(result.day == 11)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(result.time_zone, 7.0))) {
        passed = false;
    }

    /* Test Case 2: August 31st, 2025, +08:00 */
    /* Lunar intermediate: year 2025, month 7 (not leap), day 9 */
    /* Same year as above; confirms l2g distinguishes leap from non-leap */
    gregorian_input.time_zone = 8.0;
    gregorian_input.year = 2025;
    gregorian_input.month = 8;
    gregorian_input.day = 31;
    lunar = lcca_convert_gregorian_to_lunar(gregorian_input);
    result = lcca_convert_lunar_to_gregorian(lunar);
    if (!lcca_c_assert(result.year == 2025)) {
        passed = false;
    }
    if (!lcca_c_assert(result.month == 8)) {
        passed = false;
    }
    if (!lcca_c_assert(result.day == 31)) {
        passed = false;
    }
    if (!lcca_c_assert(IS_EQUAL_F64(result.time_zone, 8.0))) {
        passed = false;
    }

    return passed;
}

int main(void) {
    lcca_bool tests_passed = true;

    printf("=== Starting test_calendar ===\n");

    /* Execute lcca_new_midnight_time test suite */
    RUN_TEST(test_lcca_new_midnight_time_values);
    RUN_TEST(test_lcca_new_midnight_time_validity);

    /* Execute lcca_is_valid_time_of_day test suite */
    RUN_TEST(test_lcca_is_valid_time_of_day_nominal);
    RUN_TEST(test_lcca_is_valid_time_of_day_off_nominal);

    /* Execute lcca_is_valid_gregorian_date test suite */
    RUN_TEST(test_lcca_is_valid_gregorian_date_nominal);
    RUN_TEST(test_lcca_is_valid_gregorian_date_off_nominal);

    /* Execute lcca_get_gregorian_month_size test suite */
    RUN_TEST(test_lcca_get_gregorian_month_size_nominal);
    RUN_TEST(test_lcca_get_gregorian_month_size_violation);

    /* Execute lcca_convert_gregorian_to_jd_ut test suite */
    RUN_TEST(test_lcca_convert_gregorian_to_jd_ut_nominal);

    /* Execute lcca_convert_jd_ut_to_gregorian test suite */
    RUN_TEST(test_lcca_convert_jd_ut_to_gregorian_nominal);

    /* Execute lcca_convert_jd_ut_to_time_of_day test suite */
    RUN_TEST(test_lcca_convert_jd_ut_to_time_of_day_nominal);

    /* Execute lcca_get_delta_t_seconds test suite */
    RUN_TEST(test_lcca_get_delta_t_seconds_nominal);

    /* Execute lcca_get_delta_t_seconds_td test suite */
    RUN_TEST(test_lcca_get_delta_t_seconds_td_nominal);

    /* Execute lcca_convert_gregorian_to_lunar test suite */
    RUN_TEST(test_lcca_convert_gregorian_to_lunar_nominal);
    RUN_TEST(test_lcca_convert_gregorian_to_lunar_leap_month);

    /* Execute lcca_convert_lunar_to_gregorian test suite */
    RUN_TEST(test_lcca_convert_lunar_to_gregorian_nominal);
    RUN_TEST(test_lcca_convert_lunar_to_gregorian_leap_month);

    printf("=== Test Suite Finished ===\n");

    return tests_passed ? 0 : 1;
}
