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

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>

/**
 * @brief   Month storage type.
 */
using month_t = std::uint8_t;

/**
 * @brief   Day storage type.
 */
using day_t   = std::uint8_t;

/**
 * @brief   Date storage type.
 *
 * @tparam  Int       Year storage type.
 */
template <typename Int>
struct date_t {
  Int     year;
  month_t month;
  day_t   day;
};

/**
 * @brief   Checks if a given number is multiple of 100 or not using the mcomp algorithm [1].
 *
 * This a special implementation, supposedly faster than built-in operator %, for a subrange of
 * std::int32_t values containing [-32767, 32767].
 *
 * [1] https://accu.org/var/uploads/journals/Overload155.pdf#page=16
 *
 * @param   n         The given number.
 * @pre               -536870800 <= n && n <= 536870999
 */
bool constexpr
is_multiple_of_100(std::int32_t n) noexcept {
  // From qmodular: ./div mcomp 100
  std::uint32_t constexpr multiplier   = 42949673;
  std::uint32_t constexpr bound        = 42949669;
  std::uint32_t constexpr max_dividend = 1073741799;

  std::uint32_t constexpr offset       = max_dividend / 2 / 100 * 100; //  536870800
//std::int32_t  constexpr min          = -offset;                      // -536870800
//std::int32_t  constexpr max          = max_dividend - offset;        //  536870999

  return multiplier * (n + offset) < bound;
}

/**
 * @brief   Checks if a given number is multiple of 100 or not uisng built-in operator %.
 *
 * @tparam  Int       The type of the given number.
 * @param   n         The given number.
 */
template <typename Int>
bool constexpr
is_multiple_of_100(Int n) noexcept {
  return n % 100 == 0;
}

/**
 * @brief   Checks if a given year is leap or not.
 *
 * @tparam  Int       Type of the given year.
 * @param   year      The given year.
 *
 * @pre               -536870800 <= year && year <= 536870999
 */
template <typename Int>
bool constexpr
is_leap_year(Int year) noexcept {
  // http://quick-bench.com/BRo2jU8FDDt1jKqAhTwRasFPoXI
  // http://stackoverflow.com/a/60646967/1137388
  return (!is_multiple_of_100(year) || year % 400 == 0) & (year % 4 == 0);
}

/**
 * @brief   Returns the last day of the month for a given year and month.
 *
 * @tparam  Int       Type of the given year.
 * @param   year      The given year.
 * @param   month     The given month.
 */
template <typename Int>
std::uint8_t constexpr
last_day_of_month(Int year, month_t month) noexcept {
  // One MUST see benchmark results below and comments therein.
  // http://quick-bench.com/40yoPY7ZJG6VQKBNv6fJtYA9-E8
  std::uint32_t constexpr b = 0b1010110101010;
  return month != 2 ? 30 + ((b >> month) & 1) : (is_leap_year(year) ? 29 : 28);
}

/**
 * @brief   Unsigned algorithms.
 *
 * @tparam  UInt      Rata die and year storage type.
 * @pre               std::is_unsigned_v<UInt> && std::numeric_limits<UInt>::digits >= 18
 */
template <typename UInt = std::uint32_t>
struct udate_algos {

  static_assert(std::is_unsigned_v<UInt>);
  static_assert(std::numeric_limits<UInt>::digits >= 18); // 146097 is 18-bits long

  /**
   * @brief Rata die and year storage type.
   */
  using rata_die_t = UInt;

  /**
   * @brief Date storage type.
   */
  using date_t = ::date_t<rata_die_t>;

  /**
   * @brief Date used as epoch.
   */
  static date_t constexpr epoch = date_t{0, 3, 1};

  /**
   * @brief Minimum rata die allowed as input to to_date.
   */
  static rata_die_t constexpr rata_die_min = 0;

  /**
   * @brief Maximum rata die allowed as input to to_date.
   */
  static rata_die_t constexpr rata_die_max = (std::numeric_limits<rata_die_t>::max() - 3) / 4;

