/*
 to_time benchmarks

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
struct time2_t {
  uint32_t hour;
  uint32_t minute;
  uint32_t second;
};

namespace neri_schneider {

time2_t to_time(uint32_t n) {

  auto constexpr p32 = uint64_t(1) << 32;

  auto const u1 = uint64_t(1193047) * n;
  auto const h  = uint32_t(u1 / p32);
  auto const r  = uint32_t(u1 % p32) / 1193047;

  auto const u2 = uint64_t(71582789) * r;
  auto const m  = uint32_t(u2 / p32);
  auto const s  = uint32_t(u2 % p32) / 71582789;

  return { h, m, s };
}
}

namespace ubiquitous {

time2_t to_time(uint32_t n) {

  auto const h = n / 3600;
  auto const r = n % 3600;

  auto const m = r / 60;
  auto const s = r % 60;

  return { h, m, s };
}
}

auto const ns = [](){
  std::uniform_int_distribution<uint32_t> uniform_dist(0, 86399);
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
        auto const time = namespace::to_time(n); \
        benchmark::DoNotOptimize(time); \
    } \
  } \
  BENCHMARK(label)

DO_BENCHMARK(Ubiquitous, ubiquitous);
DO_BENCHMARK(NeriSchneider, neri_schneider);
