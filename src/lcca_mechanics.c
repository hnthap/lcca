#include <math.h>
#include "lcca_calendar.h"
#include "lcca_common.h"
#include "lcca_math.h"
#include "lcca_mechanics.h"
#include "lcca_numeric.h"

lcca_f64 lcca_get_sun_true_longitude(const lcca_f64 jd_td) {
    /* T: Julian centuries from the epoch J2000.0. */
    const lcca_f64 T = (jd_td - 2451545) / 36525;
    const lcca_f64 T2 = T * T;
    /* L0: The geometric mean longitude of the Sun in degrees. */
    const lcca_f64 L0 = (280.46645 + (36000.76983 * T)) + (0.0003032 * T2);
    /* M: The mean anomaly of the Sun in degrees. */
    const lcca_f64 M = ((357.52910 + (35999.05030 * T)) + (0.0001559 * T2)) -
                       (0.00000048 * T2 * T);
    /* C: Center C of the Sun in degrees. */
    const lcca_f64 C =
        (((((1.914600 - (0.004817 * T)) - (0.000014 * T2)) *
           lcca_sin_degrees(M)) +
          ((0.019993 - (0.000101 * T)) * lcca_sin_degrees(2 * M))) +
         (0.000290 * lcca_sin_degrees(3 * M)));
    /* Theta: The Sun's true longitude in degrees. */
    return lcca_normalize_degrees(L0 + C);
}

lcca_f64 lcca_get_new_moon_jd_td(const lcca_i32 k) {
    /** T: Julian centuries from the epoch J2000.0. */
    const lcca_f64 T = k / 1236.85;
    const lcca_f64 T2 = T * T;
    const lcca_f64 T3 = T2 * T;
    const lcca_f64 T4 = T3 * T;
    /** E: Eccentricity of the Earth's orbit around the Sun */
    const lcca_f64 E = 1 - (0.002516 * T) - (0.0000074 * T2);
    /** JDE: The mean time of the phase */
    const lcca_f64 JDE =
        ((((2451550.09765 + (29.530588853 * k)) + (0.0001337 * T2)) -
          (0.000000150 * T3)) +
         (0.00000000073 * T4));
    /** M: Sun's mean anomaly at time JDE */
    const lcca_f64 M =
        (((2.5534 + (29.10535669 * k)) - (0.0000218 * T2)) - (0.00000011 * T3));
    /** M': Moon's mean anomaly at time JDE */
    const lcca_f64 MPrime =
        ((((201.5643 + (385.81693528 * k)) + (0.0107438 * T2)) +
          (0.00001239 * T3)) -
         (0.000000058 * T4));
    /** F: Moon's argument of latitude */
    const lcca_f64 F = ((((160.7108 + (390.67050274 * k)) - (0.0016341 * T2)) -
                         (0.00000227 * T3)) +
                        (0.000000011 * T4));
    /** Omega: Longitude of the ascending node of the lunar orbit */
    const lcca_f64 Omega = (((124.7746 - (1.56375580 * k)) + (0.0020691 * T2)) +
                            (0.00000215 * T3));
    const lcca_f64 corrections =
        (((((((((((((((((((((((((((((((((((((((0 - (0.40720 *
                                                    lcca_sin_degrees(MPrime))) +
                                              ((0.17241 * E) *
                                               lcca_sin_degrees(M))) +
                                             (0.01608 *
                                              lcca_sin_degrees(2 * MPrime))) +
                                            (0.01039 *
                                             lcca_sin_degrees(2 * F))) +
                                           ((0.00739 * E) *
                                            lcca_sin_degrees(MPrime - M))) -
                                          ((0.00514 * E) *
                                           lcca_sin_degrees(MPrime + M))) +
                                         (((0.00208 * E) * E) *
                                          lcca_sin_degrees(2 * M))) -
                                        (0.00111 *
                                         lcca_sin_degrees(MPrime - (2 * F)))) -
                                       (0.00057 *
                                        lcca_sin_degrees(MPrime + (2 * F)))) +
                                      ((0.00056 * E) *
                                       lcca_sin_degrees((2 * MPrime) + M))) -
                                     (0.00042 * lcca_sin_degrees(3 * MPrime))) +
                                    ((0.00042 * E) *
                                     lcca_sin_degrees(M + (2 * F)))) +
                                   ((0.00038 * E) *
                                    lcca_sin_degrees(M - (2 * F)))) -
                                  ((0.00024 * E) *
                                   lcca_sin_degrees((2 * MPrime) - M))) -
                                 (0.00017 * lcca_sin_degrees(Omega))) -
                                (0.00007 *
                                 lcca_sin_degrees(MPrime + (2 * M)))) +
                               (0.00004 *
                                lcca_sin_degrees((2 * MPrime) - (2 * F)))) +
                              (0.00004 * lcca_sin_degrees(3 * M))) +
                             (0.00003 *
                              lcca_sin_degrees((MPrime + M) - (2 * F)))) +
                            (0.00003 *
                             lcca_sin_degrees((2 * MPrime) + (2 * F)))) -
                           (0.00003 *
                            lcca_sin_degrees((MPrime + M) + (2 * F)))) +
                          (0.00003 *
                           lcca_sin_degrees((MPrime - M) + (2 * F)))) -
                         (0.00002 * lcca_sin_degrees((MPrime - M) - (2 * F)))) -
                        (0.00002 * lcca_sin_degrees((3 * MPrime) + M))) +
                       (0.00002 * lcca_sin_degrees(4 * MPrime))) +
                      (0.000325 * lcca_sin_degrees((299.77 + (0.107408 * k)) -
                                                   (0.009173 * T2)))) +
                     (0.000165 * lcca_sin_degrees(251.88 + (0.016321 * k)))) +
                    (0.000164 * lcca_sin_degrees(251.83 + (26.651886 * k)))) +
                   (0.000126 * lcca_sin_degrees(349.42 + (36.412478 * k)))) +
                  (0.000110 * lcca_sin_degrees(84.66 + (18.206239 * k)))) +
                 (0.000062 * lcca_sin_degrees(141.74 + (53.303771 * k)))) +
                (0.000060 * lcca_sin_degrees(207.14 + (2.453732 * k)))) +
               (0.000056 * lcca_sin_degrees(154.84 + (7.306860 * k)))) +
              (0.000047 * lcca_sin_degrees(34.52 + (27.261239 * k)))) +
             (0.000042 * lcca_sin_degrees(207.19 + (0.121824 * k)))) +
            (0.000040 * lcca_sin_degrees(291.34 + (1.844379 * k)))) +
           (0.000037 * lcca_sin_degrees(161.72 + (24.198154 * k)))) +
          (0.000035 * lcca_sin_degrees(239.56 + (25.513099 * k)))) +
         (0.000023 * lcca_sin_degrees(331.55 + (3.592518 * k))));
    return JDE + corrections;
}

