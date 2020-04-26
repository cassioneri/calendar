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
using day_t = std::uint8_t;

/**
 * @brief   Date storage type.
 *
 * @tparam  Y         Year storage type.
 */
template <typename Y>
struct date_t {
  Y       year;
  month_t month;
  day_t   day;
};

//TODO (CN): Use operator <=>.

/**
 * @brief Date comparison (operator ==).
 *
 * @tparam  Y         Year storage type.
 */
template <typename Y>
bool constexpr
operator ==(date_t<Y> const& x, date_t<Y> const& y) noexcept {
  return x.year == y.year && x.month == y.month && x.day == y.day;
}

/**
 * @brief Date comparison (operator !=).
 *
 * @tparam  Y         Year storage type.
 */
template <typename Y>
bool constexpr
operator !=(date_t<Y> const& x, date_t<Y> const& y) noexcept {
  return !(x == y);
}

/**
 * @brief Lexicographical order for dates (operator <).
 *
 * @tparam  Y         Year storage type.
 */
template <typename Y>
bool constexpr
operator <(date_t<Y> const& x, date_t<Y> const& y) noexcept {
  if (x.year  < y.year ) return true;
  if (x.year  > y.year ) return false;
  if (x.month < y.month) return true;
  if (x.month > y.month) return false;
  return x.day < y.day;
}

/**
 * @brief Lexicographical order for dates (operator <=).
 *
 * @tparam  Y         Year storage type.
 */
template <typename Y>
bool constexpr
operator <=(date_t<Y> const& x, date_t<Y> const& y) noexcept {
  return !(y < x);
}

/**
 * @brief Stream operator for dates (operator <<).
 *
 * @tparam  Y         Year storage type.
 */
template <typename Y>
std::ostream&
operator <<(std::ostream& os, date_t<Y> const& d) {
  return os << d.year << '-' << (int) d.month << '-' << (int) d.day;
}

/**
 * @brief Maximum value of a given type.
 *
 * @tparam T          The given type.
 */
template <typename T>
auto constexpr max = std::numeric_limits<T>::max();

template <typename Y>
auto constexpr max<date_t<Y>> = date_t<Y>{max<Y>, 12, 31};

/**
 * @brief Minimum value of a given type.
 *
 * @tparam T          The given type.
 */
template <typename T>
auto constexpr min = std::numeric_limits<T>::min();

template <typename Y>
auto constexpr min<date_t<Y>> = date_t<Y>{min<Y>,  1,  1};

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
 * @tparam  T         The type of the given number.
 * @param   n         The given number.
 */
template <typename T>
bool constexpr
is_multiple_of_100(T n) noexcept {
  return n % 100 == 0;
}

/**
 * @brief   Checks if a given year is leap or not.
 *
 * @tparam  Y         Type of the given year.
 * @param   year      The given year.
 *
 * @pre               -536870800 <= year && year <= 536870999
 */
template <typename T>
bool constexpr
is_leap_year(T year) noexcept {
  // http://quick-bench.com/BRo2jU8FDDt1jKqAhTwRasFPoXI
  // http://stackoverflow.com/a/60646967/1137388
  return (!is_multiple_of_100(year) || year % 400 == 0) & (year % 4 == 0);
}

/**
 * @brief   Returns the last day of the month for a given year and month.
 *
 * @tparam  Y         Type of the given year.
 * @param   year      The given year.
 * @param   month     The given month.
 */
template <typename Y>
month_t constexpr
last_day_of_month(Y year, month_t month) noexcept {
  // One MUST see benchmark results below and comments therein.
  // http://quick-bench.com/40yoPY7ZJG6VQKBNv6fJtYA9-E8
  std::uint32_t constexpr b = 0b1010110101010;
  return month != 2 ? 30 + ((b >> month) & 1) : (is_leap_year(year) ? 29 : 28);
}

/**
 * @brief   Unsigned algorithms.
 *
 * @tparam  Y         Year storage type.
 * @tparam  R         Ratadie storage type
 * @pre               std::is_unsigned_v<Y> && std::is_unsigned_v<R> &&  sizeof(R) >= sizeof(Y) &&
 *                    std::numeric_limits<R>::max() >= 146097
 */
