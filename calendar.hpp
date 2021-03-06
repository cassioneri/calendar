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
 * @file calendar.hpp
 *
 * @brief Calendar algorithms.
 */

#include <algorithm>
#include <cstdint>
#include <limits>
#include <ostream>
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

/**
 * @brief Date comparison (operator ==).
 *
 * @tparam  Y         Year storage type.
 * @param   u         LHS date to be compared.
 * @param   v         RHS date to be compared.
 */
template <typename Y>
bool constexpr
operator ==(date_t<Y> const& u, date_t<Y> const& v) noexcept {
  return u.year == v.year && u.month == v.month && u.day == v.day;
}

/**
 * @brief Date comparison (operator !=).
 *
 * @tparam  Y         Year storage type.
 * @param   u         LHS date to be compared.
 * @param   v         RHS date to be compared.
 */
template <typename Y>
bool constexpr
operator !=(date_t<Y> const& u, date_t<Y> const& v) noexcept {
  return !(u == v);
}

/**
 * @brief Lexicographical order for dates (operator <).
 *
 * @tparam  Y         Year storage type.
 * @param   u         LHS date to be compared.
 * @param   v         RHS date to be compared.
 */
template <typename Y>
bool constexpr
operator <(date_t<Y> const& u, date_t<Y> const& v) noexcept {
  if (u.year  < v.year ) return true;
  if (u.year  > v.year ) return false;
  if (u.month < v.month) return true;
  if (u.month > v.month) return false;
  return u.day < v.day;
}

/**
 * @brief Lexicographical order for dates (operator <=).
 *
 * @tparam  Y         Year storage type.
 * @param   u         LHS date to be compared.
 * @param   v         RHS date to be compared.
 */
template <typename Y>
bool constexpr
operator <=(date_t<Y> const& u, date_t<Y> const& v) noexcept {
  return !(v < u);
}

/**
 * @brief Stream operator for dates (operator <<).
 *
 * @tparam  Y         Year storage type.
 * @param   u         The date to be streamed out.
 */
