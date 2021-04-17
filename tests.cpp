/***************************************************************************************************
 *
 * Copyright (C) 2020 Cassio Neri and Lorenz Schneider
 *
 * This file is part of https://github.com/cassioneri/calendar.
 *
 * This file is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software  Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY  WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this file. If not,
 * see <https://www.gnu.org/licenses/>.
 *
 **************************************************************************************************/

/**
 * @file tests.cpp
 *
 * @brief Tests of calendar algorithms. (Requires googletest [1].)
 *
 * [1] https://github.com/google/googletest
 *
 * Compile with: g++ -O3 -std=c++2a tests.cpp -o tests -lgtest -lgtest_main
 */

#include "calendar.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <iostream>

//--------------------------------------------------------------------------------------------------
// Config
//--------------------------------------------------------------------------------------------------

using year_t     = std::int16_t; // as in std::chrono::year
using month_t    = std::uint8_t; // as in std::chrono::month
using day_t      = std::uint8_t; // as in std::chrono::day
using rata_die_t = std::int32_t; // as in std::chrono::days

auto constexpr enable_static_asserts = true;

//--------------------------------------------------------------------------------------------------
// Standalone algorithms
//--------------------------------------------------------------------------------------------------

struct standalone {
  using year_t     = ::year_t;
  using month_t    = ::month_t;
  using day_t      = ::day_t;
  using rata_die_t = ::rata_die_t;
  using date_t     = ::date_t<year_t>;
};

struct neri_schneider : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = min<date_t>;
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -12687794;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = min<date_t>;
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -12687794;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // Neri and Schneider, Euclidean Affine Functions and Applications to Calendar Algorithms
  // https://arxiv.org/pdf/2102.06959.pdf

  // Proposition 6.2.
  rata_die_t static constexpr
  to_rata_die(date_t const& u2) noexcept {

    auto constexpr z2    = uint32_t(-1468000);
    auto constexpr r2_e3 = uint32_t(536895458);

    auto const y1 = uint32_t(u2.year) - z2;
    auto const m1 = uint32_t(u2.month);
    auto const d1 = uint32_t(u2.day);

    auto const j  = uint32_t(m1 < 3);
    auto const y0 = y1 - j;
    auto const m0 = j ? m1 + 12 : m1;
    auto const d0 = d1 - 1;

    auto const q1 = y0 / 100;
    auto const yc = 1461 * y0 / 4 - q1 + q1 / 4;
    auto const mc = (979 * m0 - 2919) / 32;
    auto const dc = d0;

    auto const r3 = yc + mc + dc - r2_e3;

    return r3;
  }

  // Proposition 6.3.
  date_t static constexpr
  to_date(rata_die_t r) noexcept {

    auto constexpr z2    = uint32_t(-1468000);
    auto constexpr r2_e3 = uint32_t(536895458);

    auto const r0 = r + r2_e3;

    auto const n1 = 4 * r0 + 3;
    auto const q1 = n1 / 146097;
    auto const r1 = n1 % 146097 / 4;

    auto constexpr p32 = uint64_t(1) << 32;
    auto const n2 = 4 * r1 + 3;
    auto const u2 = uint64_t(2939745) * n2;
    auto const q2 = uint32_t(u2 / p32);
    auto const r2 = uint32_t(u2 % p32) / 2939745 / 4;

    auto constexpr p16 = uint32_t(1) << 16;
    auto const n3 = 2141 * r2 + 197913;
    auto const q3 = n3 / p16;
    auto const r3 = n3 % p16 / 2141;

    auto const y0 = 100 * q1 + q2;
    auto const m0 = q3;
    auto const d0 = r3;

    auto const j  = r2 >= 306;
    auto const y1 = y0 + j;
    auto const m1 = j ? m0 - 12 : m0;
    auto const d1 = d0 + 1;

    return {year_t(y1 + z2), month_t(m1), day_t(d1)};
  }

}; // struct neri_schneider

struct baum : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{0, 3, 1};
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -719468;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = date_t{0, 3, 1};
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -719468;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // https://tinyurl.com/y44rgx2j

  // Section 5.1
  rata_die_t static constexpr
  to_rata_die(date_t const& u) noexcept {
    auto const j = u.month < 3;
    auto const z = u.year - j;                                      // step 1 / alternative 2
    auto const m = j ? u.month + 12 : u.month;                      // step 2 / alternative 3
    auto const f = (979 * m - 2918) / 32;                           //
    auto const n = u.day + f + 365 * z + z / 4 - z / 100 + z / 400; // step 3
    return n - 719469;
  }

  // Section 6.2.1/3
  date_t static constexpr
  to_date(rata_die_t n) noexcept {
    auto const z  = uint32_t(n) + 719469;
    auto const h  = 100 * z - 25;
    auto const a  = h / 3652425;
    auto const b  = a - a / 4;
    auto const y_ = (100 * b + h) / 36525;
    auto const c  = b + z - 365 * y_ - y_ / 4;
    auto const m_ = (535 * c + 48950) / 16384;
    auto const d  = c - (979 * m_ - 2918) / 32;
    auto const j  = m_ > 12;
    auto const y  = y_ + j;
    auto const m  = j ? m_ - 12 : m_;
    return {year_t(y), month_t(m), day_t(d)};
  }

}; // struct baum