lcca_f64 lcca_approximate_k(const lcca_gregorian_date gregorian,
                            const lcca_time time) {
    (void)lcca_c_assert(lcca_is_valid_gregorian_date(gregorian));
    (void)lcca_c_assert(lcca_is_valid_time_of_day(time));
    {
    const lcca_i32 K =
        ((gregorian.year % 400 == 0) ||
         ((gregorian.year % 4 == 0) && (gregorian.year % 100 != 0)))
            ? 1
            : 2;
    const lcca_i32 day_of_year =
            ((((275 * (lcca_i32)gregorian.month) / 9) /** Integer division */
              - K * (((lcca_i32)gregorian.month + 9) /
                     12)) /** Integer division */
         + (gregorian.day - 30));
        const lcca_f64 F =
            ((time.hours - gregorian.time_zone) +
                        ((time.minutes + (time.seconds / 60.0)) / 60.0) / 24.0);
    return ((((day_of_year - 1) + F) / ((K == 1) ? 366 : 365)) +
            (gregorian.year - 2000)) *
           12.3685;
    }
}

lcca_f64 lcca_get_winter_solstice_jd_td(const lcca_i32 year) {
    /** Left and right are anchors for a binary search
     * and both anchors are very far away from the Winter Solstice,
     * so assuming TD = UT is safe.
     * Omitting time zone is also trivial here.
     */
    lcca_f64 left;
    lcca_f64 right;
    lcca_f64 middle;
    lcca_f64 longitude;
    int i;
    {
        lcca_gregorian_date gregorian;
        gregorian.year = year;
        gregorian.month = 12;
        gregorian.day = 1;
        gregorian.time_zone = 0;
        left = lcca_convert_gregorian_to_jd_ut(gregorian,
                                               lcca_new_midnight_time());
    }
    right = left + 35.0;
    middle = (left + right) / 2;
    for (i = 0; i < 100; i += 1) {
        middle = (left + right) / 2;
        longitude = lcca_get_sun_true_longitude(middle);
        if (fabs(longitude - 270.0) < 1e-9) {
            return middle;
        }
        if (longitude < 270.0) {
            left = middle;
        } else {
            right = middle;
        }
    }
    return middle;
}

