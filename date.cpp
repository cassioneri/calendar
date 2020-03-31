#include "date.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

// Compile with: g++ -O3 -std=c++2a date.cpp -o date

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

template <typename T>
bool constexpr
operator == (date_t<T> const& x, date_t<T> const& y) noexcept {
  return x.year == y.year && x.month == y.month && x.day == y.day;
}

template <typename T>
bool constexpr
operator < (date_t<T> const& x, date_t<T> const& y) noexcept {
  if (x.year  < y.year ) return true;
  if (x.year  > y.year ) return false;
  if (x.month < y.month) return true;
  if (x.month > y.month) return false;
  return x.day < y.day;
}

template <typename T>
bool constexpr
operator <= (date_t<T> const& x, date_t<T> const& y) noexcept {
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

  std::cout << "date_min       = " << Algo::date_min       << '\n';
  std::cout << "date_max       = " << Algo::date_max       << '\n';

  std::cout << "days_min       = " << Algo::days_min       << '\n';
  std::cout << "days_max       = " << Algo::days_max       << '\n';

  std::cout << "round_date_min = " << Algo::round_date_min << '\n';
  std::cout << "round_date_max = " << Algo::round_date_max << '\n';

  std::cout << "round_days_min = " << Algo::round_days_min << '\n';
  std::cout << "round_days_max = " << Algo::round_days_max << '\n';
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

  static_assert(A::round_days_min == A::to_days(A::round_date_min));
  static_assert(A::round_days_max == A::to_days(A::round_date_max));

  static_assert(A::round_date_min == A::to_date(A::round_days_min));
  static_assert(A::round_date_max == A::to_date(A::round_days_max));

  // Runtime checks.

  for (auto n = A::round_days_min; n <= A::round_days_max; ++n)
    if (n != A::to_days(A::to_date(n)))
      std::cout << "round_trip_test failed for n = " << n << '\n';
}

void constexpr
standard_compliance_test() noexcept {

  using algos     = sdate_algos<std::int32_t>;
  using date_type = algos::date_type;

  // https://eel.is/c++draft/time.clock.system#overview-1
  static_assert(algos::to_date(0) == date_type{1970, 1, 1});

  // https://eel.is/c++draft/time.cal.ymd#members-20
  static_assert(algos::round_days_min <= -12687428);
  static_assert(algos::round_days_max >=  11248737);
}

template <typename A>
void
to_date_test() noexcept {

  auto date = A::to_date(A::days_min);

  for (auto days = A::days_min; days < A::days_max; ) {
    auto const tomorrow = A::to_date(++days);
    assert(tomorrow == advance(date));
  }
}

template <typename A>
void
to_days_test() noexcept {

  auto days = A::to_days(A::date_min);

  for (auto date = A::date_min; date < A::date_max; ) {
    auto const tomorrow = A::to_days(advance(date));
    assert(tomorrow == ++days);
  }
}

int
main() {

  std::cout << "--------------------------------------------------------------------------------\n";
  std::cout << "Preliminary tests:\n";
  std::cout << "--------------------------------------------------------------------------------\n";

  test_is_multiple_of_100();

  std::cout << '\n';

  std::cout << "--------------------------------------------------------------------------------\n";
  std::cout << "Unsigned algorithms:\n";
  std::cout << "--------------------------------------------------------------------------------\n";

  using ualgos = udate_algos<std::uint32_t>;

  print<ualgos>();
  round_trip_test<ualgos>();
  to_date_test<ualgos>();
  to_days_test<ualgos>();

  std::cout << '\n';

  std::cout << "--------------------------------------------------------------------------------\n";
  std::cout << "Signed algorithms:\n";
  std::cout << "--------------------------------------------------------------------------------\n";

  using salgos = sdate_algos<std::int32_t>;

  print<salgos>();
  round_trip_test<salgos>();
  to_date_test<salgos>();
  to_days_test<salgos>();
}