/*
 Code in next struct is subject to the following terms.

 Copyright (c) 2002,2003 CrystalClear Software, Inc.

 Boost Software License - Version 1.0 - August 17th, 2003

 Permission is hereby granted, free of charge, to any person or organization
 obtaining a copy of the software and accompanying documentation covered by
 this license (the "Software") to use, reproduce, display, distribute,
 execute, and transmit the Software, and to prepare derivative works of the
 Software, and to permit third-parties to whom the Software is furnished to
 do so, all subject to the following:

 The copyright notices in the Software and this entire statement, including
 the above license grant, this restriction and the following disclaimer,
 must be included in all copies of the Software, in whole or in part, and
 all derivative works of the Software, unless such copies or derivative
 works are solely in the form of machine-executable object code generated by
 a source language processor.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/
struct boost : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{-4800, 3, 1};
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -2472632;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = date_t{-4800, 3, 1};
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -2472632;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // https://tinyurl.com/ybmm7dzr
  rata_die_t static constexpr
  to_rata_die(date_t const& ymd) noexcept {
    unsigned short a = static_cast<unsigned short>((14-ymd.month)/12);
    unsigned short y = static_cast<unsigned short>(ymd.year + 4800 - a);
    unsigned short m = static_cast<unsigned short>(ymd.month + 12*a - 3);
    unsigned long  d = ymd.day + ((153*m + 2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045;
    return d - 2440588;
  }

  // https://tinyurl.com/ybq2ozhm
  date_t static constexpr
  to_date(rata_die_t dayNumber) noexcept {
    uint32_t a = dayNumber + 32044 + 2440588;
    uint32_t b = (4*a + 3)/146097;
    uint32_t c = a-((146097*b)/4);
    uint32_t d = (4*c + 3)/1461;
    uint32_t e = c - (1461*d)/4;
    uint32_t m = (5*e + 2)/153;
    day_t day = static_cast<day_t>(e - ((153*m + 2)/5) + 1);
    month_t month = static_cast<month_t>(m + 3 - 12 * (m/10));
    year_t year = static_cast<year_t>(100*b + d - 4800 + (m/10));
    return {year, month, day};
  }

}; // struct boost

/*
 Code in next struct is subject to the following terms.

 The MIT License (MIT)

 Copyright (c) .NET Foundation and Contributors

 All rights reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/
struct dotnet : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{1, 1, 1};
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -719162;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = date_t{1, 1, 1};
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -719162;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // https://tinyurl.com/y85q3qrp
  int static constexpr s_daysToMonth365[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
  int static constexpr s_daysToMonth366[] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

  // https://tinyurl.com/ycegsjyu
  bool static constexpr
  IsLeapYear(int year) noexcept {
    return (year & 3) == 0 && ((year & 15) == 0 || (year % 25) != 0);
  }

  // https://tinyurl.com/y7r8u6zo
  rata_die_t static constexpr
  to_rata_die(date_t const& date) noexcept {
    int const* days = IsLeapYear(date.year) ? s_daysToMonth366 : s_daysToMonth365;
    int y = date.year - 1;
    int n = y * 365 + y / 4 - y / 100 + y / 400 + days[date.month - 1] + date.day - 1;
    return n - 719162;
  }

  // https://tinyurl.com/yctslxyt
  int static constexpr DaysPerYear = 365;
  int static constexpr DaysPer4Years = DaysPerYear * 4 + 1;       // 1461
  int static constexpr DaysPer100Years = DaysPer4Years * 25 - 1;  // 36524
  int static constexpr DaysPer400Years = DaysPer100Years * 4 + 1; // 146097

  // https://tinyurl.com/ybyoqjep
  date_t static constexpr
  to_date(rata_die_t rata_die) noexcept {
    int n = rata_die + 719162;
    int y400 = n / DaysPer400Years;
    n -= y400 * DaysPer400Years;
    int y100 = n / DaysPer100Years;
    if (y100 == 4) y100 = 3;
    n -= y100 * DaysPer100Years;
    int y4 = n / DaysPer4Years;
    n -= y4 * DaysPer4Years;
    int y1 = n / DaysPerYear;
    if (y1 == 4) y1 = 3;
    int year = y400 * 400 + y100 * 100 + y4 * 4 + y1 + 1;
    n -= y1 * DaysPerYear;
    bool leapYear = y1 == 3 && (y4 != 24 || y100 == 3);
    int const* days = leapYear ? s_daysToMonth366 : s_daysToMonth365;
    int m = (n >> 5) + 1;
    while (n >= days[m]) m++;
    month_t month = m;
    int day = n - days[m - 1] + 1;
    return {year_t(year), month_t(month), day_t(day)};
  }

}; // struct dotnet

struct fliegel_flandern : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{-4800, 3, 1};
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -2509157;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = date_t{-4800, 3, 1};
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -2472632;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // H.F. Fliegel and T.C.V. Flandern, A Machine Algorithm for Processing Calendar Dates
  // Communications of the ACM, Vol. 11, No. 10 (1968), p657.

  rata_die_t static constexpr
  to_rata_die(date_t const& u) noexcept {
    auto const I  = rata_die_t(u.year);
    auto const J  = rata_die_t(u.month);
    auto const K  = rata_die_t(u.day);
    auto const JD = K - 32075 + 1461 * (I + 4800 + (J - 14) / 12) / 4
      + 367 * (J - 2 - (J - 14) / 12 * 12) / 12 - 3
      * ((I + 4900 + (J - 14) / 12) / 100) / 4;
    return JD - 2440588;
  }

  date_t static constexpr
  to_date(rata_die_t n) noexcept {
    auto const JD = n + 2440588;
    auto       L  = JD + 68569;
    auto const N  = 4 * L / 146097;
               L  = L - (146097 * N + 3) / 4;
    auto       I  = 4000 * (L + 1) / 1461001;
               L  = L - 1461 * I / 4 + 31;
    auto       J  = 80 * L / 2447;
    auto const K  = L - 2447 * J / 80;
               L  = J / 11;
               J  = J + 2 - 12 * L;
               I  = 100 * (N - 49) + I + L;
    return {year_t(I), month_t(J), day_t(K)};
  }

}; // struct fliegel_flandern

/*
 Code in next stuct is subject to the following terms.

 Copyright (C) 1993-2020 Free Software Foundation, Inc.

 This section of the file is part of the GNU C Library.
 Contributed by Paul Eggert <eggert@twinsun.com>.
 The GNU C Library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 The GNU C Library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.

 See <https://www.gnu.org/licenses/>.
*/
struct glibc : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = min<date_t>;
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -12687794;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = min<date_t>;
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -12687794;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // https://tinyurl.com/ydhzwd4n
  long int static constexpr
  shr(long int a, int b) noexcept {
    long int one = 1;
    return (-one >> 1 == -1 ? a >> b : a / (one << b) - (a % (one << b) < 0));
  }

  // https://tinyurl.com/ycz86upl
  #define EPOCH_YEAR 1970
  #define TM_YEAR_BASE 1900

  // https://tinyurl.com/ya33text
  bool static constexpr
  leapyear (long int year) noexcept {
    return ((year & 3) == 0 && (year % 100 != 0 ||
      ((year / 100) & 3) == (- (TM_YEAR_BASE / 100) & 3)));
  }

  // https://tinyurl.com/y9bkozw6
  unsigned short int static constexpr __mon_yday[2][13] =
    {
      /* Normal years.  */
      { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
      /* Leap years.  */
      { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
    };

  // https://tinyurl.com/y7qrapyo
  long int static constexpr
  ydhms_diff (long int year1, long int yday1, int year0) noexcept {
    int a4 = shr (year1, 2) + shr (TM_YEAR_BASE, 2) - ! (year1 & 3);
    int b4 = shr (year0, 2) + shr (TM_YEAR_BASE, 2) - ! (year0 & 3);
    int a100 = a4 / 25 - (a4 % 25 < 0);
    int b100 = b4 / 25 - (b4 % 25 < 0);
    int a400 = shr (a100, 2);
    int b400 = shr (b100, 2);
    int intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);
    long int years = year1 - year0;
    long int days = 365 * years + yday1 + intervening_leap_days;
    return days;
  }

  // https://tinyurl.com/yap87jlu
  rata_die_t static constexpr
  to_rata_die(date_t const& date) noexcept {
    int mday = date.day;
    int mon = date.month - 1;
    long int year = date.year - TM_YEAR_BASE;
    int mon_yday = (__mon_yday[leapyear(year)][mon]) - 1;
    long int yday = mon_yday + mday;
    long int t0 = ydhms_diff (year, yday, EPOCH_YEAR - TM_YEAR_BASE);
    return t0;
  }

  // https://tinyurl.com/y7y3doog
  #define __isleap(year) \
    ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

  // https://tinyurl.com/ycdpf72e
  date_t static constexpr
  to_date(rata_die_t days) noexcept {

    long int y = 1970;
    #define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
    #define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

    while (days < 0 || days >= (__isleap (y) ? 366 : 365))
      {
        /* Guess a corrected year, assuming 365 days per year.  */
        long int yg = y + days / 365 - (days % 365 < 0);

        /* Adjust DAYS and Y to match the guessed year.  */
        days -= ((yg - y) * 365
                + LEAPS_THRU_END_OF (yg - 1)
                - LEAPS_THRU_END_OF (y - 1));
        y = yg;
      }

    auto ip = __mon_yday[__isleap(y)];
    long int m = 0;
    for (m = 11; days < (long int) ip[m]; --m)
      continue;
    days -= ip[m];
    return {year_t(y), month_t(m + 1), day_t(days + 1)};
  }

}; // struct glibc

