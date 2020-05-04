/********************************************************************
 *
 * is_leap_year : Copyright (C) 2020 Cassio Neri
 *
 *******************************************************************/

#include <cstdint>
#include <random>

//-------------------------------------------------------------------
// Config
//-------------------------------------------------------------------

using year_t = std::int16_t; // as in std::chrono::year

//-------------------------------------------------------------------
// Implementations
//-------------------------------------------------------------------

namespace neri {

  // https://github.com/cassioneri/dates/blob/master/date.hpp

  bool constexpr
  is_leap_year_mod(year_t year) noexcept {
    return (year % 100 != 0 || year % 400 == 0) & (year % 4 == 0);
  }

  bool constexpr
  is_multiple_of_100(std::int32_t n) {
    std::uint32_t constexpr multiplier   = 42949673;
    std::uint32_t constexpr bound        = 42949669;
    std::uint32_t constexpr max_dividend = 1073741799;
    std::uint32_t constexpr offset       = max_dividend / 2 / 100 * 100;
    return multiplier * (n + offset) < bound;
  }

  bool constexpr
  is_leap_year_mcomp(year_t year) noexcept {
    return (!is_multiple_of_100(year) || year % 16 == 0) & (year % 4 == 0);
  }

} // namespace neri

namespace hinnant {

  // https://github.com/llvm/llvm-project/blob/8e34be2f2511dfff7a8e3018bbd4188a93e446ea/libcxx/include/chrono#L1777
  bool constexpr
  is_leap_year(year_t __y) noexcept {
    return __y % 4 == 0 && (__y % 100 != 0 || __y % 400 == 0);
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

//-------------------------------------------------------------------
// Benchmark
//-------------------------------------------------------------------

void Hinnant(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& year : years) {
      auto b = hinnant::is_leap_year(year);
      benchmark::DoNotOptimize(b);
    }
  }
}
BENCHMARK(Hinnant);

void Neri_mod(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& year : years) {
      auto b = neri::is_leap_year_mod(year);
      benchmark::DoNotOptimize(b);
    }
  }
}
BENCHMARK(Neri_mod);

void Neri_mcomp(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& year : years) {
      auto b = neri::is_leap_year_mcomp(year);
      benchmark::DoNotOptimize(b);
    }
  }
}
BENCHMARK(Neri_mcomp);
