# Date algorithms

## TL;DR

Contains low-level date algorithms that can be used to implement the following C++20's functions:

    std::chrono::year::is_leap()                                 // a.k.a. is_leap_year
    std::chrono::year_month_day_last::day()                      // a.k.a. last_day_of_month
    std::chrono::year_month_day::operator sys_days()             // a.k.a. to_rata_die
    std::chrono::year_month_day::year_month_day(const sys_days&) // a.k.a. to_date

This work is similar to those by Peter Baum [[1]](#baum) and Howard Hinnant [[2]](#hinnant).
Many ideas implemented here were inspired by their work. Benchmark results suggest that
implementations here perform considerably faster than theirs and also than those of GLIBC, .NET and
boost:

![Benchmarks](https://github.com/cassioneri/dates/blob/master/benchmarks/benchmarks.png)

(See live [[3]](http://quick-bench.com/kug8p_gx6wTrmgh4BkBSalIz3Zs),
[[4]](http://quick-bench.com/dz8cCjlCinmQR7XRJPty3rK3x74),
[[5]](http://quick-bench.com/Lw0iZnKj3ubLOlXUkz7_EzmH0CY) and
[[6]](http://quick-bench.com/s0Y67aXtkV-7krxXKZ9hOLJTen0).)

`is_leap_year`: Neri_mcomp is 3.3x faster than Hinnant. Neri_mod is 2x faster than Hinnant.

`last_day_of_month`: Neri is 1.1x faster than Boost and 1.1x faster than Hinnant.

`to_rata_die`: Neri is 2.5x faster than GLIBC, 1.9x faster than .NET, 1.7x fater than boost, 1.6x
faster than Hinnant and 1.3x faster than Baum.

`to_date`: Neri is 5.9x faster than GLIBC, 2.7x faster than .NET, 2.1x faster than boost, 2.1x
faster than Hinnant and 1.5x faster than Baum.

**Disclaimer**: Benchmarks above compare implementations as of 2020-May-02. They have been edited,
mainly to get consistent function signatures across implementations. Storage types for years,
months, days and day counts might have been changed for closer compliance with C++20 requirements.
Some original implementations deal with time and for this benchmark we have used simplified versions
that only deals with dates.

Tests show correctness and compliance with the C++ Standard, that is, the algorithms are strictly
increasing 1-to-1 maps between dates in [-32768-Jan-01, 32767-Dec-31] and day counts in [-12687794,
11248737] with 1970-Jan-01 being mapped into 0.

Implementations are split into building blocks allowing configurable features. For those only
interested in the case specified by the C++ Standard (previous paragraph), ready to copy-and-paste
implementations can be seen in the benchmark links above.

The configurable features allow even better performance than showed by the benchmarks above at the
expense of diverging from the C++ Standard choices on types, ranges and epoch. The best performance
is achieved when only dates after 0000-Mar-01 are allowed.

## Design choices

The code does not implement a full date library. However, despite its limited usage, some design
choices were made with generality and performance in mind.

Configurable features include:

1. The epoch for the rata die.
2. The type used to store rata dies.
3. The type used to store years.

The ranges of possible dates and rata dies depend on the choices above. These ranges are calculated
by the code and are available for evaluations in `constexpr` contexts.

The most important functions are implemented in two template classes:

1. `ugregorian_t`
2. `gregorian_t`

The former assumes years and rata dies are positive and the epoch is 0000-Mar-01. The rationale is
that `unsigned` computations are generally faster than `signed` ones. (For instance, [division by
constants](https://godbolt.org/z/4JxB4J).) Therefore `ugregorian_t` gets better performance by
working only on `unsigned` integers.

The latter is more configurable than ugregorian_t allowing negative years and rata dies. It also
allows different epochs. (By default, the epoch is the Unix date 1970-Jan-01.) This is a thin but
not free layer class around `ugregorian_t`. Indeed, each function in `gregorian_t` simply adapts
inputs and outputs (generally through one addition and one subtraction) before/after delegating to a
corresponding function in `ugregorian_t`.

## Contents

1. `date.hpp`   : Implementations.
2. `tests.cpp`  : Tests. (Including exhaustive full-range round-trips taking around 30s to complete
on commodity hardware.)
3. `search.cpp` : Helper program that was used to find some coefficients used by the algorithms.

## References

[1] <span id="baum"> Peter Baum, *Date Algorithms*,
  https://www.researchgate.net/publication/316558298_Date_Algorithms <br>
[2] <span id="hinnant"> Howard Hinnant, *chrono-Compatible Low-Level Date Algorithms*,
  https://howardhinnant.github.io/date_algorithms.html <br>
[3] <span id="is_leap_year"> Cassio Neri, *`is_leap_year` benchmarks*,
  http://quick-bench.com/kug8p_gx6wTrmgh4BkBSalIz3Zs <br>
[4] <span id="last_day_of_month"> Cassio Neri, *`last_day_of_month` benchmarks*,
  http://quick-bench.com/dz8cCjlCinmQR7XRJPty3rK3x74 <br>
[5] <span id="to_rata_die"> Cassio Neri, *`to_rata_die` benchmarks*,
  http://quick-bench.com/Lw0iZnKj3ubLOlXUkz7_EzmH0CY <br>
[6] <span id="to_date"> Cassio Neri, *`to_date` benchmarsk*,
  http://quick-bench.com/s0Y67aXtkV-7krxXKZ9hOLJTen0 <br>