struct hatcher : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{1900, 3,  1};
  date_t     static constexpr date_max           = date_t{2100, 2, 28};
  rata_die_t static constexpr rata_die_min       = -25495;
  rata_die_t static constexpr rata_die_max       = 47540;

  date_t     static constexpr round_date_min     = date_t{1900, 3, 14};
  date_t     static constexpr round_date_max     = date_t{2100, 2, 28};
  rata_die_t static constexpr round_rata_die_min = -25495;
  rata_die_t static constexpr round_rata_die_max = 47540;

  // Algorithms by D.A. Hactcher as appeared in
  // E.G. Richards, Mapping Time, The CALENDAR and its HISTORY, Oxford University Press, 1998

  // Table 25.1, page 311.
  auto static constexpr y = rata_die_t(4716);
  auto static constexpr m = rata_die_t(3);
  auto static constexpr n = rata_die_t(12);
  auto static constexpr r = rata_die_t(4);
  auto static constexpr p = rata_die_t(1461);
  auto static constexpr q = rata_die_t(0);
  auto static constexpr v = rata_die_t(3);
  auto static constexpr u = rata_die_t(5);
  auto static constexpr s = rata_die_t(153);
  auto static constexpr t = rata_die_t(2);
  auto static constexpr w = rata_die_t(2);

  // Table 25.4, page 320.
  auto static constexpr A = rata_die_t(184);
  auto static constexpr B = rata_die_t(274277);
  auto static constexpr G = rata_die_t(-38);
  // Page 319
  auto static constexpr K = 36524;

  // Algorithm E, page 323.
  rata_die_t static constexpr
  to_rata_die(date_t const& x) noexcept {
    auto const Y  = rata_die_t(x.year);
    auto const M  = rata_die_t(x.month);
    auto const D  = rata_die_t(x.day);
    auto const Yp = Y + y - (n + m - 1 - M) / n;
    auto const Mp = (M - m + n) % n;
    auto const Dp = D - 1;
    auto const c  = (p * Yp + q) / r;
    auto const d  = (s * Mp + t) / u;
    auto const g  = 3 * ((Yp + A) / 100) / 4 + G;
    auto const j  = 1401 + g;
    auto const J  = c + d + Dp - j - g;
    return J - 2440575;
  }

  // Algorithm F, page 324.
  date_t static constexpr
  to_date(rata_die_t x) noexcept {
    auto const J  = x + 2440575;
    auto const g  = 3 * ((4 * J + B) / (4 * K + 1)) / 4 + G;
    auto const j  = 1401 + g;
    auto const Jp = J + j + g;
    auto const Yp = (r * Jp + v) / p;
    auto const Tp = (r * Jp + v) % p / r;
    auto const Mp = (u * Tp + w) / s;
    auto const Dp = (u * Tp + w) % s / u;
    auto const D  = Dp + 1;
    auto const M  = (Mp + m - 1) % n + 1;
    auto const Y  = Yp - y + (n + m - 1 - M) / n;
    return {year_t(Y), month_t(M), day_t(D)};
  }

}; // struct hatcher