template <typename Y = std::uint32_t, typename R = Y>
struct udate_algos {

  static_assert(std::is_unsigned_v<Y> && std::is_unsigned_v<R> &&  sizeof(R) >= sizeof(Y) &&
    std::numeric_limits<R>::max() >= 146097);

  /**
   * @brief Year storage type.
   */
  using year_t = Y;

  /**
   * @brief Rata die storage type.
   */
  using rata_die_t = R;

  /**
   * @brief Date storage type.
   */
  using date_t = ::date_t<year_t>;

  /**
   * @brief Date used as epoch.
   */
  static date_t constexpr epoch = date_t{0, 3, 1};

private:

  /**
   * @brief Returns the number of days prior to a given year.
   *
   * @param y         The given year.
   * @pre             y <= max<year_t> / (std::is_constant_evaluated() ? 366 : 1461)
   */
  rata_die_t static constexpr
  year_count(rata_die_t y) noexcept {
    if (std::is_constant_evaluated())
      return 365 * y + y / 4 - y / 100 + y / 400;
    auto const c = y / 100;
    return 1461 * y / 4 - c + c / 4;
  }

  /**
   * @brief Returns the number of days prior to a given month.
   *
   * @param m         The given month.
   * @pre             3 <= m && m <= 14
   */
  rata_die_t static constexpr
  month_count(rata_die_t m) noexcept {
    return (979 * m - 2922) / 32;
  }

  /**
   * @brief Returns the year and the rest (sum of month count and day count) corresponding to a
   * given rata die.
   *
   * @param n         The given rata die.
   * @pre             n <= (max<rata_die_t> - 3) / 4
   */
  std::pair<rata_die_t, rata_die_t> static constexpr
  year_and_rest(rata_die_t n) noexcept {
    auto const n1 = 4 * n + 3;
    auto const c1 = n1 / 146097;
    auto const n2 = n1 % 146097 + c1 % 4;
    auto const c2 = n2 / 1461;
    auto const n3 = n2 % 1461 / 4;
    auto const z  = 100 * c1 + c2;
    return {z, n3};
  }

public:

  /**
   * @brief Minimum rata die allowed as input to to_date.
   */
  rata_die_t static constexpr rata_die_min = 0;

  /**
   * @brief Maximum rata die allowed as input to to_date.
   */
  rata_die_t static constexpr rata_die_max = []{

    auto constexpr y  = rata_die_t(max<year_t>);
    auto constexpr n1 = year_count(y);
    auto constexpr n2 = (max<rata_die_t> - 3) / 4;

    if (y <= year_and_rest(n2).first)
      return n1 + std::min(rata_die_t(305), max<rata_die_t> - n1);

    return n2;
  }();

  /**
   * @brief Returns the date corresponding to a given rata die.
   *
   * @param n         The given rata_die.
   * @pre             rata_die_min <= n && n <= rata_die_max
   */
  date_t static constexpr
  to_date(rata_die_t n) noexcept {
    // http://quick-bench.com/jXnc-jdHbqYyp4BHrKrCDpzqbpU
    auto const [y_, n3] = year_and_rest(n);
    auto const m_       = (535 * n3 + 49483) / 16384;
    auto const d_       = n3 - month_count(m_);
    auto const j        = n3 > 305;
    auto const y        = y_ + j;
    auto const m        = j ? m_ - 12 : m_;
    auto const d        = d_ + 1;
    return { year_t(y), month_t(m), day_t(d) };
  }

 /**
  * @brief  Minimum date allowed as input to to_rata_die.
  */
  date_t static constexpr date_min = epoch;

  /**
  * @brief  Maximum date allowed as input to to_rata_die.
  */
  date_t static constexpr date_max = []{

    auto constexpr y = max<rata_die_t> / 1461;
    auto constexpr r = max<rata_die_t> - year_count(y);
    auto constexpr i = is_leap_year(y + 1);

    if (y >= max<year_t>)
      return date_t{max<year_t>, 12, 31};

    if (r > 365 + i)
      return date_t{year_t(y + 1), month_t(2), day_t(28 + i)};

    auto constexpr x = to_date(r);
    return date_t{year_t(y + (r > 305)), x.month, x.day};
  }();

