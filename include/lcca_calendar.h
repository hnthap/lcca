/**
 * @file lcca_calendar.h
 * @brief Calendar systems, Julian Day, time-of-day, and lunar calendar
 *        conversion API.
 *
 * This header provides the public calendar facilities of the LCCA library.
 * It defines fundamental date and time structures together with functions for
 * validation, calendar arithmetic, Julian Day (JD) conversion, Delta T
 * estimation, and bidirectional conversion between the Gregorian and the
 * Vietnamese/Chinese lunisolar calendar.
 *
 * The API is designed as a stateless utility library:
 * - All functions are pure computations and perform no dynamic memory
 *   allocation.
 * - Calendar objects are passed and returned by value.
 * - All time zones are represented as decimal hour offsets from UTC
 *   (for example, +07:00 is represented as 7.0).
 *
 * ## Supported functionality
 *
 * - Representation of:
 *   - Gregorian calendar dates
 *   - Lunar calendar dates
 *   - Time of day with sub-second precision
 * - Gregorian date validation
 * - Gregorian month length calculation
 * - Conversion between Gregorian calendar and Julian Day (UT)
 * - Extraction of Gregorian date and time from Julian Day
 * - Delta T (TD − UT) estimation
 * - Conversion between Gregorian and lunisolar calendar dates
 *
 * ## Astronomical basis
 *
 * Julian Day conversion follows the astronomical algorithms described in:
 *
 * Jean Meeus,
 * Astronomical Algorithms,
 * Willmann-Bell, 1991.
 *
 * Delta T calculations are likewise based on the methods presented in the
 * same reference.
 *
 * Lunar calendar conversion is performed using astronomical new moon and solar
 * longitude calculations derived from the Julian Day representation.
 *
 * ## Calendar conventions
 *
 * Gregorian years use astronomical year numbering:
 *
 * - AD 1  -> 1
 * - 1 BC  -> 0
 * - 2 BC  -> -1
 *
 * Lunar dates contain additional metadata such as:
 * - lunation number
 * - leap-month flag
 * - lunar month length
 * - associated time zone
 *
 * ## Error handling
 *
 * Functions document their required preconditions individually.
 *
 * Unless otherwise specified:
 * - violating a documented precondition results in undefined behavior from the
 *   caller's perspective,
 * - implementations may invoke `lcca_c_assert()`,
 * - functions that explicitly document sentinel return values may return those
 *   values after assertion handling.
 *
 * ## Thread safety
 *
 * All functions are reentrant and thread-safe provided that the underlying
 * assertion implementation (if enabled) is thread-safe.
 *
 * ## Memory management
 *
 * This API performs no heap allocation. The caller owns all objects and no
 * cleanup is required.
 */

#ifndef LCCA_LUNAR_CALENDAR_H
#define LCCA_LUNAR_CALENDAR_H

#include "lcca_numeric.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a date in the Lunar calendar.
 */
typedef struct lcca_lunar_date {
    lcca_f64 time_zone; /**< Time zone offset in hours (e.g. +07:00 is 7.0) */
    lcca_i32 k;         /**< Lunation number (k), */
                        /**< starting from January 6th, 2000 */
    lcca_i32 year;      /**< Approximate Gregorian year of the lunar date */
    lcca_i8 month;      /**< Lunar month, from 1 to 12 */
    lcca_i8 month_size; /**< Lunar month's number of days (29 or 30) */
    lcca_i8 day;        /**< Day of month, from 1 to 30 */
    lcca_bool leap;     /**< True if the date is in a leap month */
} lcca_lunar_date;

/**
 * @brief Represents a date in the Gregorian calendar.
 */
typedef struct lcca_gregorian_date {
    lcca_f64 time_zone; /**< Time zone offset in hours (e.g. +07:00 is 7.0) */
    lcca_i32 year;      /**< Gregorian year */
                        /**< (e.g. AD 2000 is 2000, 1 BC is 0, 2 BC is 1) */
    lcca_i8 month;      /**< Gregorian month, */
                        /**< from 1 (January) to 12 (December) */
    lcca_i8 day;        /**< Day of month, from 1 to 31 */
} lcca_gregorian_date;

/**
 * @brief Represents a moment in a day, from 00:00 (inclusive) to 24:00
 * (exclusive) in 24-hour format, with sub-second precision.
 */
typedef struct lcca_time {
    lcca_f64 seconds; /**< Seconds, as a real number from 0 (inclusive) */
                      /**< to 60 (exclusive) */
    lcca_i8 hours;    /**< Hours, as an integer from 0 (inclusive) */
                      /**< to 24 (exclusive) */
    lcca_i8 minutes;  /**< Minutes, as an integer from 0 (inclusive) */
                      /**< to 60 (exclusive) */
} lcca_time;

/**
 * @brief Creates and initializes an lcca_time structure representing midnight.
 *
 * This function initializes a time-of-day object strictly to 00:00:00.000.
 * It allocates no dynamic memory and returns the initialized structure by
 * value.
 *
 * @post The returned time evaluates as valid under lcca_is_valid_time_of_day.
 *
 * @post This function has no side effects.
 *
 * @returns An lcca_time structure with hours, minutes, and seconds set to 0.
 */