/*
 Code in next struct is subject to the following terms.

 Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.
 DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

 This code is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 only, as
 published by the Free Software Foundation.  Oracle designates this
 particular file as subject to the "Classpath" exception as provided
 by Oracle in the LICENSE file that accompanied this code.

 This code is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 version 2 for more details (a copy is included in the LICENSE file that
 accompanied this code).

 You should have received a copy of the GNU General Public License version
 2 along with this work; if not, write to the Free Software Foundation,
 Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.

 Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 or visit www.oracle.com if you need additional information or have any
 questions.

 ---

 This file is available under and governed by the GNU General Public
 License version 2 only, as published by the Free Software Foundation.
 However, the following notice accompanied the original version of this
 file:

 Copyright (c) 2007-2012, Stephen Colebourne & Michael Nascimento Santos

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of JSR-310 nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
struct openjdk : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = min<date_t>;
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -12687794;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = min<date_t>;
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -12687794;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // https://tinyurl.com/ybdspcjc
  bool static constexpr
  isLeapYear(long prolepticYear) noexcept {
    return ((prolepticYear & 3) == 0) && ((prolepticYear % 100) != 0 || (prolepticYear % 400) == 0);
  }

  // https://tinyurl.com/y99vybz9
  int static constexpr DAYS_PER_CYCLE = 146097;
  int static constexpr DAYS_0000_TO_1970 = (DAYS_PER_CYCLE * 5L) - (30L * 365L + 7L);

  // https://tinyurl.com/ydxp5346
  rata_die_t static constexpr
  to_rata_die(date_t const& date) noexcept {
    long y = date.year;
    long m = date.month;
    long total = 0;
    total += 365 * y;
    if (y >= 0) {
      total += (y + 3) / 4 - (y + 99) / 100 + (y + 399) / 400;
    } else {
      total -= y / -4 - y / -100 + y / -400;
    }
    total += ((367 * m - 362) / 12);
    total += date.day - 1;
    if (m > 2) {
      total--;
      if (isLeapYear(y) == false) {
          total--;
      }
    }
    return total - DAYS_0000_TO_1970;
  }

  // https://tinyurl.com/ybb3nnhw
  date_t static constexpr
  to_date(rata_die_t epochDay) noexcept {

    long zeroDay = epochDay + DAYS_0000_TO_1970;
    // find the march-based year
    zeroDay -= 60;  // adjust to 0000-03-01 so leap day is at end of four year cycle
    long adjust = 0;
    if (zeroDay < 0) {
        // adjust negative years to positive for calculation
        long adjustCycles = (zeroDay + 1) / DAYS_PER_CYCLE - 1;
        adjust = adjustCycles * 400;
        zeroDay += -adjustCycles * DAYS_PER_CYCLE;
    }
    long yearEst = (400 * zeroDay + 591) / DAYS_PER_CYCLE;
    long doyEst = zeroDay - (365 * yearEst + yearEst / 4 - yearEst / 100 + yearEst / 400);
    if (doyEst < 0) {
        // fix estimate
        yearEst--;
        doyEst = zeroDay - (365 * yearEst + yearEst / 4 - yearEst / 100 + yearEst / 400);
    }
    yearEst += adjust;  // reset any negative year
    int marchDoy0 = (int) doyEst;

    // convert march-based values back to january-based
    int marchMonth0 = (marchDoy0 * 5 + 2) / 153;
    int month = (marchMonth0 + 2) % 12 + 1;
    int dom = marchDoy0 - (marchMonth0 * 306 + 5) / 10 + 1;
    yearEst += marchMonth0 / 10;

    // check year now we are certain it is correct
    int year = (int) yearEst;

    return {year_t(yearEst), month_t(month), day_t(dom)};
  }

}; // struct openjdk

struct reingold_dershowitz : standalone {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{0, 3, 1};
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -719468;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = date_t{0, 3, 1};
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -719468;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // E. M. Reingold and N. Dershowitz, Calendrical Calculations, The Ultimate Edition, Cambridge
  // University Press, 2018.

  // Table 1.2, page 17.
  rata_die_t static constexpr gregorian_epoch = 1;

  // alt-fixed-from-gregorian, equation (2.28), page 65.
  rata_die_t static constexpr
  to_rata_die(date_t const& date) noexcept {

    auto const year  = rata_die_t(date.year );
    auto const month = rata_die_t(date.month);
    auto const day   = rata_die_t(date.day  );

    // mp := (month - 3) mod 12. If month - 3 < 0, then % doesn't match mod. The below provides the
    // intended result and keeps performance of the original formula.
    auto const mp = (month + 9) % 12;
    auto const yp = year - mp / 10;

    // On page 66, quantities below are denoted by n400, n100, n4 and n1.
    // Equation (1.42), page 28, with b = <4, 25, 4>, i.e., b0 = 4, b1 = 25 and b2 = 4 gives
    auto const a0 = (yp / 400);
    auto const a1 = (yp / 100) %  4;
    auto const a2 = (yp /   4) % 25;
    auto const a3 = (yp /   1) %  4;

    auto const n = gregorian_epoch - 1 - 306 + 365 * yp + 97 * a0 + 24 * a1 + 1 * a2 + 0 * a3 +
      (3 * mp + 2) / 5 + 30 * mp + day;
    return n - 719163;
  }

  // gregorian-year-from-fixed, equation (2.21), page 61.
  rata_die_t static constexpr
  gregorian_year_from_fixed(rata_die_t const& date) noexcept {
    auto const d0   = date - gregorian_epoch;
    auto const n400 = d0 / 146097;
    auto const d1   = d0 % 146097;
    auto const n100 = d1 / 36524;
    auto const d2   = d1 % 36524;
    auto const n4   = d2 / 1461;
    auto const d3   = d2 % 1461;
    auto const n1   = d3 / 365;
    auto const year = 400 * n400 + 100 * n100 + 4 * n4 + n1;
    return (n100 == 4 | n1 == 4) ? year : year + 1;
  }

  // alt-fixed-from-gregorian, equation (2.28), page 65.
  rata_die_t static constexpr
  fixed_from_gregorian(date_t const& date) noexcept {
    return to_rata_die(date) + 719163;
  }

  rata_die_t static constexpr
  mod_1_12(rata_die_t month) noexcept {
    return month > 12 ? month - 12 : month;
  }

  // alt-gregorian-from-fixed, equation (2.29), page 66.
  date_t static constexpr
  to_date(rata_die_t date) noexcept {
    date = date + 719163;
    auto const y          = gregorian_year_from_fixed(gregorian_epoch - 1 + date + 306);
    auto const prior_days = date - fixed_from_gregorian(date_t{year_t(y - 1), 3, 1});
    auto const month      = mod_1_12((5 * prior_days + 2) / 153 + 3);
    auto const year       = y - (month + 9) / 12;
    auto const day        = date - fixed_from_gregorian(date_t{year_t(year), month_t(month), 1})
      + 1;
    return {year_t(year), month_t(month), day_t(day)};
  }

}; // struct reingold_dershowitz

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * Advances a date by one day.
 *
 * @param date        Date to be advanced.
 *
 * @pre               date < max<date_t>.
 */
