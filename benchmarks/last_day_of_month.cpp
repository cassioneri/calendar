/********************************************************************
 *
 * last_day_of_month : Copyright (C) 2020 Cassio Neri
 *
 *******************************************************************/

#include <cstdint>
#include <random>

//-------------------------------------------------------------------
// Config
//-------------------------------------------------------------------

using year_t  = std::int16_t; // as in std::chrono::year
using month_t = std::uint8_t; // as in std::chrono::month
using day_t   = std::uint8_t; // as in std::chrono::day

//-------------------------------------------------------------------
// Implementations
//-------------------------------------------------------------------

namespace neri {

  // https://github.com/cassioneri/dates/blob/master/date.hpp

  bool constexpr
  is_multiple_of_100(std::int32_t n) {
    std::uint32_t constexpr multiplier   = 42949673;
    std::uint32_t constexpr bound        = 42949669;
    std::uint32_t constexpr max_dividend = 1073741799;
    std::uint32_t constexpr offset       = max_dividend / 2 / 100 * 100;
    return multiplier * (n + offset) < bound;
  }

  bool constexpr
  is_leap_year(year_t year) noexcept {
    return (!is_multiple_of_100(year) || year % 16 == 0) & (year % 4 == 0);
  }

  day_t constexpr
  last_day_of_month(year_t year, month_t month) noexcept {
    return month != 2 ? ((month ^ (month >> 3)) & 1) | 30 :
      is_leap_year(year) ? 29 : 28;
  }

} // namespace neri

namespace boost {

  // https://github.com/boostorg/date_time/blob/4e1b7cde45edf8fdda73ec5c60053c9257138292/include/boost/date_time/gregorian_calendar.ipp#L161
  bool constexpr
  is_leap_year(year_t year) {
    return (!(year % 4))  && ((year % 100) || (!(year % 400)));
  }

  // https://github.com/boostorg/date_time/blob/4e1b7cde45edf8fdda73ec5c60053c9257138292/include/boost/date_time/gregorian_calendar.ipp#L175
  day_t constexpr
  last_day_of_month(year_t year, month_t month) noexcept {
    switch (month) {
    case 2:
      if (is_leap_year(year)) {
        return 29;
      } else {
        return 28;
      };
    case 4:
    case 6:
    case 9:
    case 11:
      return 30;
    default:
      return 31;
    };
  }

} // namespace boost

namespace hinnant {

  // https://github.com/llvm/llvm-project/blob/8e34be2f2511dfff7a8e3018bbd4188a93e446ea/libcxx/include/chrono#L1777
  bool constexpr
  is_leap_year(year_t __y) noexcept {
    return __y % 4 == 0 && (__y % 100 != 0 || __y % 400 == 0);
  }

  // https://github.com/llvm/llvm-project/blob/8e34be2f2511dfff7a8e3018bbd4188a93e446ea/libcxx/include/chrono#L2447
  day_t constexpr
  last_day_of_month(year_t __y, month_t month) noexcept {
    day_t constexpr __d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return month != 2 || !is_leap_year(__y) ?
        __d[static_cast<unsigned>(month - 1)] : day_t{29};
  }

} // namespace hinnant

//-------------------------------------------------------------------
// Benchmark data
//-------------------------------------------------------------------

auto const years = [](){
  std::uniform_int_distribution<year_t> uniform_dist(-400, 399);
  std::mt19937 rng;
  std::array<year_t, 65536> years;
  for (auto& year : years)
    year = uniform_dist(rng);
  return years;
}();

auto const months = [](){
  std::uniform_int_distribution<month_t> uniform_dist(1, 12);
  std::mt19937 rng;
  std::array<month_t, 65536> months;
  for (auto& month : months)
    month = uniform_dist(rng);
  return months;
}();

//-------------------------------------------------------------------
// Benchmark
//-------------------------------------------------------------------

void Boost(benchmark::State& state) {
  for (auto _ : state) {
    for (std::int32_t i = 0; i < 65536; ++i) {
      auto day = hinnant::last_day_of_month(years[i], months[i]);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(Boost);

void Hinnant(benchmark::State& state) {
  for (auto _ : state) {
    for (std::int32_t i = 0; i < 65536; ++i) {
      auto day = hinnant::last_day_of_month(years[i], months[i]);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(Hinnant);

void Neri(benchmark::State& state) {
  for (auto _ : state) {
    for (std::int32_t i = 0; i < 65536; ++i) {
      auto day = neri::last_day_of_month(years[i], months[i]);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(Neri);
