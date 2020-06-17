/***************************************************************************************************
 *
 * Copyright (C) 2020 Cassio Neri
 *
 * This file is part of https://github.com/cassioneri/dates.
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

#include "date.hpp"

#include <cstdint>
#include <iostream>

// Compile with: g++ -O3 -std=c++2a tests.cpp -o tests

//--------------------------------------------------------------------------------------------------
// Config
//--------------------------------------------------------------------------------------------------

using year_t     = std::int16_t; // as in std::chrono::year
using month_t    = std::uint8_t; // as in std::chrono::month
using day_t      = std::uint8_t; // as in std::chrono::day
using rata_die_t = std::int32_t; // as in std::chrono::days

auto constexpr disable_static_asserts = false;
auto constexpr test_baum              = true;
auto constexpr test_dotnet            = true;
auto constexpr test_reingold          = true;
auto constexpr test_glibc             = true;
auto constexpr test_hatcher           = true;

//--------------------------------------------------------------------------------------------------
// Other implementations
//--------------------------------------------------------------------------------------------------

struct other_base {
  using year_t     = std::int16_t; // as in std::chrono::year
  using month_t    = std::uint8_t; // as in std::chrono::month
  using day_t      = std::uint8_t; // as in std::chrono::day
  using rata_die_t = std::int32_t; // as in std::chrono::days
  using date_t     = ::date_t<year_t>;
};

struct baum : other_base {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{0, 3, 1};
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -719468;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = date_t{0, 3, 1};
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -719468;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // https://www.researchgate.net/publication/316558298_Date_Algorithms

  // Section 5.1
  rata_die_t static constexpr
  to_rata_die(const date_t& u) noexcept {
    auto const j = u.month < 3;
    auto const z = u.year - j;                                      // step 1 / alternative 2
    auto const m = j ? u.month + 12 : u.month;                      // step 2 / alternative 3
    auto const f = (979 * m - 2918) / 32;                           //
    auto const n = u.day + f + 365 * z + z / 4 - z / 100 + z / 400; // step 3
    return rata_die_t(n - 719469);                                  // adjusted to unix epoch
  }

  // Section 6.2.1/3
  date_t static constexpr
  to_date(rata_die_t n) noexcept {
    auto const z  = std::uint32_t(n) + 719469; // adjusted to unix epoch
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
    return { year_t(y), month_t(m), day_t(d) };
  }

}; // struct baum

struct dotnet : other_base {

  // Code of this stuct is subject to the following terms.

  // The MIT License (MIT)

  // Copyright (c) .NET Foundation and Contributors

  // All rights reserved.

  // Permission is hereby granted, free of charge, to any person obtaining a copy
  // of this software and associated documentation files (the "Software"), to deal
  // in the Software without restriction, including without limitation the rights
  // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  // copies of the Software, and to permit persons to whom the Software is
  // furnished to do so, subject to the following conditions:

  // The above copyright notice and this permission notice shall be included in all
  // copies or substantial portions of the Software.

  // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  // SOFTWARE.

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{1, 1, 1};
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -719162;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = date_t{1, 1, 1};
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -719162;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // https://github.com/dotnet/runtime/blob/master/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L102
  rata_die_t static constexpr s_daysToMonth365[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
  rata_die_t static constexpr s_daysToMonth366[] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

  //https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L1123
  bool static constexpr
  IsLeapYear(year_t year) noexcept {
    return (year & 3) == 0 && ((year & 15) == 0 || (year % 25) != 0);
  }

  //https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L625
  rata_die_t static constexpr
  to_rata_die(const date_t& date) noexcept {
    rata_die_t const* days = IsLeapYear(date.year) ? s_daysToMonth366 : s_daysToMonth365;
    rata_die_t y = date.year - 1;
    rata_die_t n = y * 365 + y / 4 - y / 100 + y / 400 + days[date.month - 1] + date.day - 1;
    return n - 719162; // adjusted to unix epoch
  }

  // https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L64
  rata_die_t static constexpr DaysPerYear = 365;
  rata_die_t static constexpr DaysPer4Years = DaysPerYear * 4 + 1;       // 1461
  rata_die_t static constexpr DaysPer100Years = DaysPer4Years * 25 - 1;  // 36524
  rata_die_t static constexpr DaysPer400Years = DaysPer100Years * 4 + 1; // 146097

  // https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L938
  date_t static constexpr
  to_date(rata_die_t rata_die) noexcept {
    rata_die_t n = rata_die + 719162; // adjusted to unix epoch
    rata_die_t y400 = n / DaysPer400Years;
    n -= y400 * DaysPer400Years;
    rata_die_t y100 = n / DaysPer100Years;
    if (y100 == 4) y100 = 3;
    n -= y100 * DaysPer100Years;
    rata_die_t y4 = n / DaysPer4Years;
    n -= y4 * DaysPer4Years;
    rata_die_t y1 = n / DaysPerYear;
    if (y1 == 4) y1 = 3;
    year_t year = y400 * 400 + y100 * 100 + y4 * 4 + y1 + 1;
    n -= y1 * DaysPerYear;
    bool leapYear = y1 == 3 && (y4 != 24 || y100 == 3);
    rata_die_t const* days = leapYear ? s_daysToMonth366 : s_daysToMonth365;
    month_t m = (n >> 5) + 1;
    while (n >= days[m]) m++;
    month_t month = m;
    day_t day = n - days[m - 1] + 1;
    return date_t{year, month, day};
  }

}; // struct dotnet

struct reingold : other_base {

  date_t     static constexpr epoch              = date_t{0, 12, 31};

  date_t     static constexpr date_min           = date_t{0, 3, 1};
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -305;
  rata_die_t static constexpr rata_die_max       = 11967900;

  date_t     static constexpr round_date_min     = date_t{0, 3, 1};
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -305;
  rata_die_t static constexpr round_rata_die_max = 11967900;

  // E. M. Reingold and N. Dershowitz, Calendrical Calculations, The Ultimate Edition, Cambridge
  // University Press, 2018.

  // Table 1.2, page 17.
  rata_die_t static constexpr gregorian_epoch = 1;

  // alt-fixed-from-gregorian, equation (2.28), page 65:
  rata_die_t static constexpr
  to_rata_die(date_t date) noexcept {

    auto const year  = rata_die_t(date.year );
    auto const month = rata_die_t(date.month);
    auto const day   = rata_die_t(date.day  );

    // In the book mp = (month - 3) mod 12, where mod denotes Euclidean remainder. When month < 3 we
    // have month - 3 < 0 and % does not match mod. The alternative below provides the intended
    // result even in this case and keeps the expected performance of the original formula.
    auto const mp = (month + 9) % 12;
    auto const yp = year - mp / 10;

    // Equation (1.42), page 28, with b = <4, 25, 4>, i.e., b0 = 4, b1 = 25 and b2 = 4 gives
    auto const a0 = (yp / 400);
    auto const a1 = (yp / 100) %  4;
    auto const a2 = (yp /   4) % 25;
    auto const a3 = (yp /   1) %  4;
    // On page 66, quantities above are denoted by n400, n100, n4 and n1.

    return gregorian_epoch - 1 - 306 + 365 * yp + 97 * a0 + 24 * a1 + 1 * a2 + 0 * a3 +
      (3 * mp + 2) / 5 + 30 * mp + day;
  }

  // gregorian-year-from-fixed, equation (2.21), page 61:
  rata_die_t static constexpr
  gregorian_year_from_fixed(rata_die_t date) noexcept {
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

  // alt-fixed-from-gregorian, equation (2.28), page 65:
  rata_die_t static constexpr
  fixed_from_gregorian(date_t date) noexcept {
    return to_rata_die(date);
  }

  rata_die_t static constexpr
  mod_1_12(rata_die_t month) noexcept {
    return month > 12 ? month - 12 : month;
  }

  // alt-gregorian-from-fixed, equation (2.29), page 66:
  date_t static constexpr
  to_date(rata_die_t date) noexcept {
    auto const y          = gregorian_year_from_fixed(gregorian_epoch - 1 + date + 306);
    auto const prior_days = date - fixed_from_gregorian(date_t{year_t(y - 1), 3, 1});
    auto const month      = mod_1_12((5 * prior_days + 2) / 153 + 3);
    auto const year       = y - (month + 9) / 12;
    auto const day        = date - fixed_from_gregorian(date_t{year_t(year), month_t(month), 1})
      + 1;
    return { year_t(year), month_t(month), day_t(day) };
  }

}; // struct reingold

struct glibc : other_base {

  // Code of this stuct is subject to the following terms.

  // Copyright (C) 1993-2020 Free Software Foundation, Inc.

  // This section of the file is part of the GNU C Library.
  // Contributed by Paul Eggert <eggert@twinsun.com>.
  // The GNU C Library is free software; you can redistribute it and/or
  // modify it under the terms of the GNU Lesser General Public
  // License as published by the Free Software Foundation; either
  // version 2.1 of the License, or (at your option) any later version.

  // The GNU C Library is distributed in the hope that it will be useful,
  // but WITHOUT ANY WARRANTY; without even the implied warranty of
  // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  // Lesser General Public License for more details.

  // You should have received a copy of the GNU Lesser General Public
  // License along with the GNU C Library; if not, see
  // <https://www.gnu.org/licenses/>.

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = min<date_t>;
  date_t     static constexpr date_max           = max<date_t>;
  rata_die_t static constexpr rata_die_min       = -12687794;
  rata_die_t static constexpr rata_die_max       = 11248737;

  date_t     static constexpr round_date_min     = min<date_t>;
  date_t     static constexpr round_date_max     = max<date_t>;
  rata_die_t static constexpr round_rata_die_min = -12687794;
  rata_die_t static constexpr round_rata_die_max = 11248737;

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l138
  rata_die_t static constexpr
  shr(rata_die_t a, int b) noexcept {
    rata_die_t one = 1;
    return (-one >> 1 == -1 ? a >> b : a / (one << b) - (a % (one << b) < 0));
  }

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l157
  #define EPOCH_YEAR 1970
  #define TM_YEAR_BASE 1900

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l161
  bool static constexpr
  leapyear (rata_die_t year) noexcept {
    return ((year & 3) == 0 && (year % 100 != 0 ||
      ((year / 100) & 3) == (- (TM_YEAR_BASE / 100) & 3)));
  }

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l173
  unsigned short int static constexpr __mon_yday[2][13] =
    {
      /* Normal years.  */
      { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
      /* Leap years.  */
      { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
    };

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l194
  rata_die_t static constexpr
  ydhms_diff (rata_die_t year1, rata_die_t yday1, rata_die_t year0) noexcept {
    rata_die_t a4 = shr (year1, 2) + shr (TM_YEAR_BASE, 2) - ! (year1 & 3);
    rata_die_t b4 = shr (year0, 2) + shr (TM_YEAR_BASE, 2) - ! (year0 & 3);
    rata_die_t a100 = a4 / 25 - (a4 % 25 < 0);
    rata_die_t b100 = b4 / 25 - (b4 % 25 < 0);
    rata_die_t a400 = shr (a100, 2);
    rata_die_t b400 = shr (b100, 2);
    rata_die_t intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);
    rata_die_t years = year1 - year0;
    rata_die_t days = 365 * years + yday1 + intervening_leap_days;
    return days;
  }

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l312
  rata_die_t static constexpr
  to_rata_die(date_t date) noexcept {
    rata_die_t mday     = date.day;
    rata_die_t mon      = date.month - 1;
    rata_die_t year     = date.year - TM_YEAR_BASE;
    rata_die_t mon_yday = (__mon_yday[leapyear(year)][mon]) - 1;
    rata_die_t yday     = mon_yday + mday;
    rata_die_t t0       = ydhms_diff (year, yday, EPOCH_YEAR - TM_YEAR_BASE);
    return t0;
  }

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/time.h;h=015bc1c7f3b5d3db689f68de2a0c6ebbbc94f138#l179
  #define __isleap(year)	\
    ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/offtime.c;h=1415b1b4013834f39e4b0c11f0479dd866aab617#l24
  date_t static constexpr
  to_date(rata_die_t days) noexcept {

    rata_die_t y = 1970;
    #define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
    #define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

    while (days < 0 || days >= (__isleap (y) ? 366 : 365))
      {
        /* Guess a corrected year, assuming 365 days per year.  */
        rata_die_t yg = y + days / 365 - (days % 365 < 0);

        /* Adjust DAYS and Y to match the guessed year.  */
        days -= ((yg - y) * 365
                + LEAPS_THRU_END_OF (yg - 1)
                - LEAPS_THRU_END_OF (y - 1));
        y = yg;
      }

    auto ip = __mon_yday[__isleap(y)];
    rata_die_t m = 0;
    for (m = 11; days < (long int) ip[m]; --m)
      continue;
    days -= ip[m];
    return date_t{year_t(y), month_t(m + 1), day_t(days + 1)};
  }

}; // struct glibc

