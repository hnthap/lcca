/**
 * @file lcca_mechanics.h
 * @brief Astronomical mechanics and lunisolar calendar algorithms.
 *
 * This header provides the low-level astronomical algorithms that underpin the
 * calendar conversion facilities of the LCCA library. Unlike the higher-level
 * calendar API, which operates on Gregorian and lunar dates, this module
 * exposes the underlying celestial mechanics used to derive those dates from
 * first principles.
 *
 * The algorithms primarily implement the astronomical models described in
 * Jean Meeus' Astronomical Algorithms, including calculations for
 * solar coordinates, New Moon epochs, seasonal markers, and the relationship
 * between lunar months and the solar year.
 *
 * Most applications should use the higher-level conversion routines provided
 * by `lcca_calendar.h`. This header is intended primarily for library
 * implementation, advanced users, and applications requiring direct access to
 * intermediate astronomical quantities.
 *
 * ## Provided functionality
 *
 * - True geometric longitude of the Sun
 * - Precise New Moon calculations
 * - Lunation number estimation
 * - Winter Solstice determination
 * - Identification of Lunar Month 11
 * - Leap-month detection
 * - Conversion between lunation numbers and civil calendar boundaries
 *
 * ## Astronomical model
 *
 * The lunar calendar implemented by LCCA is fundamentally astronomical rather
 * than tabular. Calendar months are defined by successive astronomical New
 * Moons, while month numbering and leap-month determination are governed by
 * the apparent motion of the Sun along the ecliptic.
 *
 * Several concepts recur throughout this API:
 *
 * - **Julian Day (JD)** serves as the continuous time representation used by
 *   astronomical calculations.
 * - **Dynamic Time (TD)** is used for ephemeris calculations because celestial
 *   motion is modeled in a uniform time scale.
 * - **Universal Time (UT)** is used when mapping astronomical events to civil
 *   calendar dates.
 * - **Lunation number (k)** identifies successive New Moons, with
 *   `k = 0` corresponding approximately to the New Moon of
 *   6 January 2000.
 *
 * ## Leap-month determination
 *
 * A lunisolar calendar must occasionally insert an additional lunar month to
 * keep lunar months synchronized with the solar year.
 *
 * This module determines leap months according to the traditional East Asian
 * calendrical rule: a leap month is the first lunar month between consecutive
 * Month 11 boundaries that does not contain a Principal Solar Term. The helper
 * structures and functions provided by this header expose the intermediate
 * calculations required to identify that month.
 *
 * ## Intended usage
 *
 * Most public applications should not call these functions directly.
 * Instead, they serve as the computational foundation for:
 *
 * - Gregorian ↔ lunar calendar conversion
 * - Lunar calendar construction
 * - Astronomical event calculations
 * - Validation and testing of calendrical algorithms
 *
 * Direct use is appropriate when implementing alternative calendar systems,
 * performing astronomical analysis, or inspecting intermediate results that
 * are intentionally hidden by the higher-level calendar API.
 *
 * ## Thread safety
 *
 * All functions are deterministic numerical computations with no mutable
 * global state. They are fully reentrant and thread-safe.
 *
 * ## Memory management
 *
 * This API performs no dynamic memory allocation. All structures are returned
 * by value and require no explicit cleanup.
 *
 * ## References
 *
 * The astronomical algorithms implemented by this module are derived primarily
 * from:
 *
 * Jean Meeus,
 * Astronomical Algorithms,
 * Willmann-Bell, 1991.
 */

#ifndef LCCA_MECHANICS_H
#define LCCA_MECHANICS_H

#include "lcca_calendar.h"
#include "lcca_numeric.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The result of lcca_get_k_of_leap_month().
 */
typedef struct lcca_leap_month_result {
    lcca_bool have_leap_month; /**< Whether a leap month exists */
    lcca_i32 k;                /**< Lunation number (k) of the leap month
                                    if exists, undefined otherwise */
} lcca_leap_month_result;

