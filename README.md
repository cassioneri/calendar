# Gregorian calendar algorithms

## This repository is obsolete.</span>

A research article [[2]](#EAF-spe) on the topic has been published by Wiley and a new [repository](https://github.com/cassioneri/eaf) for the article's supplementary is now available.

## TL;DR

This is a pracademic project on Gregorian calendar algorithms. Its academic side is supported by the
mathematical study of Euclidean Affine Functions (EAF for short) [[1]](#EAF-arxiv). For practioners,
this project provides working implementations that could be used in many pieces of software
including the following C++20's functions:

    std::chrono::year::is_leap()                                 // a.k.a. is_leap_year
    std::chrono::year_month_day_last::day()                      // a.k.a. last_day_of_month
    std::chrono::year_month_day::operator sys_days()             // a.k.a. to_rata_die
    std::chrono::year_month_day::year_month_day(const sys_days&) // a.k.a. to_date

Our implementations are benchmarked against counterparts, including some of the most widely used
C (glibc), C++ (boost and libc++), C# (.NET) and Java (OpenJDK and Android)
libraries; and our own implementations of algorithms found in academic literature. Charts below
suggest that our algorithms perform considerably faster than others.

#### Notes:
  April 2021 - Our algorithms were implemented in [libstdc++](https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/include/std/chrono;hb=b92d12d3fe3f1aa56d190d960e40c62869a6cfbb#l2453)
    released with gcc 11.0.

  June 2021 - One of our algorithms was implemented in the [Linux Kernel](https://github.com/torvalds/linux/blob/276010551664f73b6f1616dde471d6f0d63a73ba/kernel/time/timeconv.c#L77)
    released with version 5.14. Another [implementation](https://github.com/torvalds/linux/blob/1d1bb12a8b1805ddeef9793ebeb920179fb0fa38/drivers/rtc/lib.c#L69)
    in the RTC subsystem is expected on a forthcoming release.

![Benchmarks](https://github.com/cassioneri/calendar/blob/master/benchmarks/benchmarks.png)

[to_date](https://quick-bench.com/q/hfPkKtGaBZFhF5dCR7Ha0ms_Z38): NeriSchneider is
1.5x faster than Baum,
1.4x faster than Boost,
3.7x faster than .NET,
2.7x faster than FliegelFlandern,
6.5x faster than glibc,
2.8x faster than Hatcher,
2.2x faster than libc++,
2.2x faster than OpenJDK and
6.9x faster than ReingoldDershowitz.

[to_rata_die](https://quick-bench.com/q/SXJ_w2Lq_F_EDkhINNCV8ZnvCz0): NeriSchneider is
1.5x faster than Baum,
1.7x faster than Boost,
1.7x faster than .NET,
2.2x faster than FliegelFlandern,
2.2x faster than glibc,
2.3x faster than Hatcher,
1.7x faster than libc++,
2.5x faster than OpenJDK and
3.2x faster than ReingoldDershowitz.

[is_leap_year](https://quick-bench.com/q/gK6a9RUDJYFnbgBX-h_xECTxF9E): Ubiquitous (the
implementation used virtually everywhere) is
2.6x slower than NeriSchneider_mod,
3.3x slower than NeriSchneider_mcomp,
1.9x slower than Drepper,
3.3x slower than DrepperNeriSchneider_mcomp1 and
3.6x slower than DrepperNeriSchneider_mcomp2.

[last_day_of_month](https://quick-bench.com/q/jpbILbbUq5AeGKds3etV5gwMCyk): NeriSchneider is
2.0x faster than Boost and
1.1x faster than libc++.

**Disclaimer**: Benchmarks above show a single run and YMMV (especially when changing compiler since
they might emit substantially different instructions). They compare implementations as of
2020-May-02 which were slightly edited to get consistent: (a) function signatures; (b) storage types
(for years, months, days and day counts) closer to C++20 requirements; (c) epoch (unix time
1970-Jan-01). Some originals deal with date and time but the variants used here work on dates only.

Tests show correctness and compliance with the C++ Standard, that is, `to_rata_die` and `to_date`
are strictly increasing 1-to-1 maps between dates in [-32768-Jan-01, 32767-Dec-31] and day counts in
[-12687794, 11248737] with 1970-Jan-01 being mapped into 0.

Implementations are split into building blocks allowing configurable features. For those only
interested in the case specified by the C++ Standard (previous paragraph), ready to copy-and-paste
implementations can be seen in the benchmark links above.

The configurable features allow even better performance than showed by the benchmarks above at the
expense of diverging from the C++ Standard choices on types, ranges and epoch. The best performance
is achieved when only dates after 0000-Mar-01 are allowed.

### Other examples

The same techniques used for calendar algorithms can be used to other problems. For instance,
conversion from elapsed seconds since the start of the day to hour, minute and seconds, is
customarily implemented through division by 3600 and 60. Similarly, implementations of itoa usualy
perform repeated division by 10. Charts below suggest improvements for these problems.

![Benchmarks](https://github.com/cassioneri/calendar/blob/master/benchmarks/others.png)

[to_time](https://quick-bench.com/q/n21P53DGEDPtIp6wu3YZ1ebLLtg): NeriSchneider is 1.2x faster than
Ubiquitous.

[itoa](https://quick-bench.com/q/-iNSxF1zQFE6LgUwbyuN8ae_4CQ): NeriSchneider is 1.1x faster than
Ubiquitous.

## Improving instruction level parallelism of division and modular calculations.

Example 3.12 of [[1]](#EAF-arxiv) provides an alternative for the evaluations of q = n / 1461 and
r = n % 1461, claiming it foster instruction-level parallelism implemented by superscalar processors
and they profit from the backward compatibility features that drove the design of the x86_64
instruction set. Document ![Parallelism](https://github.com/cassioneri/calendar/blob/master/Parallelism.md)
provides more details.

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

1. `calendar.hpp` : Implementations.
2. `tests.cpp`    : Tests.
3. `fast_eaf.cpp` : Fast EAF algorithms.
4. `troesch.cpp`  : Coefficients search algorithm by Albert Troesch.

## References

<span id="EAF-arxiv">[1] Cassio Neri and Lorenz Schneider,
*Euclidean Affine Functions and Applications to Calendar Algorithms*,
[preprint](https://arxiv.org/abs/2102.06959), February 2021.<br>
    
<span id="EAF-spe">[2] Cassio Neri and Lorenz Schneider,
*Euclidean affine functions and their application tocalendar algorithms*. Softw Pract Exper. 2022;1-34. [doi: 10.1002/spe.3172](https://onlinelibrary.wiley.com/doi/full/10.1002/spe.3172).