lcca_i32 lcca_get_k_of_month_11(const lcca_i32 year, const lcca_f64 time_zone) {
    (void)lcca_c_assert((time_zone < 24.0) && (time_zone > -24.0));
    {
        /* w: The moment of the Winter Solstice in JD (TD) */
        const lcca_f64 w_td = lcca_get_winter_solstice_jd_td(year);
        const lcca_f64 w_Delta_T = lcca_get_delta_t_seconds_td(w_td);
        const lcca_f64 w_ut = w_td - (w_Delta_T / 86400.0);
        const lcca_gregorian_date w_date =
            lcca_convert_jd_ut_to_gregorian(w_ut, time_zone);
        const lcca_time w_time =
            lcca_convert_jd_ut_to_time_of_day(w_ut, time_zone);

        /* m: The moment of New Moon in JD (TD) */
        const lcca_f64 k_approx =
            floor(lcca_approximate_k(w_date, w_time) + 0.5);
        const lcca_i32 k = (lcca_i32)k_approx;
        const lcca_f64 m_td = lcca_get_new_moon_jd_td(k);
        const lcca_f64 m_Delta_T = lcca_get_delta_t_seconds_td(m_td);
        const lcca_f64 m_ut = m_td - (m_Delta_T / 86400.0);
        const lcca_gregorian_date m_date =
            lcca_convert_jd_ut_to_gregorian(m_ut, time_zone);

        /* Represent dates as packed integers to compare */
        const lcca_i64 w_packed =
            (((lcca_i64)w_date.year * 10000) + ((lcca_i64)w_date.month * 100)) +
            w_date.day;
        const lcca_i64 m_packed =
            ((lcca_i64)m_date.year * 10000 + ((lcca_i64)m_date.month * 100)) +
            m_date.day;

        return m_packed > w_packed ? k - 1 : k;
    }
}

lcca_leap_month_result
lcca_get_k_of_leap_month(const lcca_i32 current_year_month_11_k,
                         const lcca_i32 next_year_month_11_k,
                         const lcca_f64 time_zone) {
    lcca_leap_month_result result;
    int i;
    result.have_leap_month = 0;
    result.k = 0; /* Prevents leaking stack memory */

    (void)lcca_c_assert(current_year_month_11_k < next_year_month_11_k);
    (void)lcca_c_assert((time_zone < 24.0) && (time_zone > -24.0));

    if (next_year_month_11_k - current_year_month_11_k <= 12) {
        return result;
    }
    {
        const lcca_i32 k = current_year_month_11_k;
        for (i = 1; i <= 12; i += 1) {
            const lcca_f64 current_td = lcca_get_new_moon_jd_td(k + i);
            const lcca_f64 next_td = lcca_get_new_moon_jd_td(k + (i + 1));

            const lcca_f64 current_ut =
                (current_td -
                 (lcca_get_delta_t_seconds_td(current_td) / 86400.0));
            const lcca_gregorian_date current_date =
                lcca_convert_jd_ut_to_gregorian(current_ut, time_zone);

            const lcca_f64 current_midnight_ut =
                lcca_convert_gregorian_to_jd_ut(current_date,
                                                lcca_new_midnight_time());
            const lcca_f64 current_midnight_td =
                (current_midnight_ut +
                 (lcca_get_delta_t_seconds(current_midnight_ut) / 86400.0));
            const lcca_f64 lon1 =
                lcca_get_sun_true_longitude(current_midnight_td);

            const lcca_f64 next_ut =
                next_td - (lcca_get_delta_t_seconds_td(next_td) / 86400.0);
            const lcca_gregorian_date next_date =
                lcca_convert_jd_ut_to_gregorian(next_ut, time_zone);
            const lcca_f64 next_midnight_ut = lcca_convert_gregorian_to_jd_ut(
                next_date, lcca_new_midnight_time());
            const lcca_f64 next_midnight_td =
                (next_midnight_ut +
                 lcca_get_delta_t_seconds(next_midnight_ut) / 86400.0);
            const lcca_f64 lon2 = lcca_get_sun_true_longitude(next_midnight_td);

            const lcca_f64 a = floor(lon1 / 30.0);
            const lcca_f64 b = floor(lon2 / 30.0);

            if ((lcca_i32)a == (lcca_i32)b) {
                result.have_leap_month = 1;
                result.k = k + i;
                return result;
            }
        }
    }

    return result;
}

lcca_f64 lcca_get_new_moon_midnight_jd_ut(const lcca_i32 k,
                                          const lcca_f64 time_zone) {
    lcca_f64 td;
    lcca_f64 ut;
    lcca_gregorian_date date;
    (void)lcca_c_assert((time_zone < 24.0) && (time_zone > -24.0));
    td = lcca_get_new_moon_jd_td(k);
    ut = td - (lcca_get_delta_t_seconds_td(td) / 86400.0);
    date = lcca_convert_jd_ut_to_gregorian(ut, time_zone);
    return lcca_convert_gregorian_to_jd_ut(date, lcca_new_midnight_time());
}