template <typename Y>
std::ostream&
operator <<(std::ostream& os, date_t<Y> const& u) {
  return os << u.year << '-' << std::uint32_t(u.month) << '-' << std::uint32_t(u.day);
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
 * @brief   Checks whether a given number is multiple of 100 or not using the mcomp algorithm [1].
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
 * @brief   Checks if a given number is multiple of 100 or not using built-in operator %.
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
 * @brief   Checks whether a given year is leap or not.
 *
 * @tparam  Y         Type of the given year.
 * @param   y         The given year.
 *
 * @pre               -536870800 <= y && y <= 536870999
 */
template <typename T>
bool constexpr
is_leap_year(T y) noexcept {
  // Originally, the ternary expression was similar to
  //   is_multiple_of_100(y) ? y % 16 == 0 : y % 4 == 0;
  // and Ulrich Drepper suggested the following twist.
  return (y & (is_multiple_of_100(y) ? 15 : 3)) == 0;
}

/**
 * @brief   Returns the last day of the month for a given year and month.
 *
 * @tparam  Y         Type of the given year.
 * @param   y         The given year.
 * @param   m         The given month.
 */
template <typename Y>
month_t constexpr
last_day_of_month(Y y, month_t m) noexcept {
  // Originally the 2nd operand of the 1st ternary operator was
  //   (month ^ (month >> 3)) & 1 | 30
  // and Dr. Matthias Kretz realised '& 1' was unnecessary.
  return m != 2 ? ((m ^ (m >> 3))) | 30 : is_leap_year(y) ? 29 : 28;
}

/**
 * @brief   Gregorian calendar on unsigned integer types.
 *
 * @tparam  Y         Year storage type.
 * @tparam  R         Ratadie storage type
 * @pre               std::is_unsigned_v<Y> && std::is_unsigned_v<R> &&  sizeof(R) >= sizeof(Y) &&
 *                    std::numeric_limits<R>::max() >= 146097
 */
template <typename Y = std::uint32_t, typename R = Y>
struct ugregorian_t {

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

  /**
   * @brief Returns the rata die corresponding to a given date.
   *
   * @param u1        The given date.
   * @pre             date_min <= u && u <= date_max
   */
  rata_die_t static constexpr
  to_rata_die(date_t const& u1) noexcept {

    auto const y1 = rata_die_t(u1.year);
    auto const m1 = rata_die_t(u1.month);
    auto const d1 = rata_die_t(u1.day);

    auto const j  = rata_die_t(m1 < 3);
    auto const y0 = y1 - j;
    auto const m0 = j ? m1 + 12 : m1;
    auto const d0 = d1 - 1;

    auto const q1 = y0 / 100;
    auto const yc = 1461 * y0 / 4 - q1 + q1 / 4;
    auto const mc = (979 * m0 - 2919) / 32;
    auto const dc = d0;

    auto const r1 = yc + mc + dc;

    return r1;
  }

  /**
   * @brief Returns the date corresponding to a given rata die.
   *
   * @param r0        The given rata_die.
   * @pre             rata_die_min <= r0 && r0 <= rata_die_max
   */
  date_t static constexpr
  to_date(rata_die_t r0) noexcept {

    auto const     n1  = 4 * r0 + 3;
    auto const     q1  = n1 / 146097;
    auto const     r1  = n1 % 146097 / 4;

    auto constexpr p32 = std::uint64_t(1) << 32;
    auto const     n2  = 4 * r1 + 3;
    auto const     u2  = std::uint64_t(2939745) * n2;
    auto const     q2  = std::uint32_t(u2 / p32);
    auto const     r2  = std::uint32_t(u2 % p32) / 2939745 / 4;

    auto constexpr p16 = std::uint32_t(1) << 16;
    auto const     n3  = 2141 * r2 + 197913;
    auto const     q3  = n3 / p16;
    auto const     r3  = n3 % p16 / 2141;

    auto const     y0  = 100 * q1 + q2;
    auto const     m0  = q3;
    auto const     d0  = r3;

    auto const     j   = r2 >= 306;
    auto const     y1  = y0 + j;
    auto const     m1  = j ? m0 - 12 : m0;
    auto const     d1  = d0 + 1;

    return { year_t(y1), month_t(m1), day_t(d1) };
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
    if (max<year_t> <= y)
      return max<date_t>;

    return date_t{year_t(y + 1), month_t(2), day_t(28 + is_leap_year(y + 1))};
  }();

  /**
   * @brief Minimum rata die allowed as input to to_date.
   */
  rata_die_t static constexpr rata_die_min = 0;

  /**
   * @brief Maximum rata die allowed as input to to_date.
   */
  rata_die_t static constexpr rata_die_max = []{
    // Promoted algorithms are used to calculate rata_die_max and date_max. By promoting the year
    // storage type to rata_die_t, they mitigate the risk of having intermediate year results that
    // are not representable by year_t. This allows comparisons against max<year_t> to be safely
    // performed on rata_die_max objects.
    using pugregorian_t = ugregorian_t<rata_die_t, rata_die_t>;
    using pyear_t       = typename pugregorian_t::year_t;
    using pdate_t       = typename pugregorian_t::date_t;
    auto constexpr n = (max<rata_die_t> - 3) / 4;
    auto constexpr u = pugregorian_t::to_date(n);
    auto constexpr v = pdate_t{ pyear_t(max<date_t>.year), max<date_t>.month, max<date_t>.day};
    if (u <= v)
      return n;
    return pugregorian_t::to_rata_die(v);
  }();

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

}; // struct ugregorian_t

/**
 * @brief   The Unix epoch, i.e., 1970-Jan-01.
 *
 * @tparam  Y         Type of year data member.
 */
template <typename Y = std::int32_t>
auto constexpr unix_epoch = date_t<Y>{1970, 1, 1};

/**
 * @brief   Gregorian calendar on signed integer types.
 *
 * This class is more configurable than ugregorian_t allowing negative years and rata dies. It also
 * allows different epochs (by default, unix_epoch). This is a thin but not free layer class around
 * ugregorian_t. Indeed, each function in gregorian_t simply adapts inputs and outputs (generally
 * through one addition and one subtraction) before/after delegating to a corresponding function in
 * ugregorian_t.
 *
 * @tparam  Y         Year storage type.
 * @tparam  R         Rata die storage type.
 * @tparam  e         Date used as epoch.
 * @pre               std::is_signed_v<Y> && std::is_signed_v<R>
 */
template <typename Y, typename R = Y, date_t<Y> e = unix_epoch<Y>>
struct gregorian_t {

  static_assert(std::is_signed_v<Y> && std::is_signed_v<R>);

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
  date_t static constexpr epoch = e;

private:

  // Preconditions related to representability of years by its storage type should be addressed in
  // this class and not in the ugregorian_t helper. Therefore, to disable such constraints in the
  // helper class, the storage type for years is set to be the same as for rata dies.
  using uyear_t      = std::make_unsigned_t<rata_die_t>;
  using urata_die_t  = std::make_unsigned_t<rata_die_t>;
  using ugregorian_t = ::ugregorian_t<uyear_t, urata_die_t>;
  using udate_t      = typename ugregorian_t::date_t;

public:

  struct offset_t {
    uyear_t     year;
    urata_die_t rata_die;
  };

  offset_t static constexpr offset = []{
    auto constexpr q = epoch.year / 400;
    auto constexpr r = epoch.year % 400;
    // Recall that % returns the remainder of truncated division and thus, r is in [-399, 399].
    // Hence, (r, epoch.month, epoch.day) might be outside the domain of ugregorian_t::to_rata_die.
    // To mitigate this issue we take y = r + 400 which is in [1, 799]. Therefore, (y, epoch.month,
    // epoch.day) is in the domain of ugregorian_t::to_rata_die.
    auto constexpr u = udate_t{r + 400, epoch.month, epoch.day};
    // Since adding 400 years to a date increases its rata die by 146097, we subtract this value to
    // get the correct result. This number might be "negative" but this is not a problem because
    // following calculations are under modular arithmetics.
    auto constexpr n     = ugregorian_t::to_rata_die(u) - 146097;
    auto constexpr t     = ugregorian_t::rata_die_max / 146097 / 2;
    auto constexpr z2    = 400 * (q - t);
    auto constexpr n2_e3 = 146097 * t + n;

    return offset_t{z2, n2_e3};
  }();

private:

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
   * @param u         The given date.
   */
  udate_t static constexpr
  to_udate(date_t const& u) noexcept {
    return { u.year - offset.year, u.month, u.day };
  }

  /**
   * @brief Adjusts date from unsigned to signed.
   *
   * @param u         The given date.
   */
  date_t static constexpr
  from_udate(udate_t const& u) noexcept {
    return { year_t(u.year + offset.year), u.month, u.day };
  }

public:

  /**
   * @brief Returns the rata die corresponding to a given date.
   *
   * @param u2        The given date.
   * @pre             date_min <= u && u <= date_max
   */
  rata_die_t static constexpr
  to_rata_die(date_t const& u2) noexcept {
    return from_urata_die(ugregorian_t::to_rata_die(to_udate(u2)));
  }

  /**
   * @brief Returns the date corresponding to a given rata die.
   *
   * @param n3        The given rata die.
   * @pre             rata_die_min <= n && n <= rata_die_max
   */
  date_t static constexpr
  to_date(rata_die_t n3) noexcept {
    return from_udate(ugregorian_t::to_date(to_urata_die(n3)));
  }

 /**
  * @brief  Minimum date allowed as input to to_rata_die.
  */
  date_t static constexpr date_min = []{
    // Morally, the condition below should be
    //   if (to_udate(min<date_t>) < ugregorian_t::date_min)
    // However, due to the modular arithmetics of unsigned types, this would happen when
    // to_udate(min<date_t>) overflows, becoming too large and going outside the domain of
    // ugregorian_t::to_rata_die.
    if (ugregorian_t::date_max < to_udate(min<date_t>))
      return from_udate(ugregorian_t::date_min);
    return min<date_t>;
  }();

 /**
  * @brief  Maximum date allowed as input to to_rata_die.
  */
  date_t static constexpr date_max = []{
    auto constexpr x = to_udate(max<date_t>);
    if (ugregorian_t::date_max < x)
      return from_udate(ugregorian_t::date_max);
    return max<date_t>;
  }();

  /**
   * @brief Minimum rata die allowed as input to to_date.
   */
  rata_die_t static constexpr rata_die_min = []{
    // Morally, the condition below should be
    //   if (to_udate(min<date_t>) < ugregorian_t::to_date(ugregorian_t::rata_die_min))
    // However, due to the modular arithmetics of unsigned types, this would happen when
    // to_udate(min<date_t>) overflows, becoming too large and going outside the image of
    // ugregorian_t::to_date.
    if (ugregorian_t::to_date(ugregorian_t::rata_die_max) < to_udate(min<date_t>))
      return from_urata_die(ugregorian_t::rata_die_min);
    return to_rata_die(min<date_t>);
  }();

  /**
   * @brief Maximum rata die allowed as input to to_date.
   */
  rata_die_t static constexpr rata_die_max = []{
    if (ugregorian_t::to_date(ugregorian_t::rata_die_max) < to_udate(max<date_t>))
      return from_urata_die(ugregorian_t::rata_die_max);
    return to_rata_die(max<date_t>);
  }();

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

}; // struct gregorian_t
