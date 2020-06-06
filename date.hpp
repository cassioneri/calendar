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

///TODO(CN): Update doc.
/**
 * @brief   Calculates the quotient and remainder of the division by 1461.
 */
std::pair<std::uint32_t, std::uint32_t> constexpr
div_1461(std::uint32_t n) noexcept {
    auto constexpr a = std::uint64_t(1) << 32;
    auto constexpr b = std::uint32_t(a / 1461 + 1);
    auto const     p = std::uint64_t(b) * n;
    auto const     q = std::uint32_t(p / a);
    auto const     r = std::uint32_t(p % a) / b;
    return {q, r};
}

///TODO(CN): Update doc.
/**
 * @brief   Calculates the quotient and remainder of the division by 1461.
 */
template <typename T>
std::pair<T, T> constexpr
div_1461(T n) noexcept {
  return {n / 1461, n % 1461};
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
  // http://stackoverflow.com/a/60646967/1137388
  return (!is_multiple_of_100(y) || y % 16 == 0) & (y % 4 == 0);
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
  return m != 2 ? ((m ^ (m >> 3)) & 1) | 30 : is_leap_year(y) ? 29 : 28;
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
   * @param u         The given date.
   * @pre             date_min <= u && u <= date_max
   */
  rata_die_t static constexpr
  to_rata_die(date_t const& u) noexcept {
    auto const y1 = rata_die_t(u.year);
    auto const m1 = rata_die_t(u.month);
    auto const d1 = rata_die_t(u.day);
    auto const j  = rata_die_t(m1 < 3);
    auto const y  = y1 - j;
    auto const m  = j ? m1 + 12 : m1;
    auto const d  = d1 - 1;
    auto const q1 = y / 100;
    auto const y0 = 1461 * y / 4 - q1 + q1 / 4;
    auto const m0 = (979 * m - 2922) / 32;
    auto const n1 = y0 + m0 + d;
    return n1;
  }

  /**
   * @brief Returns the date corresponding to a given rata die.
   *
   * @param n         The given rata_die.
   * @pre             rata_die_min <= n && n <= rata_die_max
   */
  date_t static constexpr
  to_date(rata_die_t n) noexcept {
    auto const p1      = 4 * n + 3;
    auto const q1      = p1 / 146097;
    auto const r1      = p1 % 146097;
    auto const p2      = r1 | 3;
    auto const [q2, r] = div_1461(p2);
    auto const r2      = r / 4;
    auto const p3      = 2141 * r2 + 197657;
    auto const m       = p3 / 65536;
    auto const d       = p3 % 65536 / 2141;
    auto const y       = 100 * q1 + q2;
    auto const j       = r2 > 305;
    auto const y1      = y + j;
    auto const m1      = j ? m - 12 : m;
    auto const d1      = d + 1;
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
   * @param u         The given date.
   * @pre             date_min <= u && u <= date_max
   */
  rata_die_t static constexpr
  to_rata_die(date_t const& u) noexcept {
    return from_urata_die(ugregorian_t::to_rata_die(to_udate(u)));
  }

  /**
   * @brief Returns the date corresponding to a given rata die.
   *
   * @param n         The given rata die.
   * @pre             rata_die_min <= n && n <= rata_die_max
   */
  date_t static constexpr
  to_date(rata_die_t n) noexcept {
    return from_udate(ugregorian_t::to_date(to_urata_die(n)));
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
