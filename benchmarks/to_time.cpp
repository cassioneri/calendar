/*******************************************************************************
 *
 * to_time benchmarks
 *
 * Copyright (C) 2020 Cassio Neri and Lorenz Schneider
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
 * this file. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <array>
#include <cstdint>
#include <random>

//----------------------------
// Implementations
//----------------------------

// time_t is already defined.
struct time2_t {
  std::uint32_t hour;
  std::uint32_t minute;
  std::uint32_t second;
};

namespace neri_schneider {

  time2_t
  to_time(std::uint32_t n) noexcept {

    auto constexpr p32 = uint64_t(1) << 32;

    auto const u1 = std::uint64_t(1193047) * n;
    auto const h  = std::uint32_t(u1 / p32);
    auto const r  = std::uint32_t(u1 % p32) / 1193047;

    auto const u2 = std::uint64_t(71582789) * r;
    auto const m  = std::uint32_t(u2 / p32);
    auto const s  = std::uint32_t(u2 % p32) / 71582789;

    return { h, m, s };
  }

} // namespace neri_schneider

namespace ubiquitous {

  time2_t
  to_time(std::uint32_t n) noexcept {

    auto const h = n / 3600;
    auto const r = n % 3600;

    auto const m = r / 60;
    auto const s = r % 60;

    return { h, m, s };
  }

} // namespace ubiquitous

//----------------------------
// Benchmark data
//----------------------------

auto const ns = [](){
  std::uniform_int_distribution<std::uint32_t> uniform_dist(0, 86399);
  std::mt19937 rng;
  std::array<std::uint32_t, 16384> ns;
  for (auto& n : ns)
    n = uniform_dist(rng);
  return ns;
}();

//----------------------------
// Benchmark
//----------------------------

// If defined, likely to be running on quick-bench.
#ifndef BENCHMARK

  #include <benchmark/benchmark.h>

  void Scan(benchmark::State& state) {
    for (auto _ : state)
      for (auto const n : ns)
        benchmark::DoNotOptimize(n);
  }
  BENCHMARK(Scan);

#endif

#define DO_BENCHMARK(label, namespace)           \
  void label(benchmark::State& state) {          \
    for (auto _ : state)                         \
      for (auto const n : ns) {                  \
        auto const time = namespace::to_time(n); \
        benchmark::DoNotOptimize(time);          \
    }                                            \
  }                                              \
  BENCHMARK(label)                               \

DO_BENCHMARK(Ubiquitous   , ubiquitous    );
DO_BENCHMARK(NeriSchneider, neri_schneider);
