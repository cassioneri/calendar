#include "date.hpp"

#include <cstdint>
#include <iostream>

// Compile with: g++ -O3 -std=c++2a date.cpp -o date

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

template <typename T>
bool constexpr
operator ==(date_t<T> const& x, date_t<T> const& y) noexcept {
  return x.year == y.year && x.month == y.month && x.day == y.day;
}

template <typename T>
bool constexpr
operator !=(date_t<T> const& x, date_t<T> const& y) noexcept {
  return !(x == y);
}

template <typename T>
bool constexpr
operator <(date_t<T> const& x, date_t<T> const& y) noexcept {
  if (x.year  < y.year ) return true;
  if (x.year  > y.year ) return false;
  if (x.month < y.month) return true;
  if (x.month > y.month) return false;
  return x.day < y.day;
}

template <typename T>
bool constexpr
operator <=(date_t<T> const& x, date_t<T> const& y) noexcept {
  return !(y < x);
}

template <typename T>
std::ostream&
operator <<(std::ostream& os, date_t<T> const& d) {
  return os << d.year << '-' << (int) d.month << '-' << (int) d.day;
}

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

//--------------------------------------------------------------------------------------------------
// Information
//--------------------------------------------------------------------------------------------------

template <typename Algo>
void
print() {

  std::cout << "date_min        = " << Algo::date_min        << '\n';
  std::cout << "date_max        = " << Algo::date_max        << '\n';

  std::cout << "count_min       = " << Algo::count_min       << '\n';
  std::cout << "count_max       = " << Algo::count_max       << '\n';

  std::cout << "round_date_min  = " << Algo::round_date_min  << '\n';
  std::cout << "round_date_max  = " << Algo::round_date_max  << '\n';

  std::cout << "round_count_min = " << Algo::round_count_min << '\n';
  std::cout << "round_count_max = " << Algo::round_count_min << '\n';
}

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

void // could be constexpr but... No!
test_is_multiple_of_100() {
  for (std::int32_t n = -536870800; n <= 536870999; ++n)
    if ((n % 100 == 0) != is_multiple_of_100(n))
      std::cout << "test_is_multiple_of_100 failed for n = " << n << '\n';
}

template <typename A>
void // could be constexpr but... No!
round_trip_test() noexcept {

  // Compile-time checks.

  static_assert(A::round_count_min == A::to_count(A::round_date_min));
  static_assert(A::round_count_max == A::to_count(A::round_date_max));

  static_assert(A::round_date_min == A::to_date(A::round_count_min));
  static_assert(A::round_date_max == A::to_date(A::round_count_max));

  // Runtime checks.

  for (auto n = A::round_count_min; n <= A::round_count_max; ++n)
    if (n != A::to_count(A::to_date(n)))
      std::cout << "round_trip_test failed for n = " << n << '\n';
}

void constexpr
standard_compliance_test() noexcept {

  using algos  = sdate_algos<std::int32_t>;
  using date_t = algos::date_t;

  // https://eel.is/c++draft/time.clock.system#overview-1
  static_assert(algos::to_date(0) == date_t{1970, 1, 1});

  // https://eel.is/c++draft/time.cal.ymd#members-20
  static_assert(algos::round_count_min <= -12687428);
  static_assert(algos::round_count_max >=  11248737);
}

template <typename A>
void
to_date_test() noexcept {

  auto date = A::to_date(A::count_min);

  for (auto count = A::count_min; count < A::count_max; ) {
    auto const tomorrow = A::to_date(++count);
    if (tomorrow != advance(date))
      std::cout << "to_date_test failed for count = " << count << '\n';
  }
}

template <typename A>
void
to_count_test() noexcept {

  auto count = A::to_count(A::date_min);

  for (auto date = A::date_min; date < A::date_max; ) {
    auto const tomorrow = A::to_count(advance(date));
    if (tomorrow != ++count)
      std::cout << "to_count_test failed for date = " << date << '\n';
  }
}

int
main() {

  std::cout << "--------------------\n";
  std::cout << "Preliminary tests:  \n";
  std::cout << "--------------------\n";

  test_is_multiple_of_100();

  std::cout << '\n';

  std::cout << "--------------------\n";
  std::cout << "Unsigned algorithms:\n";
  std::cout << "--------------------\n";

  using ualgos = udate_algos<std::uint32_t>;

  print<ualgos>();
  round_trip_test<ualgos>();
  to_date_test<ualgos>();
  to_count_test<ualgos>();

  std::cout << '\n';

  std::cout << "--------------------\n";
  std::cout << "Signed algorithms:  \n";
  std::cout << "--------------------\n";

  using salgos = sdate_algos<std::int32_t>;

  print<salgos>();
  round_trip_test<salgos>();
  to_date_test<salgos>();
  to_count_test<salgos>();
}

