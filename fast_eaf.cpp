/***************************************************************************************************
 *
 * Copyright (C) 2020 Cassio Neri and Lorenz Schneider
 *
 * This file is part of https://github.com/cassioneri/calendar.
 *
 * This file is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software  Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY  WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this file. If not,
 * see <https://www.gnu.org/licenses/>.
 *
 **************************************************************************************************/

/**
 * @file fast_eaf.cpp
 *
 * @brief Finds coefficients and upper bound of fast EAF.
 *
 * Compile with: g++ -O3 -std=c++2a fast_eaf.cpp -o fast_eaf
 */

#include <algorithm>
#include <cinttypes>
#include <cstring>
#include <iostream>

/**
 * @brief Coefficients of EAF.
 */
struct eaf_t {
  std::intmax_t alpha;
  std::intmax_t beta;
  std::intmax_t delta;
};

/**
 * @brief Coefficients and upper bound of fast EAFs.
 */
struct fast_eaf_t {
  eaf_t         fast;
  std::uint32_t k;
  std::intmax_t upper_bound;
};

std::ostream& operator <<(std::ostream& os, fast_eaf_t const& eaf) {
  return os <<
    "alpha'      = " << eaf.fast.alpha  << "\n"
    "beta'       = " << eaf.fast.beta   << "\n"
    "delta'      = " << eaf.fast.delta  << "\n"
    "k           = " << eaf.k           << "\n"
    "upper bound = " << eaf.upper_bound << "\n"
    ;
}

/**
 * @brief Finds coefficients and upper bound of fast EAF.
 * 
 * @param   k         Exponent of the divisor.
 * @param   eaf       Original EAF.
 */
fast_eaf_t constexpr
get_fast_eaf(bool round_up, std::uint32_t k, eaf_t const& eaf) noexcept {

  auto const two_k       = std::intmax_t(1) << k;
  auto const two_k_alpha = two_k * eaf.alpha;
  auto const div         = two_k_alpha / eaf.delta;
  auto const mod         = two_k_alpha % eaf.delta;
  auto const alpha_prime = round_up ? div + 1 : div;
  auto const epsilon     = round_up ? eaf.delta - mod : mod;
  
  // g(r) = alpha' * r - 2^k * f(r)
  auto g = [&](std::intmax_t r) {

    auto const num = eaf.alpha * r + eaf.beta;
    
    // Since operator / implements truncated division, we need to adjust negative numerators to get
    // the result of Euclidean division.
    auto const adjusted_num = num >= 0 ? num : num - (eaf.delta - 1);
    
    return alpha_prime * r - two_k * (adjusted_num / eaf.delta);
  };

  auto const beta_prime = [&]() {
    if (round_up) {
      auto min = g(0);
      for (std::intmax_t r = 1; r < eaf.delta; ++r)
        min = std::min(min, g(r));
      return -min;
    }
    else {
      auto max = g(0);
      for (std::intmax_t r = 1; r < eaf.delta; ++r)
        max = std::max(max, g(r));
      return two_k - 1 - max;
    }
  }();
  
  auto M = [&](std::intmax_t r) {
    if (round_up) {
      auto const num = two_k - (g(r) + beta_prime);
      if (num <= 0) return r;
      auto const q = (num + (epsilon - 1)) / epsilon;
      return q * eaf.delta + r;
    }
    else {
      auto const num = g(r) + beta_prime;
      if (num < 0) return r;
      auto const q = num / epsilon + 1;
      return q * eaf.delta + r;
    }
  };

  auto N = M(0);
  for (std::intmax_t r = 1; r < eaf.delta; ++r)
    N = std::min(N, M(r));
    
    return { { alpha_prime, beta_prime, two_k }, k, N };
}

int main(int argc, char* argv[]) {

  if (argc < 6) {
    std::cerr << argv[0] << ": requires at least 5 arguments: method, alpha, beta, delta and k\n"; 
    exit (1);
  }

  enum class method_t { up, down };
  method_t method;
  if (std::strncmp(argv[1], "up", 3) == 0)
    method = method_t::up;
  else if (std::strncmp(argv[1], "down", 5) == 0)
    method = method_t::down;
  else {
    std::cerr << argv[0] << ": unknown method '" << argv[1] << "'\n"; 
    exit (1);
  }
  
  char* end_ptr;

  auto const alpha = std::strtoimax(argv[2], &end_ptr, 10);
  if (end_ptr == argv[2]) {
    std::cerr << argv[0] << ": cannot parse alpha argument.\n";
    std::exit(1);
  }
  
  auto const beta  = std::strtoimax(argv[3], &end_ptr, 10);
  if (end_ptr == argv[3]) {
    std::cerr << argv[0] << ": cannot parse beta argument.\n";
    std::exit(1);
  }
  
  auto const delta = std::strtoimax(argv[4], &end_ptr, 10);
  if (end_ptr == argv[4]) {
    std::cerr << argv[0] << ": cannot parse delta argument.\n";
    std::exit(1);
  }
  
  if (alpha <= 0 || delta <= 0) {
    std::cerr << argv[0] << ": alpha and delta must be strictly positive.\n";
    std::exit(1);
  }
  
  auto const eaf = eaf_t{ alpha, beta, delta };
  
  for (std::int32_t i = 5; i < argc; ++i) {
    
    auto k = std::strtoimax(argv[i], &end_ptr, 10);
    if (end_ptr == argv[4]) {
      std::cerr << argv[0] << ": cannot parse k argument.\n";
      std::exit(1);
    }

    if (k < 1 || k > 63) {
      std::cerr << argv[0] << ": k must be in [1, 63] (skipping k = " << k << ")\n\n";
      continue;
    }
    
    auto const fast_eaf = get_fast_eaf(method == method_t::up, std::uint32_t(k), eaf);

    std::cout << fast_eaf << '\n';
  }
}
