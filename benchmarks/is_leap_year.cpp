/*******************************************************************************
 *
 * is_leap_year benchmarks
 *
 * Copyright (C) 2020 Cassio Neri
 *
 * This file is part of https://github.com/cassioneri/calendar.
 *
 * This file is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this file. If not,
 * see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <cstdint>
#include <random>

//------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------

using year_t = std::int16_t; // as in std::chrono::year

//------------------------------------------------------------------------------
// Implementations
//------------------------------------------------------------------------------

namespace neri {

  // https://github.com/cassioneri/calendar/blob/master/calendar.hpp

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

namespace ubiquitous {

  bool constexpr
  is_leap_year(year_t y) noexcept {
    return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
  }

} // namespace typical

//------------------------------------------------------------------------------
// Benchmark data
//------------------------------------------------------------------------------

auto const years = [](){
  std::uniform_int_distribution<year_t> uniform_dist(-400, 399);
  std::mt19937 rng;
  std::array<year_t, 16384> years;
  for (auto& year : years)
    year = uniform_dist(rng);
  return years;
}();

//------------------------------------------------------------------------------
// Benchmark
//------------------------------------------------------------------------------

void Ubiquitous(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& year : years) {
      auto b = ubiquitous::is_leap_year(year);
      benchmark::DoNotOptimize(b);
    }
  }
}
BENCHMARK(Ubiquitous);

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