  /**
   * @brief Returns the rata die corresponding to a given date.
   *
   * @param x         The given date.
   * @pre             date_min <= x && x <= date_max
   */
  rata_die_t static constexpr
  to_rata_die(date_t const& x) noexcept {
    // http://quick-bench.com/fNRyutHsrVJClleqLcw5Szg4z6g
    auto const y  = rata_die_t(x.year);
    auto const m  = rata_die_t(x.month);
    auto const d  = rata_die_t(x.day);
    auto const j  = rata_die_t(m < 3);
    auto const d_ = d - 1;
    auto const m_ = j ? m + 12 : m;
    auto const y_ = y - j;
    return year_count(y_) + month_count(m_) + d_;
  }

  /**
   * @brief Minimum rata die allowed as input to to_date for round trip.
   */
  rata_die_t static constexpr round_rata_die_min = std::max(rata_die_min, to_rata_die(date_min));

  /**
   * @brief Maximum rata die allowed as input to to_date for round trip.
   */
  rata_die_t static constexpr round_rata_die_max = std::min(rata_die_max, to_rata_die(date_max));

  /**
   * @brief Minimum date allowed as input to to_rata_die for round trip.
   */
  date_t static constexpr round_date_min = to_date(round_rata_die_min);

  /**
   * @brief Maximum date allowed as input to to_rata_die for round trip.
   */
  date_t static constexpr round_date_max = to_date(round_rata_die_max);

}; // struct udate_algos

/**
 * @brief   The Unix epoch, i.e., 1970-Jan-01.
 *
 * @tparam  Y         Type of year data member.
 */
template <typename Y = std::int32_t>
auto constexpr unix_epoch = date_t<Y>{1970, 1, 1};

/**
 * @brief   Signed algorithms.
 *
 * This class is more configurable than udate_algos allowing negative years and rata dies as well as
 * a different epoch (by default, unix_epoch). This is a thin but not free layer class around
 * udate_algos. Indeed, each function in sdate_algos simply adapts inputs and outputs (generally
 * through one addition and one subtraction) before/after delegating to a corresponding function in
 * udate_algos.
 *
 * @tparam  Y         Year storage type.
 * @tparam  R         Rata die storage type.
 * @tparam  epoch     Date used as epoch.
 * @pre               std::is_signed_v<Y> && std::is_signed_v<R> && epoch.year >= 0
 */
template <typename Y, typename R = Y, date_t<Y> epoch_ = unix_epoch<Y>>
struct sdate_algos {

  static_assert(std::is_signed_v<Y> && std::is_signed_v<R> && epoch_.year >= 0);

  /**
   * @brief Year storage type.
   */
  using year_t = Y;

  /**
   * @brief Rata die storage type.
   */
  using rata_die_t = R;

  /**
   * @brief Date storage type.
   */
  using date_t = ::date_t<year_t>;

  /**
   * @brief Date used as epoch.
   */
  static date_t constexpr epoch = epoch_;

private:

  // Preconditions related to representability of years by its storage type should be addressed in
  // this class and not in the unsigned algorithm helper. Therefore, to disable such contraints in
  // the helper class it uses the storage type for years is set to be the same as for rata dies.
  using uyear_t     = std::make_unsigned_t<rata_die_t>;
  using urata_die_t = std::make_unsigned_t<rata_die_t>;
  using ualgos      = udate_algos<uyear_t, urata_die_t>;
  using udate_t     = ualgos::date_t;

  struct offset_t {
    uyear_t     year;
    urata_die_t rata_die;
  };

  offset_t static constexpr offset = []{
    auto constexpr era     = uyear_t(epoch.year) / 400;
    auto constexpr yoe     = uyear_t(epoch.year) % 400;
    auto constexpr r       = ualgos::to_rata_die({yoe, epoch.month, epoch.day});
    auto constexpr mid_era = (ualgos::rata_die_max / 2 - r) / 146097;
    return offset_t{ 400 * (mid_era - uyear_t(epoch.year) / 400), 146097 * mid_era + r};
  }();

  /**
   * @brief Adjusts rata die from signed to unsigned.
   *
   * @param n         The given rata die.
   */
  urata_die_t static constexpr
  to_urata_die(rata_die_t n) noexcept {
    return n + offset.rata_die;
  }