struct hatcher : other_base {

  date_t     static constexpr epoch              = unix_epoch<year_t>;

  date_t     static constexpr date_min           = date_t{1900, 3,  1};
  date_t     static constexpr date_max           = date_t{2100, 2, 28};
  rata_die_t static constexpr rata_die_min       = -25495;
  rata_die_t static constexpr rata_die_max       = 47540;

  date_t     static constexpr round_date_min     = date_t{1900, 3, 14};
  date_t     static constexpr round_date_max     = date_t{2100, 2, 28};
  rata_die_t static constexpr round_rata_die_min = -25495;
  rata_die_t static constexpr round_rata_die_max = 47540;

  // Algorithms by D. A. Hactcher as appeared in
  // E. G. Richards, Mapping Time, The calendar and its history, Oxford University Press, 1998.

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
  to_rata_die(const date_t& x) noexcept {
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
    return J - rata_die_t(2440575); // adjusted to unix epoch
  }

  // Algorithm F, page 324.
  date_t static constexpr
  to_date(rata_die_t x) noexcept {
    auto const J  = x + rata_die_t(2440575); // adjusted to unix epoch
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
    return date_t{year_t(Y), month_t(M), day_t(D)};
  }

}; // struct hatcher

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * Advance date by one day.
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
 * Return next date.
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
 * Regress date by one day.
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
 * Return previos date.
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
// Information
//--------------------------------------------------------------------------------------------------