template <typename T>
date_t<T> constexpr
advance(date_t<T>& date) noexcept {
  if (date.day != last_day_of_month(date.year, date.month))
    ++date.day;
  else {
    date.day = 1;
    if (date.month != 12)
      ++date.month;
    else {
      date.month = 1;
      ++date.year;
    }
  }
  return date;
}

/**
 * Returns the next date.
 *
 * @param date        Date to be advanced.
 *
 * @pre               date < max<date_t>.
 */
template <typename T>
date_t<T> constexpr
next(date_t<T> date) noexcept {
  return advance(date);
}

/**
 * Regresses a date by one day.
 *
 * @param date        Date to be regressed.
 *
 * @pre               date > min<date_t>.
 */
template <typename T>
date_t<T> constexpr
regress(date_t<T>& date) noexcept {
  if (date.day != 1)
    --date.day;
  else {
    if (date.month != 1)
      --date.month;
    else {
      date.month = 12;
      --date.year;
    }
    date.day = last_day_of_month(date.year, date.month);
  }
  return date;
}

/**
 * Returns the previos date.
 *
 * @param date        Date to be regressed.
 *
 * @pre               date > min<date_t>.
 */
template <typename T>
date_t<T> constexpr
previous(date_t<T> date) noexcept {
  return regress(date);
}