  /**
   * @brief Adjusts rata die from unsigned to signed.
   *
   * @param n         The given rata die.
   */
  rata_die_t static constexpr
  from_urata_die(urata_die_t n) noexcept {
    return n - offset.rata_die;
  }

  /**
   * @brief Adjusts date from signed to unsigned.
   *
   * @param x         The given date.
   */
  udate_t static constexpr
  to_udate(date_t const& x) noexcept {
    return { x.year + offset.year, x.month, x.day };
  }

  /**
   * @brief Adjusts date from unsigned to signed.
   *
   * @param x         The given date.
   */
  date_t static constexpr
  from_udate(udate_t const& x) noexcept {
    return { year_t(x.year - offset.year), x.month, x.day };
  }

public:

 /**
  * @brief  Minimum date allowed as input to to_rata_die.
  */
  date_t static constexpr date_min = []{
    // Morally, the condition below should be
    //   if (to_udate(min<date_t>) < ualgos::date_min)
    // However, due to the modular arithmetics of unsigned types, this would happen when
    // to_udate(min<date_t>) overflows, becoming too large and going outside the domain of
    // ualgos::to_rata_die.
    if (ualgos::date_max < to_udate(min<date_t>))
      return from_udate(ualgos::date_min);
    return min<date_t>;
  }();

 /**
  * @brief  Maximum date allowed as input to to_rata_die.
  */
  date_t static constexpr date_max = []{
    auto constexpr x = to_udate(max<date_t>);
    if (ualgos::date_max < x)
      return from_udate(ualgos::date_max);
    return max<date_t>;
  }();

  /**
   * @brief Returns the rata die corresponding to a given date.
   *
   * @param x         The given date.
   * @pre             date_min <= x && x <= date_max
   */
  rata_die_t static constexpr
  to_rata_die(date_t const& x) noexcept {
    // http://quick-bench.com/fNRyutHsrVJClleqLcw5Szg4z6g
    return from_urata_die(ualgos::to_rata_die(to_udate(x)));
  }

  /**
   * @brief Minimum rata die allowed as input to to_date.
   */
  rata_die_t static constexpr rata_die_min = []{
    // Morally, the condition below should be
    //   if (to_udate(min<date_t>) < ualgos::to_date(ualgos::rata_die_min))
    // However, due to the modular arithmetics of unsigned types, this would happen when
    // to_udate(min<date_t>) overflows, becoming too large and going outside the image of
    // ualgos::to_date.
    if (ualgos::to_date(ualgos::rata_die_max) < to_udate(min<date_t>))
      return from_urata_die(ualgos::rata_die_min);
    return to_rata_die(min<date_t>);
  }();

  /**
   * @brief Maximum rata die allowed as input to to_date.
   */
  rata_die_t static constexpr rata_die_max = []{
    if (ualgos::to_date(ualgos::rata_die_max) < to_udate(max<date_t>))
      return from_urata_die(ualgos::rata_die_max);
    return to_rata_die(max<date_t>);
  }();

  /**
   * @brief Returns the date corresponding to a given rata die.
   *
   * @param n         The given rata die.
   * @pre             rata_die_min <= n && n <= rata_die_max
   */
  date_t static constexpr
  to_date(rata_die_t n) noexcept {
    // http://quick-bench.com/jXnc-jdHbqYyp4BHrKrCDpzqbpU
    return from_udate(ualgos::to_date(to_urata_die(n)));
  }

  /**
   * @brief Minimum rata die allowed as input to to_date for round trip.
   */
  rata_die_t static constexpr round_rata_die_min = std::max(rata_die_min, to_rata_die(date_min));

  /**
   * @brief Maximum rata die allowed as input to to_date for round trip.
   */
  rata_die_t static constexpr round_rata_die_max = std::min(rata_die_max, to_rata_die(date_max));

  /**
   * @brief Minimum date allowed as input to to_rata_die for round trip.
   */
  date_t static constexpr round_date_min = to_date(round_rata_die_min);

  /**
   * @brief Maximum date allowed as input to to_rata_die for round trip.
   */
  date_t static constexpr round_date_max = to_date(round_rata_die_max);

}; // struct sdate_algos
