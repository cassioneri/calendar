#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>

/**
 * A simple year-month-day data holder.
 *
 * @tparam T          Type of year data member.
 */
template <typename T>
struct date_t {
  T            year;
  std::uint8_t month;
  std::uint8_t day;
};

/**
 * Checks if a given number is multiple of 100 or not through the mcomp algorithm [1].
 *
 * [1] https://accu.org/var/uploads/journals/Overload155.pdf#page=16
 *
 * @param             The given number.
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
 * Checks if a given number is multiple of 100 or not.
 *
 * @param             The given number.
 */
template <typename T>
bool constexpr
is_multiple_of_100(T n) noexcept {
  return n % 100 == 0;
}

/**
 * Checks if a given year is leap year or not.
 *
 * @tparam T          Type of year argument.
 * @param  year       Given year.
 *
 * @pre               -536870800 <= year && year <= 536870999
 */
template <typename Y>
bool constexpr
is_leap_year(Y year) noexcept {
  // http://quick-bench.com/BRo2jU8FDDt1jKqAhTwRasFPoXI
  // http://stackoverflow.com/a/60646967/1137388
  return (!is_multiple_of_100(year) || year % 400 == 0) & (year % 4 == 0);
}

/**
 * Returns the last day of the month for a given year and month.
 *
 * @tparam T          Type of year argument.
 * @param  year       Given year.
 * @param  month      Given month.
 */
template <typename Y>
std::uint8_t constexpr
last_day_of_month(Y year, std::uint8_t month) noexcept {
  // One MUST see benchmark results below and comments therein.
  // http://quick-bench.com/40yoPY7ZJG6VQKBNv6fJtYA9-E8
  std::uint32_t constexpr b = 0b1010110101010;
  return month != 2 ? 30 + ((b >> month) & 1) : (is_leap_year(year) ? 29 : 28);
}

/**
 * Unsigned to_days/to_date algorithms and information about these algoritm.
 *
 * Type members:
 *
 *    days_type       Type used to count days and store years. (Provided as a template argument.)
 *    date_type       Type used to store dates. (An instantiation of date_t.)
 *
 * Static data members:
 *
 *    epoch           Date used as epoch. (Set to 0000-Mar-01.)
 *
 *    days_min        Minimum count of days allowed as input to to_date. (Set to 0.)
 *    days_max        Maximum count of days allowed as input to to_date.
 *
 *    date_min        Minimum date allowed as input to to_days. (Set to epoch.)
 *    date_max        Maximum date allowed as input to to_days.
 *
 *    round_days_min  Minimum count of days allowed as input to to_date for round trip.
 *    round_days_max  Maximum count of days allowed as input to to_date for round trip.
 *
 *    round_date_min  Minimum date allowed as input to to_days for round trip.
 *    round_date_max  Maximum date allowed as input to to_days for round trip.
 *
 * @tparam UInt       Type used to count days and store years.
 * @pre               std::is_unsigned_v<UInt> && std::numeric_limits<UInt>::digits >= 18
 */
template <typename UInt = std::uint32_t>
struct udate_algos {

  static_assert(std::is_unsigned_v<UInt>);
  static_assert(std::numeric_limits<UInt>::digits >= 18); // 146097 is 18-bits long

  using days_type   = UInt;
  using date_type   = date_t<UInt>;

  static date_type constexpr epoch = date_type{0, 3, 1};

  static days_type constexpr days_min = 0;
  static days_type constexpr days_max = (std::numeric_limits<days_type>::max() - 3) / 4;

  /**
   * Returns the date corresponding to a given number of days since epoch.
   *
   * @param days      The given number of days.
   * @pre             days_min <= days && days <= days_max
   */
  static constexpr
  date_type to_date(days_type days) noexcept {

    // http://quick-bench.com/lHE2aolOkILburgCIpL-iQLbiG4

    auto const century         = (4 * days + 3) / 146097;
    auto const day_of_century  = days - 146097 * century / 4;
    auto const year_of_century = (4 * day_of_century + 3) / 1461;
    auto const day_of_year     = day_of_century - 1461 * year_of_century / 4;
    auto const year_aux        = 100 * century + year_of_century;
    auto const month_aux       = (535 * day_of_year + 331) / 16384;
    auto const day_aux         = day_of_year - (979 * month_aux + 15) / 32;
    auto const jan_or_feb      = day_of_year > 305;
    auto const year            = year_aux + jan_or_feb;
    auto const month           = jan_or_feb ? month_aux - 9 : month_aux + 3;
    auto const day             = day_aux + 1;
    return { year, std::uint8_t(month), std::uint8_t(day) };
  }

  static date_type constexpr date_min = epoch;
  static date_type constexpr date_max = {std::numeric_limits<days_type>::max() / 1461 + 1, 2, 28};

  /**
   * Returns the number of days from epoch to a given date.
   *
   * @param date      The given date.
   * @pre             date_min <= date && date <= date_max
   */
  static constexpr
  days_type to_days(date_type const& date) noexcept {

    // http://quick-bench.com/lKEBKGAqd3Ln7yJOBc3Cr8VL7DY

    auto const jan_or_feb = date.month < 3;
    auto const month_aux  = jan_or_feb ? date.month + 9 : date.month - 3;
    auto const year_aux   = date.year - jan_or_feb;
    auto const century    = year_aux / 100;
    auto const days       = 1461 * year_aux / 4 - century + century / 4 +
      (979 * month_aux + 15) / 32 + date.day - 1;
    return days;
  }

