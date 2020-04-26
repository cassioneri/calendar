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

using year_t     = std::int32_t; // as in std::chrono::year
using month_t    = std::uint8_t; // as in std::chrono::month
using day_t      = std::uint8_t; // as in std::chrono::day
using rata_die_t = std::int32_t;

//--------------------------------------------------------------------------------------------------
// Other implementations
//--------------------------------------------------------------------------------------------------

namespace others {

using date_t = ::date_t<year_t>;

namespace hinnant {

// __from_days : https://github.com/llvm/llvm-project/blob/master/libcxx/include/chrono
date_t constexpr to_date(rata_die_t __d) noexcept {
  const int      __z = __d + 719468;
  const int      __era = (__z >= 0 ? __z : __z - 146096) / 146097;
  const unsigned __doe = static_cast<unsigned>(__z - __era * 146097);              // [0, 146096]
  const unsigned __yoe = (__doe - __doe/1460 + __doe/36524 - __doe/146096) / 365;  // [0, 399]
  const int      __yr = static_cast<int>(__yoe) + __era * 400;
  const unsigned __doy = __doe - (365 * __yoe + __yoe/4 - __yoe/100);              // [0, 365]
  const unsigned __mp = (5 * __doy + 2)/153;                                       // [0, 11]
  const unsigned __dy = __doy - (153 * __mp + 2)/5 + 1;                            // [1, 31]
  const unsigned __mth = __mp + (__mp < 10 ? 3 : -9);                              // [1, 12]
  return date_t{year_t(__yr + (__mth <= 2)), month_t(__mth), day_t(__dy)};
}

// __to_days : https://github.com/llvm/llvm-project/blob/master/libcxx/include/chrono
rata_die_t constexpr to_rata_die(date_t date) noexcept {
  const int      __yr  = static_cast<int>(date.year) - (date.month <= 2);
  const unsigned __mth = static_cast<unsigned>(date.month);
  const unsigned __dy  = static_cast<unsigned>(date.day);
  const int      __era = (__yr >= 0 ? __yr : __yr - 399) / 400;
  const unsigned __yoe = static_cast<unsigned>(__yr - __era * 400);                // [0, 399]
  const unsigned __doy = (153 * (__mth + (__mth > 2 ? -3 : 9)) + 2) / 5 + __dy-1;  // [0, 365]
  const unsigned __doe = __yoe * 365 + __yoe/4 - __yoe/100 + __doy;                // [0, 146096]
  return rata_die_t{__era * 146097 + static_cast<int>(__doe) - 719468};
}

} // namespace hinnant

namespace baum {

// Section 6.2.1/3 : https://www.researchgate.net/publication/316558298_Date_Algorithms
date_t constexpr to_date(rata_die_t rata_die) noexcept {
  auto const z      = std::uint32_t(rata_die) + 719103; // adjusted to unix epoch
  auto const h      = 100 * z + 25;
  auto const a      = h / 3652425;
  auto const b      = a - a / 4;
  auto const year_  = (100 * b + h) / 36525;
  auto const c      = b + z - 365 * year_ - year_ / 4;
  auto const month_ = (535 * c + 48950) / 16384;
  auto const day    = c - (979 * month_ - 2918) / 32;
  auto const jof    = month_ > 12;
  auto const year   = year_ + jof;
  auto const month  = jof ? month_ - 12 : month_;
  return { year_t(year), month_t(month), day_t(day) };
}

// Section 5.1 : https://www.researchgate.net/publication/316558298_Date_Algorithms
rata_die_t constexpr to_rata_die(date_t date) noexcept {
  auto const jof = date.month < 3;
  auto const z  = date.year - jof;                // step 1 / alternative 2
  auto const f  = (979 * date.month - 2918) / 32; // step 2 / alternative 3
  return rata_die_t{date.day + f +                // step 3 (adjusted to unix epoch)
    365 * z + z / 4 - z / 100 + z / 400 - 719103};
}

} // namespace baum

} // namespace others

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * Advance date by one day.
 *
 * @param date        Date to be advanced.
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
 */
template <typename T>
date_t<T> constexpr
next(date_t<T> date) noexcept {
  return advance(date);
}

/**
 * Return previous date.
 *
 * @param date        Date to be regressed.
 */
