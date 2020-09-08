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
 * @file fast_eaf.hpp
 * 
 * @brief Utilities for searching fast EAFs.
 */

#include <cstdint>
#include <cstdlib>
#include <iostream>

/**
 * @brief Coefficients of EAF.
 */
struct eaf_t {
  std::int64_t alpha;
  std::int64_t beta;
  std::int64_t delta;
};

/**
 * @brief Coefficients and upper bound of fast EAFs.
 */
struct fast_eaf_t {
  eaf_t        fast;
  std::int32_t k;
  std::int64_t upper_bound;
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
 * @brief Parse the command line to extract and check EAF coeffitients.
 * 
 * If validation fails, then this functions call std::exit.
 * 
 * @param   argc      Number of command line arguments. (As passed to the main function.)
 * @param   argv      Command line arguments. (As passed to the main function.)
 */
eaf_t
parse_cmd_line_and_exit_if_invalid(int argc, char* const argv[]) noexcept {
  
  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " alpha beta delta\n";
    std::exit(1);
  }
  
  auto alpha = std::atoi(argv[1]);
  auto beta  = std::atoi(argv[2]);
  auto delta = std::atoi(argv[3]);

  if (alpha <= 0 || delta <= 0) {
    std::cerr << "error: alpha and delta must be strictly positive.\n";
    std::exit(1);
  }
  
  if ((delta & (delta - 1)) == 0) {
    std::cerr << "error: delta must not be a power of two.\n";
    std::exit(1);
  }
  
  return { alpha, beta, delta };
}
