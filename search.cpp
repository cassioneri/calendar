/***************************************************************************************************
 *
 * Copyright (C) 2020 Cassio Neri
 *
 * This file is part of https://github.com/cassioneri/dates.
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

#include <cstdint>
#include <iostream>
#include <ratio>
#include <utility>

// Compile with: g++ -O3 -std=c++2a search.cpp -o search

using integer_t = std::uint32_t;

/**
 * Searcher of coefficients a, b and c such that f(n) = (a * n + b) / c gives results matching
 * expectations defined by a Tester class. For c, only powers of 2 are considered.
 *
 * For instance, for n in [0, 11] let g(n) = (153 * n + 2) / 5. This class searches for a, b and c
 * such c is a power of 2 and g(n) = f(n) for all n in [0, 11].
 *
 * @tparam Tester  Implements a static function test such that test(a, b, c)) == true if, and only
 *                 if, f mathes the expected results.
 * @tparam Hint    Lower bound hint for a / c. It must be an instantiation of std::ratio where
 *                 Hint::num == 1 or Hint::den == 1.
 */
template <typename Tester, typename Hint>
struct finder {

  static_assert(Hint::num == 1 || Hint::den == 1);

  integer_t static constexpr
  calc(integer_t n, integer_t a, integer_t b, integer_t c) noexcept {
    return (a * n + b) / c;
  }

  /**
   * Brute force search for coefficients.
   *
   * It stops when coefficients are found or when combinations are exhausted. In the former case,
   * it prints out the found coefficients.
   */
  void static constexpr
  find() noexcept {

    for (integer_t c = 1; ; ) {

      auto const a_min = (c + Hint::den - 1) / Hint::den; // = ceil(c * num / den)
      auto const a_max = Hint::den == 1 ? c * (Hint::num + 1) : c / (Hint::den - 1);

      for (integer_t a = a_min; a < a_max; ++a) {
        for (integer_t b = 0; b < a; ++b) {
          if (Tester::test(a, b, c)) {
            std::cout << "a = " << a << ", b = " << b << ", c = " << c << ".\n";
            return;
          }
        }
      }

      if (c == (~integer_t(0)) / 2 + 1)
        return;

      c <<= 1;
    }
  }
};

/**
 * For n = [0, 11], let m = n > 9 ? n - 9 : n + 3. Let d denote the first day of month m and e
 * denote the 1st of March preceding or on d. The mounth count, f(n) is defined by the number of
 * dates in [e, d[. It is known that g(n) = (153 * n + 2) / 5. This class is to be used as a Tester
 * class of finder that tests for month count.
 */
struct month_count {

  using finder = ::finder<month_count, std::ratio<30, 1>>;

  integer_t static constexpr
  calc(integer_t n) noexcept {
    return (153 * n + 2) / 5;
  }

  bool static constexpr
  test(integer_t a, integer_t b, integer_t c) noexcept {
    for (integer_t n = 0; n < 12; ++n)
      if (finder::calc(n, a, b, c) != calc(n))
        return false;
    return true;
  }
};

/**
 * Search coefficients for function that returns the month for a given day of year.
 */
struct month {

  using finder = ::finder<month, std::ratio<1, 31>>;

  bool static constexpr
  test(integer_t a, integer_t b, integer_t c) noexcept {
    for (integer_t n = 0; n < 12; ++n) {
      // The month is m = n > 9 ? n - 9 : n + 3
      if (finder::calc(month_count::calc(n), a, b, c) != n ||
        finder::calc(month_count::calc(n + 1) - 1, a, b, c) != n)
        return false;
    }
    return true;
  }
};

/**
 * Search coefficients for function that returns the year for a given day of century.
 */
struct year_of_century {

  using finder = ::finder<year_of_century, std::ratio<1, 366>>;

  integer_t static constexpr
  year_count(integer_t y) noexcept {
    return 1461 * y / 4;
  }

  bool static constexpr
  test(integer_t a, integer_t b, integer_t c) noexcept {
    for (integer_t y = 0; y < 100; ++y)
      if (finder::calc(year_count(y), a, b, c) != y ||
        finder::calc(year_count(y + 1) - 1, a, b, c) != y)
        return false;
    return true;
  }
};

int main() {

  std::cout << "Coefficients for month_count: ";
  // Result: a = 979, b =  15, c = 32.
  month_count::finder::find();

  std::cout << "Coefficients for month from day of year: ";
  // Result: a = 535, b = 331, c = 16384.
  month::finder::find();

  std::cout << "Coefficients for year of century: ";
  // For integer_t = std::uint32_t finishes after around 15min and no coefficients are found.
  // For integer_t = std::uint64_t cannot find coefficients in a reasonable amount of time.
  year_of_century::finder::find();
}