//--------------------------------------------------------------------------------------------------
// Standard compliance tests
//--------------------------------------------------------------------------------------------------

/**
 * Tests compliance with the C++ 2020 Standard.
 */
TEST(standard_compliance_tests, epoch_and_limits) {

  using year_t      = std::int16_t; // as in std::chrono::year
  using month_t     = std::uint8_t; // as in std::chrono::month
  using day_t       = std::uint8_t; // as in std::chrono::day
  using rata_die_t  = std::int32_t; // as in std::chrono::days

  using gregorian_t = ::gregorian_t<year_t, rata_die_t>;

  // https://eel.is/c++draft/time.clock.system#overview-1

  static_assert(!enable_static_asserts ||
    unix_epoch<year_t> == date_t<year_t>{1970, 1, 1});

  static_assert(!enable_static_asserts ||
    gregorian_t::to_date(0) == unix_epoch<year_t>);

  // https://eel.is/c++draft/time.cal.ymd#members-20

  static_assert(!enable_static_asserts ||
    gregorian_t::round_rata_die_min <= -12687428);

  static_assert(!enable_static_asserts ||
    gregorian_t::round_rata_die_max >=  11248737);
}

//--------------------------------------------------------------------------------------------------
// Fast alternative tests
//--------------------------------------------------------------------------------------------------

auto constexpr p16         = std::uint32_t(1) << 16;
auto constexpr p32         = std::uint64_t(1) << 32;
auto constexpr month_count = [](auto n) { return (153 * n - 457) / 5; };
auto constexpr month       = [](auto n) { return (5 * n + 461) / 153; };

/**
 * Tests fast month count. (Rounding up.)
 */
TEST(fast, month_count_rounding_up) {

  auto constexpr month_count_fast = [](auto n) { return (980 * n - 2928) / 32; };

  auto constexpr N = std::uint32_t(12);
  for (std::uint32_t n = 3; n < N; ++n)
    ASSERT_EQ(month_count(n), month_count_fast(n)) << "Failed for n = " << n;
  ASSERT_NE  (month_count(N), month_count_fast(N)) << "Upper bound is not sharp.";
}

/**
 * Tests fast month count. (Rounding down.)
 */
TEST(fast, month_count_rounding_down) {

  auto constexpr month_count_fast = [](auto n) { return (979 * n - 2919) / 32; };

  auto constexpr N = std::uint32_t(34);
  for (std::uint32_t n = 3; n < N; ++n)
    ASSERT_EQ(month_count(n), month_count_fast(n)) << "Failed for n = " << n;
  ASSERT_NE  (month_count(N), month_count_fast(N)) << "Upper bound is not sharp.";
}

/**
 * Tests fast month. (Rounding up.)
 */
TEST(fast, month_rounding_up) {

  auto constexpr month_fast = [](auto n) { return (2142 * n + 197428) / p16; };
  auto constexpr day_fast   = [](auto n) { return (2142 * n + 197428) % p16 / 2142; };

  auto constexpr N = std::uint32_t(1560);
  for (std::uint32_t n = 0; n < N; ++n) {
    ASSERT_EQ(month(n), month_fast(n)) << "Failed for n = " << n;
    ASSERT_EQ(n - month_count(month(n)), day_fast(n)) << "Failed for n = " << n;
  }
  ASSERT_NE  (month(N), month_fast(N)) << "Upper bound is not sharp.";
}