template <typename T>
date_t<T> constexpr
previous(date_t<T> date) noexcept {
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

//--------------------------------------------------------------------------------------------------
// Information
//--------------------------------------------------------------------------------------------------

template <typename Algo>
void
print() {

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
baum_epoch_test() {
  static_assert(others::baum::to_rata_die(unix_epoch<year_t>) == 0);
}

void constexpr
standard_compliance_test() noexcept {

  using algos = sdate_algos<year_t, rata_die_t, unix_epoch<year_t>>;

  // https://eel.is/c++draft/time.clock.system#overview-1
  static_assert(unix_epoch<year_t> == date_t<year_t>{1970, 1, 1});
  static_assert(algos::to_date(0) == unix_epoch<year_t>);

  // https://eel.is/c++draft/time.cal.ymd#members-20
  static_assert(algos::round_rata_die_min <= -12687428);
  static_assert(algos::round_rata_die_max >=  11248737);
}

void constexpr
get_month_test() {
  auto constexpr f = [](std::uint32_t n) { return (535 * n + 49483) / 16384; };
  #define GET_MONTH_TEST(m, b, e) static_assert(f(b) == m && f(e) == m)
  GET_MONTH_TEST( 3,   0,  30);
  GET_MONTH_TEST( 4,  31,  60);
  GET_MONTH_TEST( 5,  61,  91);
  GET_MONTH_TEST( 6,  92, 121);
  GET_MONTH_TEST( 7, 122, 152);
  GET_MONTH_TEST( 8, 153, 183);
  GET_MONTH_TEST( 9, 184, 213);
  GET_MONTH_TEST(10, 214, 244);
  GET_MONTH_TEST(11, 245, 274);
  GET_MONTH_TEST(12, 275, 305);
  GET_MONTH_TEST(13, 306, 336);
  GET_MONTH_TEST(14, 337, 365);
  #undef GET_MONTH_TEST
}

//--------------------------------------------------------------------------------------------------
// Dynamic tests
//--------------------------------------------------------------------------------------------------

void
is_multiple_of_100_test() {

  std::cout << "test_is_multiple_of_100... ";
  auto failed = false;

  for (std::int32_t n = -536870800; n <= 536870999; ++n)
    if (failed = ((n % 100 == 0) != is_multiple_of_100(n)))
      std::cout << "failed for n = " << n << '\n';

  if (!failed)
    std::cout << "OK\n";
}

template <typename A>
void
round_trip_test() {

  std::cout << "round_trip_test... ";
  auto failed = false;

  // Compile-time checks.

  static_assert(A::round_rata_die_min == A::to_rata_die(A::round_date_min));
  static_assert(A::round_rata_die_max == A::to_rata_die(A::round_date_max));

  static_assert(A::round_date_min == A::to_date(A::round_rata_die_min));
  static_assert(A::round_date_max == A::to_date(A::round_rata_die_max));

  // Runtime checks.

  for (auto n = A::round_rata_die_min; n <= A::round_rata_die_max; ++n)
    if (failed = (n != A::to_rata_die(A::to_date(n)))) {
      std::cout << "failed for n = " << n << '\n';
      break;
    }

  if (!failed)
    std::cout << "OK\n";
}

template <typename A>
void
to_date_test() {

  std::cout << "to_date_test... ";
  auto failed = false;

  using date_t     = A::date_t;
  using rata_die_t = A::rata_die_t;

  auto constexpr first = A::to_date(A::rata_die_min);
   static_assert(A::rata_die_min == min<rata_die_t> || first == min<date_t> ||
     A::to_date(A::rata_die_min - 1) != previous(first));

  auto constexpr last = A::to_date(A::rata_die_max);
  static_assert(A::rata_die_max == max<rata_die_t> || last == max<date_t> ||
    A::to_date(A::rata_die_max + 1) != next(A::to_date(A::rata_die_max)));

  auto date = A::to_date(A::rata_die_min);

  for (auto rata_die = A::rata_die_min; rata_die < A::rata_die_max; ) {
    auto const tomorrow = A::to_date(++rata_die);
    if (failed = (tomorrow != advance(date))) {
      std::cout << "failed for rata_die = " << rata_die << '\n';
      break;
    }
  }

  if (!failed)
    std::cout << "OK\n";
}

template <auto>
struct foo;

template <typename A>
void
to_rata_die_test() {

  std::cout << "to_rata_die_test... ";
  auto failed = false;

  using date_t     = A::date_t;
  using rata_die_t = A::rata_die_t;

  auto constexpr first = A::to_rata_die(A::date_min);
  static_assert(A::date_min == min<date_t> || first == min<rata_die_t> ||
     A::to_rata_die(previous(A::date_min)) != first - 1);

  auto constexpr last = A::to_rata_die(A::date_max);
  static_assert(A::date_max == max<date_t> || last == max<rata_die_t> ||
     A::to_rata_die(next(A::date_max)) != last + 1);

  auto rata_die = A::to_rata_die(A::date_min);

  for (auto date = A::date_min; date < A::date_max; ) {
    auto const tomorrow = A::to_rata_die(advance(date));
    if (failed = (tomorrow != ++rata_die)) {
      std::cout << "failed for date = " << date << '\n';
      break;
    }
  }

  if (!failed)
    std::cout << "OK\n";
}

int
main() {

  std::cout << "--------------------------\n";
  std::cout << "Preliminary tests:        \n";
  std::cout << "--------------------------\n";

  is_multiple_of_100_test();
  get_month_test();

  std::cout << '\n';

  std::cout << "--------------------------\n";
  std::cout << "Unsigned algorithms tests:\n";
  std::cout << "--------------------------\n";

  using ualgos = udate_algos<std::make_unsigned_t<year_t>, std::make_unsigned_t<rata_die_t>>;

  print<ualgos>();
  round_trip_test<ualgos>();
  to_date_test<ualgos>();
  to_rata_die_test<ualgos>();

  std::cout << '\n';

  std::cout << "--------------------------\n";
  std::cout << "Signed algorithms:        \n";
  std::cout << "--------------------------\n";

  using salgos = sdate_algos<year_t, rata_die_t>;

   print<salgos>();
   round_trip_test<salgos>();
   to_date_test<salgos>();
   to_rata_die_test<salgos>();
}
