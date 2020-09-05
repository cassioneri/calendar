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
 * @file simple_fast_eaf.cpp
 * 
 * @brief Finds coefficients and upper bound of simple fast EAF.
 *
 * Compile with: g++ -O3 -std=c++2a simple_fast_eaf.cpp -o simple_fast_eaf
 */

#include "fast_eaf.hpp"

#include <cstdint>
#include <iostream>

/**
 * @brief Finds coefficients and upper bound of simple fast EAF.
 * 
 * @param   k         Exponent of the divisor.
 * @param   eaf       Original EAF.
 */
fast_eaf_t constexpr
get_simple_fast_eaf(std::int32_t k, eaf_t const& eaf) noexcept {
  auto const two_k = std::int64_t(1) << k;
  auto const mu    = two_k / eaf.delta + 1;
  auto const nu    = eaf.delta - two_k % eaf.delta;
  auto const N     = (mu / nu + (mu % nu != 0)) * eaf.delta - 1;
  return { { mu * eaf.alpha, mu * eaf.beta, two_k }, k, (nu <= mu ? N : 0) };
}

int main(int argc, char* argv[]) {

  auto eaf = parse_cmd_line_and_exit_if_invalid(argc, argv);
  
  for (std::int32_t k = 1; k <= 32; ++k) {
    auto const fast_eaf = get_simple_fast_eaf(k, eaf);
    std::cout << fast_eaf << '\n';
  }
}