void
print_banner(char const* banner) {
  std::cout << "------------------------------------------\n";
  std::cout << banner << '\n';
  std::cout << "------------------------------------------\n";
}

template <typename Algo>
void
print_info() {

  std::cout << "epoch              = " << Algo::epoch              << '\n';

  std::cout << "date_min           = " << Algo::date_min           << '\n';
  std::cout << "date_max           = " << Algo::date_max           << '\n';

  std::cout << "rata_die_min       = " << Algo::rata_die_min       << '\n';
  std::cout << "rata_die_max       = " << Algo::rata_die_max       << '\n';

  std::cout << "round_date_min     = " << Algo::round_date_min     << '\n';
  std::cout << "round_date_max     = " << Algo::round_date_max     << '\n';

  std::cout << "round_rata_die_min = " << Algo::round_rata_die_min << '\n';
  std::cout << "round_rata_die_max = " << Algo::round_rata_die_max << '\n';
}

//--------------------------------------------------------------------------------------------------
// Static tests
//--------------------------------------------------------------------------------------------------

void constexpr
standard_compliance_test() noexcept {

  using year_t     = std::int16_t; // as in std::chrono::year
  using month_t    = std::uint8_t; // as in std::chrono::month
  using day_t      = std::uint8_t; // as in std::chrono::day
  using rata_die_t = std::int32_t; // as in std::chrono::days

  using gregorian_t = ::gregorian_t<year_t, rata_die_t>;

  // https://eel.is/c++draft/time.clock.system#overview-1
  static_assert(disable_static_asserts ||
    unix_epoch<year_t> == date_t<year_t>{1970, 1, 1});
  static_assert(disable_static_asserts ||
    gregorian_t::to_date(0) == unix_epoch<year_t>);

  // https://eel.is/c++draft/time.cal.ymd#members-20
  static_assert(disable_static_asserts ||
    gregorian_t::round_rata_die_min <= -12687428);
  static_assert(disable_static_asserts ||
    gregorian_t::round_rata_die_max >=  11248737);
}

