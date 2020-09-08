/***************************************************************************************************
 *
 * Copyright (C) 2020 Cassio Neri
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

#include "fast_eaf.hpp"

#include <algorithm>
#include <iostream>
#include <cstdint>

/**
 * @brief Finds coefficients and upper bound of fast EAF.
 * 
 * @param   k         Exponent of the divisor.
 * @param   eaf       Original EAF.
 */
fast_eaf_t constexpr
get_fast_eaf(std::int32_t k, eaf_t const& eaf) noexcept {

  auto const two_k       = std::int64_t(1) << k;
  auto const two_k_alpha = two_k * eaf.alpha;
  auto const div         = two_k_alpha / eaf.delta;
  auto const mod         = two_k_alpha % eaf.delta;
  auto const round_up    = mod > eaf.delta - mod;
  auto const alpha_prime = round_up ? div + 1 : div;
  auto const nu          = round_up ? eaf.delta - mod : mod;
  
  auto f = [&](std::int64_t r) {

    auto const num = eaf.alpha * r + eaf.beta;
    
    // Since operator / implements truncated division, we need to adjust negative numerators to get
    // the result of Euclidean division.
    auto const adjusted_num = num >= 0 ? num : num - (eaf.delta - 1);
    
    return alpha_prime * r - two_k * (adjusted_num / eaf.delta);
  };

  auto const beta_prime = [&]() {
    if (round_up) {
      auto min = f(0);
      for (std::int64_t r = 1; r < eaf.delta; ++r)
        min = std::min(min, f(r));
      return -min;
    }
    else {
      auto max = f(0);
      for (std::int64_t r = 1; r < eaf.delta; ++r)
        max = std::max(max, f(r));
      return two_k - max - 1;
    }
  }();
  
  auto Nr = [&](std::int64_t r) {
    if (round_up) {
      auto const num = two_k - (f(r) + beta_prime);
      if (num <= 0) return r;
      auto const q = (num + (nu - 1)) / nu;
      return q * eaf.delta + r;
    }
    else {
      auto const num = f(r) + beta_prime;
      if (num < 0) return r;
      auto const q = num / nu + 1;
      return q * eaf.delta + r;
    }
  };

  auto N = Nr(0);
  for (std::int64_t r = 1; r < eaf.delta; ++r)
    N = std::min(N, Nr(r));
    
    return { { alpha_prime, beta_prime, two_k }, k, N };
}

int main(int argc, char* argv[]) {

  auto eaf = parse_cmd_line_and_exit_if_invalid(argc, argv);
  
  for (std::int32_t k = 1; k <= 32; ++k) {
    auto const fast_eaf = get_fast_eaf(k, eaf);
    std::cout << fast_eaf << '\n';
  }
}
