# Date algorithms

This repo contains low-level date algorithms that can be used to implement the following C++20's
functions:

    std::chrono::year_month_day::year_month_day(const sys_days&) // a.k.a. to_date
    std::chrono::year_month_day::operator sys_days()             // a.k.a. to_count
    std::chrono::year::is_leap()
    std::chrono::year_month_day_last::day()

This work was inspired by Howard Hinnant's excellent paper [[1]](https://howardhinnant.github.io/date_algorithms.html)
on date algorithms. The following benchmark results suggest that implementations here perform
considerably faster (up to 3.5x) than Hinnant's:
![to_date](https://github.com/cassioneri/dates/blob/master/benchmarks.png)

(See live [[2]](http://quick-bench.com/3Fvm8jIhDA-DzMZKyOtDffMeDF0),
[[3]](http://quick-bench.com/Alkp9m6RusQhMQNkKh-ArhFdfRM) and
[[4]](http://quick-bench.com/BRo2jU8FDDt1jKqAhTwRasFPoXI).)

# Design choices

The code does not implement a full date library (even less C++20's). However, despite its limited
usage, some design choices were made with generality and performance in mind.

Configurable features include:

1. The epoch from which dates are counted.
2. The type used to count number of days since epoch.
3. The type used to store years.

The ranges of possible dates and day counts depend on the choices above. These ranges are calculated
by the code and are available for evaluations in `constexpr` contexts.

The most important functions are implemented in two template classes:

1. `udate_algos`
2. `sdate_algos`

The former assumes years and day counts are positive and the epoch is 0000-Mar-01. The rationale is
that `unsigned` computations are generally faster than `signed` ones. (For instance, [division by
constants](https://godbolt.org/z/4JxB4J).) Therefore `udate_algos` gets better performance by
working only on `unsigned` integers.

The latter is more configurable allowing for negative years and day counts as well as different
epochs (By default, the epoch is the Unix date 1970-Jan-01.) This is a thin but not free layer
class around `udate_algos`. Indeed, each function in `sdate_algos` simply adapts inputs and outputs
(generally through one addition and one subtraction) before/after delegating to a corresponding
function in `udate_algos`.

# Contents

1. `date.hpp`   : Implementations.
2. `date.cpp`   : Tests. (Including exhastive full-range round-trips taking around 30s to complete
on commodity hardware.)
3. `search.cpp` : Helper program that was used to find some coefficients used by the algorithms.

# References

[1] Howard Hinnant, *chrono-Compatible Low-Level Date Algorithms*, https://howardhinnant.github.io/date_algorithms.html<br>
[2] Cassio Neri, *`to_date` benchmark*, http://quick-bench.com/3Fvm8jIhDA-DzMZKyOtDffMeDF0<br>
[3] Cassio Neri, *`to_count` benchmark*, http://quick-bench.com/Alkp9m6RusQhMQNkKh-ArhFdfRM<br>
[4] Cassio Neri, *`is_leap_year` benchmark*, http://quick-bench.com/BRo2jU8FDDt1jKqAhTwRasFPoXI<br>

