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

#include <algorithm>
#include <array>
#include <iostream>

/**
 * @file troesch.cpp
 *
 * Coefficients search algorithm by Troesch [1].
 *
 * Usage:
 *
 * troesch X1 X2 [Xn]...
 *
 * Tell if (X1, X2, ..., Xn) is the code of a line or not and, if so, then if also outputs the
 * equation line. For instance, for the Gregorian months from March to February (regardless of leap
 * year) we have
 *
 * $ ./troesch 31 30 31 30 31 31 30 31 30 31 31 30
 * The line is y = (153 * x + 2) / 5.
 *
 * This means that (153 * x + 2) / 5 is the sum of all elememts of the vector {31, 30, 31, 30, 31,
 * 31, 30, 31, 30, 31, 31, 30 } prior to index x.
 *
 * [1] Albert Troesch, Droites discrètes et calendriers, Mathématiques et sciences humaines, tome
 *     141 (1998), p. 11-41.
 *
 * Compile with: g++ -O3 -std=c++2a troesch.cpp -o troesch
 */

/**
 * @brief Code container.
 */
using code_t = std::vector<int>;

/**
 * @brief   Returns the difference between the maximum and the minimun of a code_t.
 *
 * @param   c         The given code.
 *
 * @pre               !c.empty();
 */
int amplitude(code_t const& c) {
  auto const [iterator_min, iterator_max] = std::minmax_element(std::begin(c), std::end(c));
  return *iterator_max - *iterator_min;
}

/**
 * @brief   Returns true if code elements are in {x, x + 1} for some integer x.
 *
 * @param   c         The given code.
 *
 * @pre               !c.empty();
 */
int has_at_most_two_consecutives(code_t const& c) {
  return amplitude(c) <= 1;
}

/**
 * @brief   Returns true if all code elements are equal.
 *
 * @param   c         The given code.
 *
 * @pre               !c.empty();
 */
bool is_constant(code_t const& c) {
  return amplitude(c) == 0;
}

/**
 * @brief   Returns the code's minimum element.
 *
 * @param   c         The given code.
 *
 * @pre               !c.empty();
 */
int min_element(code_t const& c) {
  return *min_element(std::cbegin(c), std::cend(c));
}

/**
 * @brief   Subtracts a given number from all elements of a given code.
 *
 * @param   p         The given number.
 * @param   c         The given code.
 */
void substract_element(int p, code_t& c) {
  for (auto& y : c)
    y -= p;
}

/**
 * @brief   Returns true if the code does not contain two consecutive elements equal to 1.
 *
 * @param   c         The given code.
 */
bool is_1_isolated(code_t const& c) {
  auto const size = std::size(c);
  for (auto i = 1; i < size; ++i) {
    if (c[i] == 1 && c[i - 1] == 1)
      return false;
  }
  return true;
}

/**
 * @brief   Replace all 1 elements of a given code with 0 and vive versa.
 *
 * @param   c         The given code.
 *
 * @pre               c[i] == 1 || c[i] == 0 for all i in {0, ..., c.size() - 1}
 */
void swap_0_1(code_t& c) {
  for (auto& y : c)
    y = 1 - y;
}

/**
 * @brief   Replace code elements with plateau lengths.
 *
 * @param   c         The given code_t.
 *
 * @pre               c[i] == 1 || c[i] == 0 for all i in {0, ..., c.size() - 1}
 */
auto replace_with_lengths(code_t& c) {

  auto const size = std::size(c);

  int index_previous_1 = -1;
  int first_length     = -1;
  int n_plateaus       = 0;
  int min_lenght       = size; // works as if +infinity

  for (int i = 0; i < size; ++i) {
    if (c[i] == 1) {
      ++n_plateaus;
      auto const length = i - index_previous_1;
      index_previous_1 = i;
      if (n_plateaus == 1)
        first_length = length;
      else if (length > min_lenght)
          min_lenght = length;
    }
  }
  auto const is_terminal = c.back() == 0;
  n_plateaus += is_terminal;
  int last_length = is_terminal ? size - index_previous_1 : 0;

  code_t lengths;

  if (n_plateaus > 2) {

    auto const skip_first = first_length <= min_lenght;
    if (!skip_first)
      lengths.push_back(first_length);

    index_previous_1 = first_length - 1;
    for (int i = first_length; i < size; ++i) {
      if (c[i] == 1) {
        lengths.push_back(i - index_previous_1);
        index_previous_1 = i;
      }
    }

    if (last_length > min_lenght)
      lengths.push_back(first_length);

    c = std::move(lengths);
    return skip_first ? first_length : 0;
  }

  if (n_plateaus == 1 || first_length >= last_length) {
    c.resize(1);
    c[0] = first_length;
    return 0;
  }

  c.resize(1);
  c[0] = last_length;
  return first_length;
}

/**
 * Return the remainder of Euclidean division of n by d.
 *
 * @param   n         The dividend.
 * @param   d         The divisor.
 *
 * @pre               d != 0.
 */
auto mod(int n, int d) {
  auto r = n % d;
  if (r < 0)
    r += d > 0 ? d: - d;
  return r;
}

/**
 * @brief Result of Troesch's algorithm.
 */
struct result_t {
  bool is_line;
  int  a;
  int  b;
  int  r;
};

/**
 * @brief Runs Troesch's algorithm on a given a code.
 */
result_t troesch(code_t& c) {

  std::vector<char> e; // avoiding vector<bool>;
  code_t p, g;
  int n;

  auto increment_n = [&]() {
    ++n;
    p.resize(n + 1);
    e.resize(n + 1);
    g.resize(n + 1);
  };

  bool is_line = has_at_most_two_consecutives(c);
  n = 0;
  while (is_line && !is_constant(c)) {
    increment_n();
    p[n] = min_element(c);
    substract_element(p[n], c);
    e[n] = !is_1_isolated(c);
    if (e[n]) {
      increment_n();
      swap_0_1(c);
    }
    increment_n();
    g[n] = replace_with_lengths(c);
    is_line = has_at_most_two_consecutives(c);
  }

  if (!is_line)
    return { false, 0, 0, 0 };

  auto a = c[0];
  auto b = 1;
  auto r = 0;
  while (n > 0) { // Error in Troesch's article: while (n >= 0)
    --n;
    std::swap(a, b);
    r = a - 1 - r;
    r = mod(r - g[n + 1] * a, b);
    if (e[n - 1]) { // Error in Troesch's article: if (e[n])
      --n;
      a = b - a;
      r = b - 1 - r;
    }
    --n;
    a = a + p[n + 1] * b;
  }
  return { true, a, b, r };
}

int main(int argc, char* argv[]) {

  code_t c;

  for (int i = 1; i < argc; ++i)
    c.push_back(std::atoi(argv[i]));

  auto results = troesch(c);
  if (results.is_line)
    std::cout << "The line is y = (" << results.a << " * x + " << results.r << ") / " <<
      results.b << ".\n";
  else
    std::cout << "This is not the code of a line.\n";
}
