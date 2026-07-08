#include "lcca_calendar.h"
#include "lcca_common.h"
#include "lcca_mechanics.h"
#include "lcca_numeric.h"
#include <float.h>
#include <math.h>

lcca_time lcca_new_midnight_time(void) {
    lcca_time time;
    time.hours = 0;
    time.minutes = 0;
    time.seconds = 0;
    return time;
}

lcca_bool lcca_is_valid_time_of_day(const lcca_time time) {
    return ((((time.hours >= 0) && (time.hours < 24)) &&
             ((time.minutes >= 0) && (time.minutes < 60))) &&
            ((time.seconds >= 0) && (time.seconds < 60)));
}

lcca_bool lcca_is_valid_gregorian_date(const lcca_gregorian_date gregorian) {
    return ((((gregorian.month >= 1) && (gregorian.month <= 12)) &&
             ((gregorian.day >= 1) &&
              (gregorian.day <= lcca_get_gregorian_month_size(gregorian)))) &&
            ((gregorian.time_zone <= 14.0) && (gregorian.time_zone >= -12.0)));
}

lcca_i8 lcca_get_gregorian_month_size(const lcca_gregorian_date gregorian) {
    lcca_i8 month_size = 31;
    if ((((gregorian.month == 4) || (gregorian.month == 6)) ||
         (gregorian.month == 9)) ||
        (gregorian.month == 11)) {
        month_size = 30;
    } else if (gregorian.month == 2) {
        month_size =
            ((gregorian.year % 400 == 0) ||
             ((gregorian.year % 4 == 0) && (gregorian.year % 100 != 0)))
                ? 29
                : 28;
    }
    if (!lcca_c_assert((gregorian.month >= 1) && (gregorian.month <= 12))) {
        return -1;
    }
    return month_size;
}

lcca_f64 lcca_convert_gregorian_to_jd_ut(const lcca_gregorian_date gregorian,
                                         const lcca_time time) {
    lcca_i8 m = gregorian.month;
    lcca_i32 y = gregorian.year;
    lcca_i32 A, B;
    const lcca_f64 d =
        gregorian.day + (((time.hours - gregorian.time_zone) +
                          ((time.minutes + (time.seconds / 60.0)) / 60.0)) /
                         24.0);

    if (m < 3) {
        y = y - 1;
        m = m + 12;
    }
    A = y / 100;
    B = 2 - A + (A / 4);

    return ((floor(365.25 * (y + 4716)) + floor(30.6001 * (m + 1))) +
            ((d + B) - 1524.5));
}

lcca_gregorian_date lcca_convert_jd_ut_to_gregorian(const lcca_f64 jd_ut,
                                                    const lcca_f64 time_zone) {
    const lcca_f64 jd = jd_ut + (time_zone / 24.0);
    const lcca_f64 Z = floor(jd + 0.5);
    const lcca_f64 F = (jd + 0.5) - Z;
    const lcca_f64 alpha = floor((Z - 1867216.25) / 36524.25);
    const lcca_f64 A = ((Z + 1) + alpha) - floor(alpha / 4.0);
    const lcca_f64 B = A + 1524;
    const lcca_f64 C = floor((B - 122.1) / 365.25);
    const lcca_f64 D = floor(365.25 * C);
    const lcca_f64 E = floor((B - D) / 30.6001);

    const lcca_i8 month = (lcca_i8)((E < 14) ? (E - 1) : (E - 13));
    const lcca_i32 year = (lcca_i32)((month > 2) ? (C - 4716) : (C - 4715));
    const lcca_f64 t = ((B - D) - floor(30.6001 * E)) + F;
    const lcca_f64 day = floor(t);

    lcca_gregorian_date gregorian;
    gregorian.year = year;
    gregorian.month = month;
    gregorian.day = (lcca_i8)day;
    gregorian.time_zone = time_zone;

    return gregorian;
}

lcca_time lcca_convert_jd_ut_to_time_of_day(const lcca_f64 jd_ut,
                                            const lcca_f64 time_zone) {
    const lcca_f64 jd_fraction = jd_ut - floor(jd_ut);
    const lcca_f64 shifted_fraction =
        (jd_fraction + ((time_zone / 24.0) + 0.5));
    const lcca_f64 fraction = (shifted_fraction - floor(shifted_fraction));

    const lcca_f64 h = fraction * 24.0;
    const lcca_f64 hours = floor(h);

    const lcca_f64 m = (h - hours) * 60.0;
    const lcca_f64 minutes = floor(m);

    const lcca_f64 seconds = (m - minutes) * 60.0;

    lcca_time time;
    time.hours = (lcca_i8)hours;
    time.minutes = (lcca_i8)minutes;
    time.seconds = seconds;
    return time;
}

lcca_f64 lcca_get_delta_t_seconds(const lcca_f64 jd_ut) {
    return -15.0 + (((jd_ut - 2382148.0) * (jd_ut - 2382148.0)) / 41048480.0);
}

