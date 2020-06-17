# Gregorian calendar algorithms

## TL;DR

This is a pracademic project on Gregorian calendar algorithms. Its academic side will be shown by
a paper (to appear) that reviews literature and brings new mathematical insights. For practioners,
the project provides working implementations that could be used in many pieces of software including
the following C++20's functions:

    std::chrono::year::is_leap()                                 // a.k.a. is_leap_year
    std::chrono::year_month_day_last::day()                      // a.k.a. last_day_of_month
    std::chrono::year_month_day::operator sys_days()             // a.k.a. to_rata_die
    std::chrono::year_month_day::year_month_day(const sys_days&) // a.k.a. to_date

Our implementations are benchmarked against counterparts, including some of popular and widely used
libraries (glibc, .NET, Boost and LLVM). Results, as per charts below, suggest that our
implementations perform considerably faster than others.

![Benchmarks](https://github.com/cassioneri/dates/blob/master/benchmarks/benchmarks.png)

[is_leap_year](http://quick-bench.com/0HV3XYJeGuN9mgomWtMbixF28C0): Neri_mcomp is 3.3x faster than
Ubiquitous (the implementation used virtually everywhere.) Neri_mod is 2x faster than Ubiquitous.

[last_day_of_month](http://quick-bench.com/SLM-7N7CUCaTmEewgs2OZ6JBTjc): Neri is 3.9x faster than
Boost and 1.2x faster than LLVM.

[to_rata_die](http://quick-bench.com/mE3H4OV2zFQCmh4rGwIMfju5sxQ): Neri is 3.1x faster than
Reingold, 2.2x faster than glibc, 2.1x faster than Hatcher, 1.8x faster than .NET, 1.7x fater than
Boost, 1.6x faster than LLVM and 1.3x faster than Baum.

[to_date](http://quick-bench.com/uWz8_ZgBxochr8v78f67wtnsaCo): Neri is 7x faster than Reingold, 6.1x
faster than glibc, 3.5x faster than .NET, 2.9x faster than Hatcher, 2.6x faster than Boost, 2.2x
faster than LLVM and 1.5x faster than Baum.

**Disclaimer**: Benchmarks above compare implementations as of 2020-May-02. They might have been
slightly edited to get: (a) consistent function signatures; (b) storage types (for years, months,
days and day counts) closer to C++20 requirements; (c) consistent epoch (unix time 1970-Jan-01).
Some originals deal with date and time but the variants used here work on dates only.

Tests show correctness and compliance with the C++ Standard, that is, `to_rata_die` and `to_date`
are strictly increasing 1-to-1 maps between dates in [-32768-Jan-01, 32767-Dec-31] and day counts in
[-12687794, 11248737] with 1970-Jan-01 being mapped into 0.

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