lcca_time lcca_new_midnight_time(void);

/**
 * @brief Evaluates whether an lcca_time structure contains a valid time of day.
 *
 * A valid time of day falls strictly within the 24-hour clock constraints:
 * hours must be between 0 (inclusive) and 24 (exclusive), minutes between
 * 0 (inclusive) and 60 (exclusive), and seconds between 0.0 (inclusive)
 * and 60.0 (exclusive).
 *
 * @post This function has no side effects.
 *
 * @param[in] time The time structure to be validated.
 *
 * @returns true if the time is valid, otherwise false.
 */
lcca_bool lcca_is_valid_time_of_day(const lcca_time time);

/**
 * @brief Checks whether a date in the Gregorian calendar is valid.
 *
 * @post This function has no side effects.
 *
 * @param[in] gregorian A date in the Gregorian calendar
 *
 * @returns 1 if the Gregorian date is valid, otherwise, 0.
 */
lcca_bool lcca_is_valid_gregorian_date(const lcca_gregorian_date gregorian);

/**
 * @brief Gets the number of days in the specified month of a year.
 *
 * @pre The year and month of the input Gregorian date shall be valid.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert (if any) in case of precondition violation.
 *
 * @param[in] gregorian A date in the Gregorian calendar
 *
 * @returns The number of days in the month of the specified date.
 * Returns -1 if the input precondition is violated.
 */
lcca_i8 lcca_get_gregorian_month_size(const lcca_gregorian_date gregorian);

/**
 * @brief Converts a moment (date and time) in the Gregorian calendar to JD in
 * UT.
 *
 * See more: Chapter 7 of
 * Astronomical Algorithms (Jean Meeus, 1991).
 *
 * @pre The input Gregorian date is valid.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert (if any) in case of precondition violation.
 *
 * @param[in] gregorian The Gregorian date with time of day
 *
 * @returns The equivalent JD (UT)
 */
lcca_f64 lcca_convert_gregorian_to_jd_ut(const lcca_gregorian_date gregorian,
                                         const lcca_time time);

/**
 * @brief Converts a positive JD (UT) to the equivalent Gregorian date, with
 * respect to the specified time zone.
 *
 * See more: Chapter 7 of
 * Astronomical Algorithms (Jean Meeus, 1991).
 *
 * @pre The input JD is positive.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert (if any) in case of precondition violation.
 *
 * @param[in] jd_ut Positive JD
 *
 * @returns The Gregorian date
 */
lcca_gregorian_date lcca_convert_jd_ut_to_gregorian(const lcca_f64 jd_ut,
                                                    const lcca_f64 time_zone);

/**
 * @brief Converts a positive JD (UT) to the time of day (00:00:00.000 to before
 * 24:00:00.000), with respect to the specified time zone.
 *
 * See more: Chapter 7 of
 * Astronomical Algorithms (Jean Meeus, 1991).
 *
 * @pre The input JD is positive.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert (if any) in case of precondition violation.
 *
 * @param[in] jd_ut Positive JD
 *
 * @returns The time of day
 */
lcca_time lcca_convert_jd_ut_to_time_of_day(const lcca_f64 jd_ut,
                                            const lcca_f64 time_zone);

/**
 * @brief Calculates Delta T in seconds, i.e. the difference between Dynamic
 * Time (TD) and Universal Time (UT) for a given JD (UT).
 *
 * See more: Chapter 9 "Dynamical Time" of
 * Astronomical Algorithms (Jean Meeus, 1991).
 *
 * @post This function has no side effects.
 *
 * @param[in] jd_ut JD (UT)
 *
 * @returns Delta T
 */
lcca_f64 lcca_get_delta_t_seconds(const lcca_f64 jd_ut);

/**
 * @brief Calculates Delta T in seconds, i.e. the difference between Dynamic
 * Time (TD) and Universal Time (UT) for a given JD (TD).
 *
 * See more: Chapter 9 "Dynamical Time" of
 * Astronomical Algorithms (Jean Meeus, 1991).
 *
 * @post This function has no side effects.
 *
 * @param[in] jd_td JD (TD)
 *
 * @returns Delta T
 */
lcca_f64 lcca_get_delta_t_seconds_td(const lcca_f64 jd_td);

/**
 * @brief Converts a date in the Gregorian calendar to lunar calendar.
 *
 * @pre The input Gregorian date shall be valid.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert (if any) in case of precondition violation.
 *
 * @param[in] gregorian The Gregorian date
 *
 * @returns The equivalent lunar date
 */
lcca_lunar_date
lcca_convert_gregorian_to_lunar(const lcca_gregorian_date gregorian);

/**
 * @brief Converts a date in the lunar calendar to Gregorian calendar.
 *
 * @pre The input lunar date shall be valid.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert (if any) in case of precondition violation.
 *
 * @param[in] lunar The lunar date
 *
 * @returns The equivalent Gregorian date
 */
lcca_gregorian_date
lcca_convert_lunar_to_gregorian(const lcca_lunar_date lunar);

#ifdef __cplusplus
}
#endif

#endif /* LCCA_LUNAR_CALENDAR_H */