void constexpr
month_functions_test() {
  auto constexpr f = [](rata_die_t x) { return (979 * x - 2922) / 32; };
  auto constexpr g = [](rata_die_t r) { return 2141 * r + 197657; };
  #define MONTH_TEST(x)          \
    static_assert(disable_static_asserts || g(f(x)) / 65536 == x); \
    static_assert(disable_static_asserts || g(f(x) - 1) / 65536 == x - 1)
  MONTH_TEST( 3);
  MONTH_TEST( 4);
  MONTH_TEST( 5);
  MONTH_TEST( 6);
  MONTH_TEST( 7);
  MONTH_TEST( 8);
  MONTH_TEST( 9);
  MONTH_TEST(10);
  MONTH_TEST(11);
  MONTH_TEST(12);
  MONTH_TEST(13);
  MONTH_TEST(14);
  MONTH_TEST(15);
  #undef MONTH_TEST
}

//--------------------------------------------------------------------------------------------------
// Dynamic tests
//--------------------------------------------------------------------------------------------------

void
is_multiple_of_100_test() {

  std::cout << "test_is_multiple_of_100... ";

  for (std::int32_t n = -536870800; n <= 536870999; ++n)
    if ((n % 100 == 0) != is_multiple_of_100(n)) {
      std::cout << "failed for n = " << n << '\n';
      return;
    }

  std::cout << "OK\n";
}