lcca_f64 lcca_get_delta_t_seconds_td(const lcca_f64 jd_td) {
    lcca_f64 ut = jd_td;
    lcca_f64 previous_ut = 0.0;
    lcca_i8 i;
    for (i = 0; (i < 100) && (fabs(ut - previous_ut) > 1e-9); i += 1) {
        previous_ut = ut;
        ut = jd_td - (lcca_get_delta_t_seconds(ut) / 86400.0);
    }
    return lcca_get_delta_t_seconds(ut);
}

lcca_lunar_date
lcca_convert_gregorian_to_lunar(const lcca_gregorian_date gregorian) {
    const lcca_i32 year = gregorian.year;
    const lcca_f64 time_zone = gregorian.time_zone;
    lcca_lunar_date lunar;

    (void)lcca_c_assert(lcca_is_valid_gregorian_date(gregorian));

    {
        const lcca_time midnight = lcca_new_midnight_time();
        const lcca_f64 gregorian_jd_ut =
            lcca_convert_gregorian_to_jd_ut(gregorian, midnight);
        const lcca_f64 target_k_approx =
            floor(lcca_approximate_k(gregorian, midnight) + 0.5);
        lunar.k = (lcca_i32)target_k_approx;

        {
            lcca_f64 day_one_jd_ut =
                lcca_get_new_moon_midnight_jd_ut(lunar.k, time_zone);
            lcca_f64 next_day_one_jd_ut =
                lcca_get_new_moon_midnight_jd_ut(lunar.k + 1, time_zone);

            if (gregorian_jd_ut < day_one_jd_ut) {
                lunar.k -= 1;
                next_day_one_jd_ut = day_one_jd_ut;
                day_one_jd_ut =
                    lcca_get_new_moon_midnight_jd_ut(lunar.k, time_zone);
            } else if (gregorian_jd_ut >= next_day_one_jd_ut) {
                lunar.k += 1;
                day_one_jd_ut = next_day_one_jd_ut;
                next_day_one_jd_ut =
                    lcca_get_new_moon_midnight_jd_ut(lunar.k + 1, time_zone);
            }
            lunar.month_size = (lcca_i8)(next_day_one_jd_ut - day_one_jd_ut);
            lunar.day = (lcca_i8)((gregorian_jd_ut - day_one_jd_ut) + 1.0);
        }
    }
    {
        lcca_i32 start_month_11_k;
        lcca_i32 end_month_11_k;
        lcca_i32 leap_month_k;
        lcca_i32 base_year;
        {
            lcca_i32 this_month_11_k = lcca_get_k_of_month_11(year, time_zone);

            if (lunar.k >= this_month_11_k) {
                start_month_11_k = this_month_11_k;
                end_month_11_k = lcca_get_k_of_month_11(year + 1, time_zone);
                base_year = year;
            } else {
                start_month_11_k = lcca_get_k_of_month_11(year - 1, time_zone);
                end_month_11_k = this_month_11_k;
                base_year = year - 1;
            }
        }
        {
            const lcca_leap_month_result result = lcca_get_k_of_leap_month(
                start_month_11_k, end_month_11_k, time_zone);
            leap_month_k =
                result.have_leap_month ? result.k : (start_month_11_k - 1);
        }
        lunar.month = 11;
        lunar.leap = false;
        lunar.year = base_year;
        {
            lcca_i32 i;
            lcca_i32 k;
            for (i = 0, k = start_month_11_k + 1; (i < 100) && (k <= lunar.k);
                 i += 1, k += 1) {
                if (k == leap_month_k) {
                    lunar.leap = true;
                } else {
                    lunar.leap = false;
                    lunar.month += 1;
                    if (lunar.month == 13) {
                        lunar.month = 1;
                        lunar.year += 1;
                    }
                }
            }
        }
    }
    lunar.time_zone = time_zone;
    return lunar;
}

lcca_gregorian_date
lcca_convert_lunar_to_gregorian(const lcca_lunar_date lunar) {
    /**
     * The only sane way to validate a lunar date as we know is to convert it to
     * Gregorian, then convert it back to lunar, and then compare the new lunar
     * date with the original, where the difference indicates invalidity.
     *
     * That would introduce an infinite loop (because the first step is actually
     * this function), and "assertions shall be used to perform basic sanity
     * checks throughout the code," whereas this validation is not basic.
     * Therefore, we will skip this assertion.
     *
     * Validation is only performed by applying basic sanity bounds.
     */
    (void)lcca_c_assert(
        (((lunar.month >= 1) && (lunar.month <= 12)) &&
         ((lunar.month_size == 29) || (lunar.month_size == 30))) &&
        ((lunar.day >= 1) && (lunar.day <= 30)));
    {
        const lcca_f64 td = lcca_get_new_moon_jd_td(lunar.k);
        const lcca_f64 ut = td - (lcca_get_delta_t_seconds_td(td) / 86400.0);
        const lcca_gregorian_date new_moon_gregorian =
            lcca_convert_jd_ut_to_gregorian(ut, lunar.time_zone);
        const lcca_f64 midnight_jd_ut = lcca_convert_gregorian_to_jd_ut(
            new_moon_gregorian, lcca_new_midnight_time());
        const lcca_f64 target_jd_ut = midnight_jd_ut + (lunar.day - 1);
        return lcca_convert_jd_ut_to_gregorian(target_jd_ut, lunar.time_zone);
    }
}
