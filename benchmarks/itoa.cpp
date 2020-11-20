/*******************************************************************************
 *
 * itoa benchmarks
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
struct ditits_t {
  char digits[10] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', 0 } ;
};

namespace neri_schneider {

  ditits_t
  itoa(std::uint32_t n) noexcept {

    auto constexpr p32 = uint64_t(1) << 32;
    
    ditits_t digits;
    char* c = &digits.digits[8];

    do {
      auto const u = std::uint64_t(429496730) * n;
      auto const p = std::uint32_t(u / p32);
      auto const r = std::uint32_t(u % p32) / 429496730;
      n    = p;
      *c-- = '0' + char(r);
    } while (n);

    return digits;
  }

} // namespace neri_schneider

namespace ubiquitous {

  ditits_t
  itoa(std::uint32_t n) noexcept {

    ditits_t digits;
    char* c = &digits.digits[8];

    do {
      auto const p = n / 10;
      auto const r = n % 10;
      n    = p;
      *c-- = '0' + char(r);
    } while (n);

    return digits;
  }

} // namespace ubiquitous

//----------------------------
// Benchmark data
//----------------------------

auto const ns = [](){
  std::uniform_int_distribution<std::uint32_t> uniform_dist(0, 999999999);
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

#define DO_BENCHMARK(label, namespace)          \
  void label(benchmark::State& state) {         \
    for (auto _ : state)                        \
      for (auto const n : ns) {                 \
        auto const digits = namespace::itoa(n); \
        benchmark::DoNotOptimize(time);         \
    }                                           \
  }                                             \
  BENCHMARK(label)                              \

DO_BENCHMARK(Ubiquitous   , ubiquitous    );
DO_BENCHMARK(NeriSchneider, neri_schneider);