  static days_type constexpr round_days_min = std::max(days_min, to_days(date_min));
  static days_type constexpr round_days_max = std::min(days_max, to_days(date_max));

  static date_type constexpr round_date_min = to_date(round_days_min);
  static date_type constexpr round_date_max = to_date(round_days_max);

}; // struct udate_algos

/**
 * The Unix epoch, i.e., 1970-Jan-01.
 */
template <typename Int = std::int32_t>
auto constexpr unix_epoch = date_t<Int>{1970, 1, 1};

/**
 * Signed to_days/to_date algorithms and information about these algoritm.
 *
 * Type members:
 *
 *    days_type       Type used to count days and store years. (Provided as a template argument.)
 *    date_type       Type used to store dates. (An instantiation of date_t.)
 *
 * Static data members:
 *
 *    epoch           Date used as epoch. (Provided as a template argument.)
 *
 *    days_min        Minimum count of days allowed as input to to_date.
 *    days_max        Maximum count of days allowed as input to to_date.
 *
 *    date_min        Minimum date allowed as input to to_days.
 *    date_max        Maximum date allowed as input to to_days.
 *
 *    round_days_min  Minimum count of days allowed as input to to_date for round trip.
 *    round_days_max  Maximum count of days allowed as input to to_date for round trip.
 *
 *    round_date_min  Minimum date allowed as input to to_days for round trip.
 *    round_date_max  Maximum date allowed as input to to_days for round trip.
 *
 * @tparam Int        Type used to count days and store years.
 * @tparam epoch      Date used as epoch.
 * @pre               std::is_signed_v<Int> && epoch.year >= 0
 */
 // Todo: (CN) Acount and document pre-conditions to avoid overflow when adding/subtracting offsets.
template <typename Int, date_t<Int> epoch = unix_epoch<Int>>
struct sdate_algos {

  static_assert(std::is_signed_v<Int>);
  static_assert(epoch.year >= 0);

  using days_type   = Int;
  using date_type   = date_t<Int>;

private:

  using ualgos      = udate_algos<std::make_unsigned_t<Int>>;
  using udays_type  = ualgos::days_type;
  using udate_type  = ualgos::date_type;

  static udays_type constexpr half_periods     = ualgos::days_max / 146097 / 2;
  static udays_type constexpr periods_in_epoch = epoch.year / 400;
  static udays_type constexpr remaining_days   = ualgos::to_days(udate_type{epoch.year % 400,
    epoch.month, epoch.day});

  static udays_type constexpr year_offset      = (half_periods - periods_in_epoch) * 400;
  static udays_type constexpr day_offset       = half_periods * 146097 + remaining_days;

  /**
   * Adjusts days to be given to unsigned algorithm to account for epoch.
   */
  static constexpr
  udays_type to_udays(days_type days) noexcept {
    return days + day_offset;
  }

  /**
   * Adjusts days returned from unsigned algorithm to account for epoch.
   */
  static constexpr
  days_type from_udays(udays_type days) noexcept {
    return days - day_offset;
  }

  /**
   * Adjusts date to be given to unsigned algorithm to account for epoch.
   */
  static constexpr
  udate_type to_udate(date_type const& date) noexcept {
    return { date.year + year_offset, date.month, date.day };
  }

  /**
   * Adjusts date returned from unsigned algorithm to account for epoch.
   */
  static constexpr
  date_type from_udate(udate_type const& date) noexcept {
    return { days_type(date.year - year_offset), date.month, date.day };
  }

public:

  static days_type constexpr days_min = from_udays(ualgos::days_min);
  static days_type constexpr days_max = from_udays(ualgos::days_max);

  /**
   * Returns the date corresponding to a given number of days since epoch.
   *
   * @param days      The given number of days.
   * @pre             days_min <= days && days <= days_max
   */
  static constexpr
  date_type to_date(days_type days) noexcept {
    // http://quick-bench.com/lHE2aolOkILburgCIpL-iQLbiG4
    return from_udate(ualgos::to_date(to_udays(days)));
  }

  static date_type constexpr date_min = from_udate(ualgos::date_min);
  static date_type constexpr date_max = from_udate(ualgos::date_max);

  /**
   * Returns the number of days from epoch to a given date.
   *
   * @param date      The given date.
   * @pre             date_min <= date && date <= date_max
   */
  static constexpr
  days_type to_days(date_type const& date) noexcept {
    // http://quick-bench.com/lKEBKGAqd3Ln7yJOBc3Cr8VL7DY
    return from_udays(ualgos::to_days(to_udate(date)));
  }

  static days_type constexpr round_days_min = std::max(days_min, to_days(date_min));
  static days_type constexpr round_days_max = std::min(days_max, to_days(date_max));

  static date_type constexpr round_date_min = to_date(round_days_min);
  static date_type constexpr round_date_max = to_date(round_days_max);

}; // struct sdate_algos

