/*
 is_leap_year benchmarks

 Copyright (C) 2020 Cassio Neri and Lorenz Schneider

 This file is part of https://github.com/cassioneri/calendar.

 This file is free software: you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option) any later
 version.

 This file is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 See <https://www.gnu.org/licenses/>.
*/

#include <array>
#include <cstdint>
#include <random>

// https://github.com/cassioneri/calendar/blob/master/calendar.hpp

using year_t = int16_t;

// https://accu.org/journals/overload/28/155/overload155.pdf#page=16
bool is_multiple_of_100(int32_t n) {
  uint32_t constexpr multiplier   = 42949673;
  uint32_t constexpr bound        = 42949669;
  uint32_t constexpr max_dividend = 1073741799;
  uint32_t constexpr offset       = max_dividend / 2 / 100 * 100;
  return multiplier * (n + offset) < bound;
}

namespace neri_schneider::mod {
bool is_leap_year(year_t year) {
  // The ternary expression used here is equivalent to:
  //   year % 100 == 0 ? year % 16 == 0 : year % 4 == 0;
  // Surprinsingly, GCC generates worse code for it.
  return year % 100 == 0 ? year % 400 == 0 : year % 4 == 0;
}
}

namespace neri_schneider::mcomp {
bool is_leap_year(year_t year) {
  return is_multiple_of_100(year) ? year % 16 == 0 : year % 4 == 0;
}
}

// Ulrich Drepper.
namespace drepper {
bool is_leap_year(year_t year) {
  return (year & (year % 100 == 0 ? 15 : 3)) == 0;
}
}

namespace drepper_neri_schneider::mcomp1 {
bool is_leap_year(year_t year) {
  return (year & (is_multiple_of_100(year) ? 15 : 3)) == 0;
}
}

namespace drepper_neri_schneider::mcomp2 {
bool is_leap_year(year_t year) {
  // https://accu.org/journals/overload/28/155/overload155.pdf#page=16
  uint32_t constexpr multiplier         = 42949673;
  uint32_t constexpr bound              = 42949669;
  uint32_t constexpr max_dividend       = 1073741799;
  uint32_t constexpr offset             = max_dividend / 2 / 100 * 100;
  uint32_t const     sum                = year + offset;
  bool     const     is_multiple_of_100 = multiplier * sum < bound;
  // offset & 15 == 0 and hence, year & 15 == sum & 15.
  return (sum & (is_multiple_of_100 ? 15 : 3)) == 0;
}
}

namespace ubiquitous {
bool is_leap_year(year_t y) {
  return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}
}

auto const years = [](){
  std::uniform_int_distribution<year_t> uniform_dist(-400, 399);
  std::mt19937 rng;
  std::array<year_t, 16384> years;
  for (auto& year : years)
    year = uniform_dist(rng);
  return years;
}();

#ifndef BENCHMARK
  // Not on quick-bench
  #include <benchmark/benchmark.h>
  void Scan(benchmark::State& state) {
    for (auto _ : state)
      for (auto const year : years)
        benchmark::DoNotOptimize(year);
  }
  BENCHMARK(Scan);
#endif

#define DO_BENCHMARK(label, namespace) \
  void label(benchmark::State& state) { \
    for (auto _ : state) { \
      for (auto const year : years) { \
        auto const b = namespace::is_leap_year(year); \
        benchmark::DoNotOptimize(b); \
      } \
    } \
  } \
  BENCHMARK(label)

DO_BENCHMARK(Ubiquitous, ubiquitous);
DO_BENCHMARK(NeriSchneider_mod , neri_schneider::mod);
DO_BENCHMARK(NeriSchneider_mcomp, neri_schneider::mcomp);
DO_BENCHMARK(Drepper, drepper);
DO_BENCHMARK(DrepperNeriSchneider_mcomp1, drepper_neri_schneider::mcomp1);
DO_BENCHMARK(DrepperNeriSchneider_mcomp2, drepper_neri_schneider::mcomp2);