  /**
   * @brief Returns the date corresponding to a given rata die.
   *
   * @param rata_die  The given rata_die.
   * @pre             rata_die_min <= rata_die && rata_die <= rata_die_max
   */
  static constexpr
  date_t to_date(rata_die_t rata_die) noexcept {

    // http://quick-bench.com/KpuxxwA5mRr9t8ahuKAwHlpUD3A

    auto const century         = (4 * rata_die + 3) / 146097;
    auto const day_of_century  = rata_die - 146097 * century / 4;
    auto const year_of_century = (4 * day_of_century + 3) / 1461;
    auto const day_of_year     = day_of_century - 1461 * year_of_century / 4;
    auto const year_modified   = 100 * century + year_of_century;
    auto const month_modified  = (535 * day_of_year + 331) / 16384;
    auto const day_modified    = day_of_year - (979 * month_modified + 15) / 32;
    auto const jan_or_feb      = day_of_year > 305;
    auto const year            = year_modified + jan_or_feb;
    auto const month           = jan_or_feb ? month_modified - 9 : month_modified + 3;
    auto const day             = day_modified + 1;
    return { year, std::uint8_t(month), std::uint8_t(day) };
  }

 /**
  * @brief  Minimum date allowed as input to to_rata_die.
  */
  static date_t constexpr date_min = epoch;

 /**
  * @brief  Maximum date allowed as input to to_rata_die.
  */
  static date_t constexpr date_max = {std::numeric_limits<rata_die_t>::max() / 1461 + 1, 2, 28};

  /**
   * @brief Returns the rata die corresponding to a given date.
   *
   * @param date      The given date.
   * @pre             date_min <= date && date <= date_max
   */
  static constexpr
  rata_die_t to_rata_die(date_t const& date) noexcept {

    // http://quick-bench.com/ayO1N4nsDY6EL1I_8qa7b3nUhi0

    auto const jan_or_feb     = date.month < 3;
    auto const day_modified   = date.day - 1;
    auto const month_modified = jan_or_feb ? date.month + 9 : date.month - 3;
    auto const year_modified  = date.year - jan_or_feb;
    auto const century        = year_modified / 100;
    auto const year_count     = 1461 * year_modified / 4 - century + century / 4;
    auto const month_count    = (979 * month_modified + 15) / 32;
    auto const day_count      = day_modified;
    auto const rata_die       = year_count + month_count + day_count;
    return rata_die;
  }

  /**
   * @brief Minimum rata die allowed as input to to_date for round trip.
   */
  static rata_die_t constexpr round_rata_die_min = std::max(rata_die_min, to_rata_die(date_min));

  /**
   * @brief Maximum rata die allowed as input to to_date for round trip.
   */
  static rata_die_t constexpr round_rata_die_max = std::min(rata_die_max, to_rata_die(date_max));

  /**
   * @brief Minimum date allowed as input to to_rata_die for round trip.
   */
  static date_t  constexpr round_date_min = to_date(round_rata_die_min);

  /**
   * @brief Maximum date allowed as input to to_rata_die for round trip.
   */
  static date_t  constexpr round_date_max = to_date(round_rata_die_max);

}; // struct udate_algos

/**
 * @brief   The Unix epoch, i.e., 1970-Jan-01.
 *
 * @tparam  Int       Type of year data member.
 */
template <typename Int = std::int32_t>
auto constexpr unix_epoch = date_t<Int>{1970, 1, 1};

/**
 * @brief   Signed algorithms.
 *
 * This class is more configurable than udate_algos allowing negative years and rata dies as well as
 * a different epoch (by default, unix_epoch). This is a thin but not free layer class around
 * udate_algos. Indeed, each function in sdate_algos simply adapts inputs and outputs (generally
 * through one addition and one subtraction) before/after delegating to a corresponding function in
 * udate_algos.
 *
 * @tparam  Int       Rata die and year storage type.
 * @tparam  epoch     Date used as epoch.
 * @pre               std::is_signed_v<Int> && epoch.year >= 0
 */
 // Todo: (CN) Acount and document pre-conditions to avoid overflow when adding/subtracting offsets.
template <typename Int, date_t<Int> epoch = unix_epoch<Int>>
struct sdate_algos {

  static_assert(std::is_signed_v<Int>);
  static_assert(epoch.year >= 0);