/**
 * @brief   Calculates the true geometric longitude of the Sun for a given time
 *          in degrees.
 *
 * See more: Chapter 24 "Solar Coordinates" of
 * Astronomical Algorithms (Jean Meeus, 1991).
 *
 * @post This function has no side effects.
 *
 * @param[in] jd_td JD in Dynamic Time
 *
 * @returns The true geometric longitude of the Sun for the given time in
 *          degrees.
 */
lcca_f64 lcca_get_sun_true_longitude(const lcca_f64 jd_td);

/**
 * @brief   Calculates the exact JD in Dynamic Time of the k-th New Moon.
 *
 * See more: Chapter 47 "Phases of the Moon" of
 * Astronomical Algorithms (Jean Meeus, 1991).
 *
 * @post This function has no side effects.
 *
 * @param[in] k Lunation number (k=0 is roughly about AD 2000); it represents
 *              a New Moon as an integer.
 *
 * @returns The exact JD in Dynamic Time of the k-th New Moon.
 */
lcca_f64 lcca_get_new_moon_jd_td(const lcca_i32 k);

/**
 * @brief   Approximates the lunation number (k) given the provided *decimal*
 *          year.
 *
 * @pre The input Gregorian date is valid.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert() (if any) in case of precondition violation.
 *
 * @param[in] gregorian The Gregorian date with time zone
 *
 * @param[in] time The time of the day
 *
 * @returns The approximated lunation number (k)
 */
lcca_f64 lcca_approximate_k(const lcca_gregorian_date gregorian,
                            const lcca_time time);

/**
 * @brief Calculates the JD (TD) of the Winter Solstice of given Gregorian year
 * and time zone.
 *
 * @post This function has no side effects.
 *
 * @param[in] year The Gregorian year
 *
 * @returns JD (TD) of the Winter Solstice
 */
lcca_f64 lcca_get_winter_solstice_jd_td(const lcca_i32 year);

/**
 * @brief Calculates the Lunation number (k) of the New Moon that begins the
 * 11th lunar month (Month 11).
 *
 * @pre Time zone shall be valid. Regarding timezone offset validation, consult
 * the file-level documentation of `lcca_calendar.h`.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert() (if any) in case of precondition violation.
 *
 * @param[in] year The Gregorian year
 *
 * @param[in] time_zone Time zone offset in hours
 *
 * @returns Lunation number (k) of the New Moon that begins Month 11
 */
lcca_i32 lcca_get_k_of_month_11(const lcca_i32 year, const lcca_f64 time_zone);

/**
 * @brief Calculates the Lunation number (k) of the first month without a
 * Principal Solar Term in the lunar year from Month 11 to before the next
 * Month 11.
 *
 * @pre k of the "current" year's Month 11 shall be smaller than k of the "next"
 * year's Month 11.
 *
 * @pre Time zone shall be valid. Regarding timezone offset validation, consult
 * the file-level documentation of `lcca_calendar.h`.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert() (if any) in case of precondition violation.
 *
 * @param[in] current_year_month_11_k k of the "current" year's Month 11
 *
 * @param[in] next_year_month_11_k k of the "next" year's Month 11
 *
 * @param[in] time_zone Time zone offset in hours
 *
 * @returns k of the first month without a Principal Solar Term in the lunar
 * year from Month 11 to before the next Month 11.
 */
lcca_leap_month_result
lcca_get_k_of_leap_month(const lcca_i32 current_year_month_11_k,
                         const lcca_i32 next_year_month_11_k,
                         const lcca_f64 time_zone);

/**
 * @brief Calculate the JD (UT) of the midnight of the day which New Moon falls
 * into.
 *
 * @pre Time zone shall be valid. Regarding timezone offset validation, consult
 * the file-level documentation of `lcca_calendar.h`.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert() (if any) in case of precondition violation.
 *
 * @param[in] k Lunation number (k) of the New Moon in question
 *
 * @param[in] time_zone Time zone offset in hours
 *
 * @returns JD (UT) of the midnight of the day which New Moon falls into
 */
lcca_f64 lcca_get_new_moon_midnight_jd_ut(const lcca_i32 k,
                                          const lcca_f64 time_zone);

#ifdef __cplusplus
}
#endif

#endif /* LCCA_MECHANICS_H */