template <typename A>
void
round_trip_test() {

  std::cout << "round_trip_test... ";

  // Compile-time checks.

  static_assert(disable_static_asserts ||
    A::round_rata_die_min == A::to_rata_die(A::round_date_min));
  static_assert(disable_static_asserts ||
    A::round_rata_die_max == A::to_rata_die(A::round_date_max));

  static_assert(disable_static_asserts ||
    A::round_date_min == A::to_date(A::round_rata_die_min));
  static_assert(disable_static_asserts ||
    A::round_date_max == A::to_date(A::round_rata_die_max));

  // Runtime checks.

  for (auto n = A::round_rata_die_min; n <= A::round_rata_die_max; ++n)
    if (n != A::to_rata_die(A::to_date(n))) {
      std::cout << "failed for n = " << n << '\n';
      return;
    }

  std::cout << "OK\n";
}

template <typename A>
void
to_date_test() {

  std::cout << "to_date_test... ";

  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  auto constexpr first = A::to_date(A::rata_die_min);
  static_assert(disable_static_asserts ||
  // dotnet needs special treatment: rata_die_t is signed but rata_die_min == 0
    (A::rata_die_min == min<rata_die_t> || std::is_same_v<A, dotnet>) ||
    first == min<date_t> || A::to_date(A::rata_die_min - 1) != previous(first));

  auto constexpr last = A::to_date(A::rata_die_max);
  static_assert(disable_static_asserts ||
    A::rata_die_max == max<rata_die_t> || last == max<date_t> ||
    A::to_date(A::rata_die_max + 1) != next(last));

  date_t date;

  // Move forward: from 0 to rata_die_max.
  // Fails if rata_die_max is too large (shows correct value plus one).
  date = A::epoch;
  for (rata_die_t rata_die = 0; rata_die < A::rata_die_max; ) {

    auto const tomorrow = A::to_date(++rata_die);

    if (date == max<date_t>) {
      std::cout << "(forward) failed for rata_die = " << rata_die << " (date == max<date_t>).\n";
      return;
    }

    if (tomorrow != advance(date)) {
      std::cout << "(forward) failed for rata_die = " << rata_die << '\n';
      return;
    }
  }

  // Move backward: from 0 to rata_die_min.
  // Fails if rata_die_min is too small (shows the correct value minus one).
  date = A::epoch;
  for (rata_die_t rata_die = 0; A::rata_die_min < rata_die; ) {

    auto const yesterday = A::to_date(--rata_die);

    if (date == min<date_t>) {
      std::cout << "(backward) failed for rata_die = " << rata_die << " (date == min<date_t>).\n";
      return;
    }

    if (yesterday != regress(date)) {
      std::cout << "(backward) failed for rata_die = " << rata_die << '\n';
      return;
    }
  }

  std::cout << "OK\n";
}

