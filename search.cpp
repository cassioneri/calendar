#include <cstdint>
#include <iostream>
#include <ratio>
#include <utility>

// Compile with: g++ -O3 -std=c++2a search.cpp -o search

using integer_t = std::uint32_t;

/**
 * CRTP base class that helps finding coefficients a, b and c such that f(n) = (a * n + b) / c gives
 * results matching expectations defined by the derived class. For c, only powers of 2 are
 * considered.
 *
 * For instance, if n denotes the month (numbered as follows: Mar = 0, Apr = 1, May = 2, Jun = 3,
 * Jul = 4, Aug = 5, Sep = 6, Oct = 7, Nov = 8, Dec = 9, Jan = 10 and Feb = 11), then we might
 * search for coefficients such that f(n) returns the number of days between 01/Mar (inclusive) to
 * first of month n (exlusive). More precisely,
 *
 *   f(0) =  0 since there are  0 days from 01/Mar to 01/Mar;
 *   f(1) = 31 since there are 31 days from 01/Mar to 01/Apr (# days in Mar);
 *   f(2) = 61 since there are 61 days from 01/Mar to 01/May (# days in Mar + Apr);
 *   ...
 *
 * @tparam Derived Derived class. It must implement a function test such that
 *                 static_cast<bool>(test(a, b, c)) == true if, and only if, f mathes the expected
 *                 results.
 * @tparam Hint    Lower bound hint for a / c. It must be an instantiation of std::ratio where
 *                 Hint::num == 1 or Hint::den == 1.
 */
template <typename Derived, typename Hint>
struct finder {

  static_assert(Hint::num == 1 || Hint::den == 1);

  static integer_t calc(integer_t n, integer_t a, integer_t b, integer_t c) {
    return (a * n + b) / c;
  }

  /**
   * Brute force search for coefficients.
   *
   * It stops when coefficients are found or when combinations are exhausted. In the former case,
   * it prints out the found coefficients.
   */
  void find() const {

    for (integer_t c = 1; ; ) {

      auto const a_min = (c + Hint::den - 1) / Hint::den; // = ceil(c * num / den)
      auto const a_max = Hint::den == 1 ? c * (Hint::num + 1) : c / (Hint::den - 1);

      for (integer_t a = a_min; a < a_max; ++a) {
        for (integer_t b = 0; b < a; ++b) {
          if (static_cast<Derived const*>(this)->test(a, b, c)) {
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
 * Table with month limits.
 *
 * Months are numbered as follows: Mar = 0, Apr = 1, May = 2, Jun = 3, Jul = 4, Aug = 5, Sep = 6,
 * Oct = 7, Nov = 8, Dec = 9, Jan = 10 and Feb = 11. Then,
 *
 * month_limits[month][0] := number of days between 01/Mar (inclusive) and first of month (exlusive).
 * month_limits[month][1] := number of days between 01/Mar (inclusive) and last  of month (exlusive).
 */
static integer_t constexpr month_limits[12][2] = {
  // Mar         Apr         May         Jun         Jul         Aug
  {  0,  30}, { 31,  60}, { 61,  91}, { 92, 121}, {122, 152}, {153, 183},
  // Sep         Oct         Nov         Dec         Jan         Feb
  {184, 213}, {214, 244}, {245, 274}, {275, 305}, {306, 336}, {337, 365}
};

/**
 * Search coefficients for function that returns the month for a given the number of days since
 * 01/Mar (inclusive).
 */
struct month_from_day_of_year : public finder<month_from_day_of_year, std::ratio<1, 31>> {

  bool test(integer_t a, integer_t b, integer_t c) const {
    for (int m = 0; m < 12; ++m)
      if (calc(month_limits[m][0], a, b, c) != m || calc(month_limits[m][1], a, b, c) != m)
        return false;
    return true;
  }
};

/**
 * Search coefficients for function that returns the number of days between 01/Mar (inclusive) and
 * first of month (exlusive) for a given month.
 */
struct days_since_1st_Mar : public finder<days_since_1st_Mar, std::ratio<30, 1>> {

  bool test(integer_t a, integer_t b, integer_t c) const {
    for (int m = 0; m < 12; ++m)
      if (calc(m, a, b, c) != month_limits[m][0])
        return false;
    return true;
  }
};

/**
 * Search coefficients for function that returns the year for a given day of century.
 */
struct year_of_century : public finder<year_of_century, std::ratio<1, 366>> {

  bool test(integer_t a, integer_t b, integer_t c) const {
    for (integer_t d = 0; d < 36525; ++d)
      if (calc(d, a, b, c) != (4 * d + 3 / 1461))
        return false;
    return true;
  }
};

int main() {

  std::cout << "Coefficients for month from day of year: ";
  // Result: a = 535, b = 331, c = 16384.
  month_from_day_of_year().find();

  std::cout << "Coefficients for days since 01-Mar: ";
  // Result: a = 979, b =  15, c = 32.
  days_since_1st_Mar().find();

  std::cout << "Coefficients for year of century: ";
  // For integer_t = std::uint32_t finishes after around 15min and no coefficients are found.
  // For integer_t = std::uint64_t cannot find coefficients in a reasonable amount of time.
  year_of_century().find();
}