/**
 * Tests fast residual for month. (Rounding down.)
 */
TEST(fast, month_rounding_down) {

  auto constexpr month_fast = [](auto n) { return (2141 * n + 197913) / p16; };
  auto constexpr day_fast   = [](auto n) { return (2141 * n + 197913) % p16 / 2142; };

  auto constexpr N = std::uint32_t(734);
  for (std::uint32_t n = 0; n < N; ++n) {
    ASSERT_EQ(month(n), month_fast(n)) << "Failed for n = " << n;
    ASSERT_EQ(n - month_count(month(n)), day_fast(n)) << "Failed for n = " << n;
  }
  ASSERT_NE  (month(N), month_fast(N)) << "Upper bound is not sharp.";
}

/**
 * Tests fast division by 1461.
 */
TEST(fast, division_by_1461) {

  auto constexpr alpha_prime = std::uint64_t(2939745);

  auto constexpr N = std::uint32_t(28825529);
  for (std::uint32_t n = 0; n < N; ++n) {
    auto const u = alpha_prime * n;
    auto const q = std::uint32_t(u / p32);
    auto const r = std::uint32_t(u % p32) / 2939745;
    ASSERT_EQ(q, n / 1461) << "Failed for n = " << n;
    ASSERT_EQ(r, n % 1461) << "Failed for n = " << n;
  }
  auto const u = alpha_prime * N;
  ASSERT_NE(N / 1461, std::uint32_t(u / p32)) << "Upper bound is not sharp.";
  ASSERT_NE(N % 1461, std::uint32_t(u % p32) / 2939745) << "Upper bound is not sharp.";
}

/**
 * Tests fast is divisibility by 100.
 */
TEST(fast, is_multiple_of_100) {
  for (std::int32_t n = -536870800; n <= 536870999; ++n)
    ASSERT_EQ(n % 100 == 0, is_multiple_of_100(n)) << "Failed for n = " << n;
}

//--------------------------------------------------------------------------------------------------
// Calendar tests
//--------------------------------------------------------------------------------------------------

template <typename A>
struct calendar_tests : public ::testing::Test {
}; // struct calendar_tests

using implementations = ::testing::Types<

  // Standalone implementation

  neri_schneider,

  // Third party implementations

  baum,
  boost,
  dotnet,
  fliegel_flandern,
  glibc,
  hatcher,
  openjdk,
  reingold_dershowitz,

  // 16 bits

  ugregorian_t<std::uint16_t, std::uint32_t>,
  gregorian_t <std:: int16_t, std:: int32_t>,
  gregorian_t <std:: int16_t, std:: int32_t, date_t<std::int16_t>{     0, 3, 1}>,
  gregorian_t <std:: int16_t, std:: int32_t, date_t<std::int16_t>{     0, 1, 1}>,
  gregorian_t <std:: int16_t, std:: int32_t, date_t<std::int16_t>{-    1, 1, 1}>,
  gregorian_t <std:: int16_t, std:: int32_t, date_t<std::int16_t>{-  400, 1, 1}>,
  gregorian_t <std:: int16_t, std:: int32_t, date_t<std::int16_t>{- 1970, 1, 1}>,
  gregorian_t <std:: int16_t, std:: int32_t, date_t<std::int16_t>{-32768, 1, 1}>,

  // 32 bits

  ugregorian_t<std::uint32_t, std::uint32_t>,
  gregorian_t <std:: int32_t, std:: int32_t>,
  gregorian_t <std:: int32_t, std:: int32_t, date_t<std::int32_t>{  1912, 6, 23}>,
  gregorian_t <std:: int32_t, std:: int32_t, date_t<std::int32_t>{- 1912, 6, 23}>
>;

TYPED_TEST_SUITE(calendar_tests, implementations);

TYPED_TEST(calendar_tests, show_info) {

  using A = TypeParam;

  std::cout << "             epoch              = " << A::epoch              << '\n';

  std::cout << "             date_min           = " << A::date_min           << '\n';
  std::cout << "             date_max           = " << A::date_max           << '\n';

  std::cout << "             rata_die_min       = " << A::rata_die_min       << '\n';
  std::cout << "             rata_die_max       = " << A::rata_die_max       << '\n';

  std::cout << "             round_date_min     = " << A::round_date_min     << '\n';
  std::cout << "             round_date_max     = " << A::round_date_max     << '\n';

  std::cout << "             round_rata_die_min = " << A::round_rata_die_min << '\n';
  std::cout << "             round_rata_die_max = " << A::round_rata_die_max << '\n';
}

/**
 * Tests whether epoch is mapped to 0.
 */
TYPED_TEST(calendar_tests, epoch) {

  using A          = TypeParam;
  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  static_assert(!enable_static_asserts ||
    A::to_date(0) == A::epoch);

  static_assert(!enable_static_asserts ||
    A::to_rata_die(A::epoch) == 0);
}

/**
 * Tests round trip limits.
 */