template <typename A>
void
to_rata_die_test() {

  std::cout << "to_rata_die_test... ";

  using date_t     = typename A::date_t;
  using rata_die_t = typename A::rata_die_t;

  auto constexpr first = A::to_rata_die(A::date_min);
  static_assert(disable_static_asserts ||
    A::date_min == min<date_t> || first == min<rata_die_t> ||
    A::to_rata_die(previous(A::date_min)) != first - 1);

  auto constexpr last = A::to_rata_die(A::date_max);
  static_assert(disable_static_asserts ||
    A::date_max == max<date_t> || last == max<rata_die_t> ||
    A::to_rata_die(next(A::date_max)) != last + 1);

  rata_die_t rata_die;

  // Move forward: from epoch to date_max.
  // Fails if date_max is too large (shows correct value plus one day).
  rata_die = 0;
  for (auto date = A::epoch; date < A::date_max; ) {

    auto const tomorrow = A::to_rata_die(advance(date));

    if (rata_die == max<rata_die_t>) {
      std::cout << "(forward) failed for date = " << date << " (rata die == max<rata_die_t>).\n";
      return;
    }

    if (tomorrow != ++rata_die) {
      std::cout << "(forward) failed for date = " << date << '\n';
      return;
    }
  }

  // Move backward: from epoch to date_min.
  // Fails if date_min is too small (shows the correct value minus one day).
  rata_die = 0;
  for (auto date = A::epoch; A::date_min < date; ) {

    auto const yesterday = A::to_rata_die(regress(date));

    if (rata_die == min<rata_die_t>) {
      std::cout << "(backward) failed for date = " << date << " (rata die == min<rata_die_t>).\n";
      return;
    }

    if (yesterday != --rata_die) {
      std::cout << "(backward) failed for date = " << date << '\n';
      return;
    }
  }

  std::cout << "OK\n";
}

template <typename A>
void
calendar_tests(char const* banner) {
  print_banner(banner);
  print_info<A>();
  to_date_test<A>();
  to_rata_die_test<A>();
  round_trip_test<A>();
}

int
main() {

  print_banner("Preliminary tests");
  is_multiple_of_100_test();

  if (test_baum)
    calendar_tests<baum>("Baum tests");

  if (test_dotnet)
    calendar_tests<dotnet>("DotNet tests");

   if (test_reingold)
     calendar_tests<reingold>("Reingold tests");

  if (test_glibc)
    calendar_tests<glibc>("glibc tests");

  if (test_hatcher)
    calendar_tests<hatcher>("Hatcher tests");

  // 16 bits

  calendar_tests<ugregorian_t<std::uint16_t, std::uint32_t>>
    ("unsigned : 16");

  calendar_tests<gregorian_t<std::int16_t, std::int32_t>>
    ("signed : 16 : default epoch");

  calendar_tests<gregorian_t<std::int16_t, std::int32_t, date_t<std::int16_t>{0, 3, 1}>>
    ("signed : 16 : 0000-Mar-01");

  calendar_tests<gregorian_t<std::int16_t, std::int32_t, date_t<std::int16_t>{0, 1, 1}>>
    ("signed : 16 : 0000-Jan-01");

  calendar_tests<gregorian_t<std::int16_t, std::int32_t, date_t<std::int16_t>{-1, 1, 1}>>
    ("signed : 16 : -0001-Jan-01");

  calendar_tests<gregorian_t<std::int16_t, std::int32_t, date_t<std::int16_t>{-400, 1, 1}>>
    ("signed : 16 : -0400-Jan-01");

  calendar_tests<gregorian_t<std::int16_t, std::int32_t, date_t<std::int16_t>{-1970, 1, 1}>>
    ("signed : 16 : -1970-Jan-01");

  calendar_tests<gregorian_t<std::int16_t, std::int32_t, date_t<std::int16_t>{-32768, 1, 1}>>
    ("signed : 16 : -32768-Jan-01");

  // 32 bits

  calendar_tests<ugregorian_t<std::uint32_t, std::uint32_t>>
    ("unsigned : 32");

  calendar_tests<gregorian_t<std::int32_t, std::int32_t>>
    ("signed : 32 : default epoch");

  calendar_tests<gregorian_t<std::int32_t, std::int32_t, date_t<std::int32_t>{1912, 6, 23}>>
    ("signed : 32 : 1912-Jun-23");

  calendar_tests<gregorian_t<std::int32_t, std::int32_t, date_t<std::int32_t>{-1912, 6, 23}>>
    ("signed : 32 : -1912-Jun-23");
}
