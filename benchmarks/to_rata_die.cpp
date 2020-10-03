/*******************************************************************************
 *
 * to_rata_die benchmarks
 *
 * Copyright (C) 2020 Cassio Neri and Lorenz Schneider
 *
 * This file is part of https://github.com/cassioneri/calendar.
 *
 * This file is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this file. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <cstdint>
#include <random>
#include <type_traits>

//------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------

using year_t     = std::int16_t; // as in std::chrono::year
using month_t    = std::uint8_t; // as in std::chrono::month
using day_t      = std::uint8_t; // as in std::chrono::day
using rata_die_t = std::int32_t; // as in std::chrono::days

//------------------------------------------------------------------------------
// Implementations
//------------------------------------------------------------------------------

struct date_t {
  year_t  year;
  month_t month;
  day_t   day;
};

namespace neri_schneider {

  // https://github.com/cassioneri/calendar/blob/master/calendar.hpp

  rata_die_t constexpr
  to_rata_die(date_t const& u2) noexcept {
    
    using rata_die_t     = std::make_unsigned_t<::rata_die_t>;
    auto constexpr z2    = rata_die_t(-1468000);
    auto constexpr n2_e3 = rata_die_t(536895458);
    
    auto const     y1    = rata_die_t(u2.year) - z2;
    auto const     m1    = rata_die_t(u2.month);
    auto const     d1    = rata_die_t(u2.day);
    
    auto const     j     = rata_die_t(m1 < 3);
    auto const     y     = y1 - j;
    auto const     m     = j ? m1 + 12 : m1;
    auto const     d     = d1 - 1;
    
    auto const     q1    = y / 100;
    auto const     yc    = 1461 * y / 4 - q1 + q1 / 4;
    auto const     mc    = (979 * m - 2922) / 32;
    auto const     dc    = d;
    
    auto const     n3    = yc + mc + dc - n2_e3;
    
    return n3;
  }

} // namespace neri_schneider

namespace baum {

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

} // namespace baum

namespace boost {

  // Code in this namespace is subject to the following terms.

  // Copyright (c) 2002,2003 CrystalClear Software, Inc.

  // Boost Software License - Version 1.0 - August 17th, 2003

  // Permission is hereby granted, free of charge, to any person or organization
  // obtaining a copy of the software and accompanying documentation covered by
  // this license (the "Software") to use, reproduce, display, distribute,
  // execute, and transmit the Software, and to prepare derivative works of the
  // Software, and to permit third-parties to whom the Software is furnished to
  // do so, all subject to the following:

  // The copyright notices in the Software and this entire statement, including
  // the above license grant, this restriction and the following disclaimer,
  // must be included in all copies of the Software, in whole or in part, and
  // all derivative works of the Software, unless such copies or derivative
  // works are solely in the form of machine-executable object code generated by
  // a source language processor.

  // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  // FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
  // SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
  // FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
  // ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  // DEALINGS IN THE SOFTWARE.

  // https://github.com/boostorg/date_time/blob/4e1b7cde45edf8fdda73ec5c60053c9257138292/include/boost/date_time/gregorian_calendar.ipp#L68
  rata_die_t constexpr
  to_rata_die(const date_t& ymd) noexcept {
    unsigned short a = static_cast<unsigned short>((14-ymd.month)/12);
    unsigned short y = static_cast<unsigned short>(ymd.year + 4800 - a);
    unsigned short m = static_cast<unsigned short>(ymd.month + 12*a - 3);
    unsigned long  d = ymd.day + ((153*m + 2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045;
    return static_cast<rata_die_t>(d);
  }

} // namespace boost

namespace dotnet {

  // Code in this namespace is subject to the following terms.

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

  // https://github.com/dotnet/runtime/blob/master/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L102
  rata_die_t static constexpr s_daysToMonth365[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
  rata_die_t static constexpr s_daysToMonth366[] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

  // https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L1123
  bool static constexpr
  IsLeapYear(year_t year) noexcept {
    return (year & 3) == 0 && ((year & 15) == 0 || (year % 25) != 0);
  }

  // https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L625
  rata_die_t static constexpr
  to_rata_die(const date_t& date) noexcept {
    rata_die_t const* days = IsLeapYear(date.year) ? s_daysToMonth366 : s_daysToMonth365;
    rata_die_t y = date.year - 1;
    rata_die_t n = y * 365 + y / 4 - y / 100 + y / 400 + days[date.month - 1] + date.day - 1;
    return n - 719162; // adjusted to unix epoch
  }

} // namespace dotnet

namespace glibc {

  // Code in this namespace is subject to the following terms.

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
  to_rata_die(const date_t& date) noexcept {
    rata_die_t mday     = date.day;
    rata_die_t mon      = date.month - 1;
    rata_die_t year     = date.year - TM_YEAR_BASE;
    rata_die_t mon_yday = (__mon_yday[leapyear(year)][mon]) - 1;
    rata_die_t yday     = mon_yday + mday;
    rata_die_t t0       = ydhms_diff (year, yday, EPOCH_YEAR - TM_YEAR_BASE);
    return t0;
  }

} // namespace glibc

namespace hatcher {

  // Algorithms by D. A. Hactcher as appeared in
  // E. G. Richards, Mapping Time, The calendar and its history, Oxford University Press, 1998.

  // Table 25.1, page 311.
  auto static constexpr y = rata_die_t(4716);
  auto static constexpr m = rata_die_t(3);
  auto static constexpr n = rata_die_t(12);
  auto static constexpr r = rata_die_t(4);
  auto static constexpr p = rata_die_t(1461);
  auto static constexpr q = rata_die_t(0);
  auto static constexpr u = rata_die_t(5);
  auto static constexpr s = rata_die_t(153);
  auto static constexpr t = rata_die_t(2);

  // Table 25.4, page 320.
  auto static constexpr A = rata_die_t(184);
  auto static constexpr G = rata_die_t(-38);

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

} // namespace hatcher

namespace llvm {

  // Code in this namespace is subject to the following terms.

  // Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
  // See https://llvm.org/LICENSE.txt for license information.

  // https://github.com/llvm/llvm-project/blob/8e34be2f2511dfff7a8e3018bbd4188a93e446ea/libcxx/include/chrono#L2325
  rata_die_t constexpr
  to_rata_die(const date_t& date) noexcept {
    const int      __yr  = static_cast<int>(date.year) - (date.month <= 2);
    const unsigned __mth = static_cast<unsigned>(date.month);
    const unsigned __dy  = static_cast<unsigned>(date.day);
    const int      __era = (__yr >= 0 ? __yr : __yr - 399) / 400;
    const unsigned __yoe = static_cast<unsigned>(__yr - __era * 400);                // [0, 399]
    const unsigned __doy = (153 * (__mth + (__mth > 2 ? -3 : 9)) + 2) / 5 + __dy-1;  // [0, 365]
    const unsigned __doe = __yoe * 365 + __yoe/4 - __yoe/100 + __doy;                // [0, 146096]
    return rata_die_t{__era * 146097 + static_cast<int>(__doe) - 719468};
  }

} // namespace llvm

namespace reingold_dershowitz {

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

    auto const n = gregorian_epoch - 1 - 306 + 365 * yp + 97 * a0 + 24 * a1 + 1 * a2 + 0 * a3 +
      (3 * mp + 2) / 5 + 30 * mp + day;
    return n - 719163; // adjusted to unix epoch
  }

} // namespace reingold_dershowitz

//------------------------------------------------------------------------------
// Benchmark data
//------------------------------------------------------------------------------

date_t constexpr
to_date(rata_die_t n3) noexcept {

  using rata_die_t     = std::make_unsigned_t<::rata_die_t>;
  auto constexpr z2    = rata_die_t(-1468000);
  auto constexpr n2_e3 = rata_die_t(536895458);

  auto const     n0    = n3 + n2_e3;
  
  auto const     p1    = 4 * n0 + 3;
  auto const     q1    = p1 / 146097;
  auto const     r1    = p1 % 146097 / 4;
  
  auto constexpr p32   = std::uint64_t(1) << 32;
  auto const     p2    = 4 * r1 + 3;
  auto const     x2    = std::uint64_t(2939745) * p2;
  auto const     q2    = rata_die_t(x2 / p32);
  auto const     r2    = rata_die_t(x2 % p32 / 2939745 / 4);
  
  auto constexpr p16   = std::uint32_t(1) << 16;
  auto const     p3    = 2141 * r2 + 197657;
  auto const     q3    = p3 / p16;
  auto const     r3    = p3 % p16 / 2141;
  
  auto const     y     = 100 * q1 + q2;
  auto const     m     = q3;
  auto const     d     = r3;
  
  auto const     j     = r2 > 305;
  auto const     y1    = y + j;
  auto const     m1    = j ? m - 12 : m;
  auto const     d1    = d + 1;
  
  return { year_t(y1 + z2), month_t(m1), day_t(d1) };
}

auto const dates = [](){
  std::uniform_int_distribution<rata_die_t> uniform_dist(-146097, 146096);
  std::mt19937 rng;
  std::array<date_t, 16384> dates;
  for (auto& u : dates)
    u = to_date(uniform_dist(rng));
  return dates;
}();

//------------------------------------------------------------------------------
// Benchmark
//------------------------------------------------------------------------------

void ReingoldDershowitz(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& u : dates) {
      auto n = reingold_dershowitz::to_rata_die(u);
      benchmark::DoNotOptimize(n);
    }
  }
}
BENCHMARK(ReingoldDershowitz);

void GLIBC(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& u : dates) {
      auto n = glibc::to_rata_die(u);
      benchmark::DoNotOptimize(n);
    }
  }
}
BENCHMARK(GLIBC);

void Hatcher(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& u : dates) {
      auto n = hatcher::to_rata_die(u);
      benchmark::DoNotOptimize(n);
    }
  }
}
BENCHMARK(Hatcher);

void DotNet(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& u : dates) {
      auto n = dotnet::to_rata_die(u);
      benchmark::DoNotOptimize(n);
    }
  }
}
BENCHMARK(DotNet);

void Boost(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& u : dates) {
      auto n = boost::to_rata_die(u);
      benchmark::DoNotOptimize(n);
    }
  }
}
BENCHMARK(Boost);

void LLVM(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& u : dates) {
      auto n = llvm::to_rata_die(u);
      benchmark::DoNotOptimize(n);
    }
  }
}
BENCHMARK(LLVM);

void Baum(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& u : dates) {
      auto n = baum::to_rata_die(u);
      benchmark::DoNotOptimize(n);
    }
  }
}
BENCHMARK(Baum);

void NeriSchneider(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& u : dates) {
      auto n = neri_schneider::to_rata_die(u);
      benchmark::DoNotOptimize(n);
    }
  }
}
BENCHMARK(NeriSchneider);