TYPED_TEST(calendar_tests, round_trip_limits) {

  using A          = TypeParam;
  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  static_assert(!enable_static_asserts ||
    A::round_rata_die_min == A::to_rata_die(A::round_date_min));

  static_assert(!enable_static_asserts ||
    A::round_rata_die_max == A::to_rata_die(A::round_date_max));

  static_assert(!enable_static_asserts ||
    A::round_date_min == A::to_date(A::round_rata_die_min));

  static_assert(!enable_static_asserts ||
    A::round_date_max == A::to_date(A::round_rata_die_max));
}

/**
 * Tests round trip conversions.
 */
TYPED_TEST(calendar_tests, round_trip) {

  using A = TypeParam;

  for (auto n = A::round_rata_die_min; n <= A::round_rata_die_max; ++n) {
    auto date = A::to_date(n);
    ASSERT_EQ(n, A::to_rata_die(date)) << "Failed for rata_die = " << n;
    ASSERT_EQ(date, A::to_date(A::to_rata_die(date))) << "Failed for date = " << date;
  }
}

/**
 * Tests whether to_date limits are sharp.
 */
TYPED_TEST(calendar_tests, to_date_limits) {

  using A          = TypeParam;
  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  auto constexpr first = A::to_date(A::rata_die_min);

  static_assert(!enable_static_asserts ||
    // dotnet needs special treatment: rata_die_t is signed but rata_die_min == 0
    (A::rata_die_min == min<rata_die_t> || std::is_same_v<A, dotnet>) ||
    first == min<date_t> || A::to_date(A::rata_die_min - 1) != previous(first));

  auto constexpr last = A::to_date(A::rata_die_max);

  static_assert(!enable_static_asserts ||
    A::rata_die_max == max<rata_die_t> || last == max<date_t> ||
    A::to_date(A::rata_die_max + 1) != next(last));
}

/**
 * Tests whether to_date produce correct results going forward from 0 to rata_die_max.
 */
TYPED_TEST(calendar_tests, to_date_forward) {

  using A          = TypeParam;
  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  date_t date = A::epoch;
  for (rata_die_t rata_die = 0; rata_die < A::rata_die_max; ) {

    auto const tomorrow = A::to_date(++rata_die);

    ASSERT_NE(date, max<date_t>) << "Failed for rata_die = " << rata_die <<
      " (date == max<date_t>)";

    ASSERT_EQ(tomorrow, advance(date)) << "Failed for rata_die = " << rata_die;
  }
}

/**
 * Tests whether to_date produce correct results going backward from 0 to rata_die_min.
 */
TYPED_TEST(calendar_tests, to_date_backward) {

  using A          = TypeParam;
  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  date_t date = A::epoch;
  for (rata_die_t rata_die = 0; A::rata_die_min < rata_die; ) {

    auto const yesterday = A::to_date(--rata_die);

    ASSERT_NE(date, min<date_t>) << "Failed for rata_die = " << rata_die <<
      " (date == min<date_t>)";

    ASSERT_EQ(yesterday, regress(date)) << "Failed for rata_die = " << rata_die;
  }
}

/**
 * Tests whether to_date limits are sharp.
 */
TYPED_TEST(calendar_tests, to_rata_die_limits) {

  using A          = TypeParam;
  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  auto constexpr first = A::to_rata_die(A::date_min);

  static_assert(!enable_static_asserts ||
    A::date_min == min<date_t> || first == min<rata_die_t> ||
    A::to_rata_die(previous(A::date_min)) != first - 1);

  auto constexpr last = A::to_rata_die(A::date_max);

  static_assert(!enable_static_asserts ||
    A::date_max == max<date_t> || last == max<rata_die_t> ||
    A::to_rata_die(next(A::date_max)) != last + 1);
}

/**
 * Tests whether to_rata_die produce correct results going foward from epoch to date_max.
 */
TYPED_TEST(calendar_tests, to_rata_die_forward) {

  using A          = TypeParam;
  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  rata_die_t rata_die = 0;
  for (date_t date = A::epoch; date < A::date_max; ) {

    auto const tomorrow = A::to_rata_die(advance(date));

    ASSERT_NE(rata_die, max<rata_die_t>) << "Failed for date = " << date <<
      " (rata die == max<rata_die_t>)";

    ASSERT_EQ(tomorrow, ++rata_die) << "Failed for date = " << date;
  }
}

/**
 * Tests whether to_rata_die produce correct results backward from epoch to date_min.
 */
TYPED_TEST(calendar_tests, to_rata_die_backward) {

  using A          = TypeParam;
  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  rata_die_t rata_die = 0;
  for (auto date = A::epoch; A::date_min < date; ) {

    auto const yesterday = A::to_rata_die(regress(date));

    ASSERT_NE(rata_die, min<rata_die_t>) << "Failed for date = " << date <<
      " (rata die == min<rata_die_t>)";

    ASSERT_EQ(yesterday, --rata_die) << "Failed for date = " << date;
  }
}
