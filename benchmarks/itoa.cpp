/*
 itoa benchmarks

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

// time_t is already defined.
struct ditits_t {
  char digits[10] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', 0 };
};

namespace neri_schneider {

ditits_t itoa(uint32_t n) {

  auto constexpr p32 = uint64_t(1) << 32;

  ditits_t digits;
  char* c = &digits.digits[8];

  do {
    auto const u = uint64_t(429496730) * n;
    auto const p = uint32_t(u / p32);
    auto const r = uint32_t(u % p32) / 429496730;
    n    = p;
    *c-- = '0' + char(r);
  } while (n);

  return digits;
}
}

namespace ubiquitous {

ditits_t itoa(uint32_t n) {

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
}

auto const ns = [](){
  std::uniform_int_distribution<uint32_t> uniform_dist(0, 999999999);
  std::mt19937 rng;
  std::array<uint32_t, 16384> ns;
  for (auto& n : ns)
    n = uniform_dist(rng);
  return ns;
}();

#ifndef BENCHMARK
  // Not on quick-bench
  #include <benchmark/benchmark.h>
  void Scan(benchmark::State& state) {
    for (auto _ : state)
      for (auto const n : ns)
        benchmark::DoNotOptimize(n);
  }
  BENCHMARK(Scan);

#endif

#define DO_BENCHMARK(label, namespace) \
  void label(benchmark::State& state) { \
    for (auto _ : state) \
      for (auto const n : ns) { \
        auto const digits = namespace::itoa(n); \
        benchmark::DoNotOptimize(digits); \
    } \
  } \
  BENCHMARK(label)

DO_BENCHMARK(Ubiquitous, ubiquitous);
DO_BENCHMARK(NeriSchneider, neri_schneider);