  /**
   * @brief Rata die and year storage type.
   */
  using rata_die_t = Int;

  /**
   * @brief Date storage type.
   */
  using date_t = ::date_t<rata_die_t>;

private:

  using ualgos      = udate_algos<std::make_unsigned_t<rata_die_t>>;
  using urata_die_t = ualgos::rata_die_t;
  using udate_t     = ualgos::date_t;

  static urata_die_t constexpr half_periods     = ualgos::rata_die_max / 146097 / 2;
  static urata_die_t constexpr periods_in_epoch = epoch.year / 400;
  static urata_die_t constexpr remaining_days   = ualgos::to_rata_die(udate_t{epoch.year % 400,
    epoch.month, epoch.day});

  static urata_die_t constexpr year_offset      = (half_periods - periods_in_epoch) * 400;
  static urata_die_t constexpr rata_die_offset  = half_periods * 146097 + remaining_days;

  /**
   * @brief Adjusts rata die (to account for epoch) to be passed to ualgos::to_date.
   */
  static constexpr
  urata_die_t to_urata_die(rata_die_t rata_die) noexcept {
    return rata_die + rata_die_offset;
  }

  /**
   * @brief Adjusts rata die (to account for epoch) returned from ualgos::to_date.
   */
  static constexpr
  rata_die_t from_urata_die(urata_die_t urata_die) noexcept {
    return urata_die - rata_die_offset;
  }

  /**
   * @brief Adjusts date (to account for epoch) to be passed to ualgos::to_rata_die.
   */
  static constexpr
  udate_t to_udate(date_t const& date) noexcept {
    return { date.year + year_offset, date.month, date.day };
  }

  /**
   * @brief Adjusts date (to account for epoch) returned from ualgos::to_date.
   */
  static constexpr
  date_t from_udate(udate_t const& date) noexcept {
    return { rata_die_t(date.year - year_offset), date.month, date.day };
  }

public:

  /**
   * @brief Minimum rata die allowed as input to to_date.
   */
  static rata_die_t constexpr rata_die_min = from_urata_die(ualgos::rata_die_min);

  /**
   * @brief Maximum rata die allowed as input to to_date.
   */
  static rata_die_t constexpr rata_die_max = from_urata_die(ualgos::rata_die_max);

  /**
   * @brief Returns the date corresponding to a given rata die.
   *
   * @param rata_die  The given rata die.
   * @pre             rata_die_min <= rata_die && rata_die <= rata_die_max
   */
  static constexpr
  date_t to_date(rata_die_t rata_die) noexcept {
    // http://quick-bench.com/KpuxxwA5mRr9t8ahuKAwHlpUD3A
    return from_udate(ualgos::to_date(to_urata_die(rata_die)));
  }

 /**
  * @brief  Minimum date allowed as input to to_rata_die.
  */
  static date_t constexpr date_min = from_udate(ualgos::date_min);

 /**
  * @brief  Maximum date allowed as input to to_rata_die.
  */
  static date_t constexpr date_max = from_udate(ualgos::date_max);

  /**
   * @brief Returns the rata die corresponding to a given date.
   *
   * @param date      The given date.
   * @pre             date_min <= date && date <= date_max
   */
  static constexpr
  rata_die_t to_rata_die(date_t const& date) noexcept {
    // http://quick-bench.com/ayO1N4nsDY6EL1I_8qa7b3nUhi0
    return from_urata_die(ualgos::to_rata_die(to_udate(date)));
  }

  /**
   * @brief Minimum rata die allowed as input to to_date for round trip.
   */
  static rata_die_t constexpr round_rata_die_min = std::max(rata_die_min, to_rata_die(date_min));

  /**
   * @brief Maximum rata die allowed as input to to_date for round trip.
   */
  static rata_die_t constexpr round_rata_die_max = std::min(rata_die_max, to_rata_die(date_max));

  /**
   * @brief Minimum date allowed as input to to_rata_die for round trip.
   */
  static date_t constexpr round_date_min = to_date(round_rata_die_min);

  /**
   * @brief Maximum date allowed as input to to_rata_die for round trip.
   */
  static date_t constexpr round_date_max = to_date(round_rata_die_max);

}; // struct sdate_algos

