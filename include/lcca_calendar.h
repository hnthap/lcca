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
 * For timezone offset validation, we employ a practical arithmetic bound of -24
 * to +24 hours rather than restricting to current geopolitical realities or
 * leaving the input unbounded. Restricting validation to the current real-world
 * range of -12 to +14 hours ties correctness to temporary political decisions,
 * risking rejection of valid inputs if new timezones (such as UTC+15) are ever
 * established. Conversely, leaving the input unbounded fails to catch obvious
 * bugs, such as passing an offset of 300, because timezone offsets, unlike
 * angles, have no meaningful normalization operation. The -24 to +24 range
 * provides a stable validation boundary that accepts all current and plausible
 * future timezones while rejecting gross input errors.
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

#ifndef LCCA_CALENDAR_H
#define LCCA_CALENDAR_H

#include "lcca_numeric.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a date in the Lunar calendar.
 */
typedef struct lcca_lunar_date {
    lcca_f64 time_zone; /**< Time zone offset in hours (e.g. +07:00 is 7.0) */
    lcca_i32 k;         /**< Lunation number (k), with k = 0 corresponding */
                        /**< approximately to the New Moon of 6 January 2000 */
    lcca_i32 year;      /**< Lunar year, in Gregorian year numbering; it is */
                        /**< the Gregorian year in which the lunar year's */
                        /**< Month 1 begins, so it may differ from the */
                        /**< Gregorian year of the same day */
    lcca_i8 month;      /**< Lunar month, from 1 to 12; a leap month repeats */
                        /**< the number of the month it follows and is */
                        /**< distinguished by the `leap` flag */
    lcca_i8 month_size; /**< Lunar month's number of days (29 or 30) */
    lcca_i8 day;        /**< Day of month, from 1 to 30 */
    lcca_bool leap;     /**< True if the date is in a leap month */
} lcca_lunar_date;

/**
 * @brief Represents a date in the Gregorian calendar.
 */
typedef struct lcca_gregorian_date {
    lcca_f64 time_zone; /**< Time zone offset in hours (e.g. +07:00 is 7.0) */
    lcca_i32 year;      /**< Gregorian year, in astronomical year numbering */
                        /**< (e.g. AD 2000 is 2000, 1 BC is 0, 2 BC is -1) */
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
 * @post The returned time evaluates as valid under lcca_is_valid_time_of_day().
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
 * @brief Checks whether a date in the Gregorian calendar is valid. Regarding
 * timezone offset validation, consult the file-level documentation of
 * `lcca_calendar.h`.
 *
 * @post This function has no side effects.
 *
 * @param[in] gregorian A date in the Gregorian calendar
 *
 * @returns true if the Gregorian date is valid, otherwise false.
 */
lcca_bool lcca_is_valid_gregorian_date(const lcca_gregorian_date gregorian);

/**
 * @brief Gets the number of days in the specified month of a year.
 *
 * Only the year and month fields of the input are read; the day and time zone
 * fields are ignored. Every year value is accepted, February's length being
 * determined by the Gregorian leap-year rule applied to astronomical year
 * numbering.
 *
 * @pre The month of the input Gregorian date shall be in the range 1 to 12.
 *
 * @post This function has no side effects, but it also inherits the side
 * effects of lcca_c_assert() (if any) in case of precondition violation.
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
 * The time zone offset carried by @p gregorian is subtracted from the time of
 * day, so the result is a JD in UT rather than in local time.
 *
 * @pre The input Gregorian date is valid.
 *
 * @pre The input time of day is valid.
 *
 * @post This function has no side effects. It performs no assertion of its own;
 * a precondition violation yields an unspecified JD.
 *
 * @param[in] gregorian The Gregorian date, whose time zone offset is applied
 *
 * @param[in] time The time of day, interpreted in that time zone
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
 * @pre The time zone offset shall be valid (greater than -24 and less than 24).
 *
 * @post This function has no side effects. It performs no assertion of its own;
 * a precondition violation yields an unspecified Gregorian date.
 *
 * @param[in] jd_ut Positive JD
 *
 * @param[in] time_zone Time zone offset in hours, copied into the result
 *
 * @returns The Gregorian date, with its time zone field set to @p time_zone
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
 * @pre The time zone offset shall be valid (greater than -24 and less than 24).
 *
 * @post This function has no side effects. It performs no assertion of its own;
 * a precondition violation yields an unspecified time of day.
 *
 * @param[in] jd_ut Positive JD
 *
 * @param[in] time_zone Time zone offset in hours
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
 * @returns Delta T (TD - UT) in seconds
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
 * The argument is a JD in TD, whereas the underlying estimate is parameterized
 * by JD in UT; the corresponding UT instant is therefore recovered by fixed-
 * point iteration before the estimate is evaluated.
 *
 * @param[in] jd_td JD (TD)
 *
 * @returns Delta T (TD - UT) in seconds
 */
lcca_f64 lcca_get_delta_t_seconds_td(const lcca_f64 jd_td);

/**
 * @brief Converts a date in the Gregorian calendar to lunar calendar.
 *
 * The conversion is performed for the civil day denoted by @p gregorian; any
 * time of day is disregarded, midnight in the date's own time zone being used
 * throughout. The time zone offset of @p gregorian is copied to the result, and
 * all of the result's fields (k, year, month, month_size, day, leap) are set.
 *
 * @pre The input Gregorian date shall be valid.
 *
 * @post This function has no side effects, but it also inherits the side
 * effects of lcca_c_assert() (if any) in case of precondition violation.
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
 * The month begun by lunation @p lunar.k is located, and @p lunar.day counted
 * from its first day. Only the `k`, `day`, and `time_zone` fields are read; the
 * `year`, `month`, `month_size`, and `leap` fields are not consulted, since `k`
 * already identifies the month unambiguously. A lunar date whose `k` disagrees
 * with its `month`, `year`, and `leap` fields therefore converts according to
 * `k`.
 *
 * @pre The input lunar date shall be valid.
 *
 * @post This function has no side effects, but it also inherits the side
 * effects of lcca_c_assert() (if any) in case of precondition violation. The
 * asserted checks cover only the basic bounds on `month`, `month_size`, and
 * `day` described in the note below.
 *
 * @param[in] lunar The lunar date
 *
 * @returns The equivalent Gregorian date, with its time zone field set to
 * @p lunar.time_zone
 *
 * @note The only sane way to validate a lunar date as we know is to convert it
 * to Gregorian, then convert it back to lunar, and then compare the new lunar
 * date with the original, where the difference indicates invalidity. That would
 * introduce an infinite loop (because the first step is actually this
 * function), and "assertions shall be used to perform basic sanity checks
 * throughout the code," whereas this validation is not basic. Therefore, we
 * will skip this assertion. Lunar date validation is only performed by applying
 * basic sanity bounds.
 */
lcca_gregorian_date
lcca_convert_lunar_to_gregorian(const lcca_lunar_date lunar);

#ifdef __cplusplus
}
#endif

#endif /* LCCA_CALENDAR_H */
