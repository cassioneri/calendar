/********************************************************************
 *
 * to_date : Copyright (C) 2020 Cassio Neri
 *
 *******************************************************************/

#include <cstdint>
#include <random>
#include <type_traits>

//-------------------------------------------------------------------
// Config
//-------------------------------------------------------------------

using year_t     = std::int16_t; // as in std::chrono::year
using month_t    = std::uint8_t; // as in std::chrono::month
using day_t      = std::uint8_t; // as in std::chrono::day
using rata_die_t = std::int32_t; // as in std::chrono::days

//-------------------------------------------------------------------
// Implementations
//-------------------------------------------------------------------

struct date_t {
  year_t  year;
  month_t month;
  day_t   day;
};

namespace neri {

  // https://github.com/cassioneri/dates/blob/master/date.hpp

  date_t constexpr
  to_date(rata_die_t rata_die) noexcept {
    using rata_die_t = std::make_unsigned_t<::rata_die_t>;
    auto const n  = rata_die_t(rata_die) + 536749361; // adjusted to unsigned unix epoch
    auto const n1 = 4 * n + 3;
    auto const c1 = n1 / 146097;
    auto const n2 = n1 % 146097 + c1 % 4;
    auto const c2 = n2 / 1461;
    auto const n3 = n2 % 1461 / 4;
    auto const y_ = 100 * c1 + c2;
    auto const m_ = (535 * n3 + 49483) / 16384;
    auto const d_ = n3 - (979 * m_ - 2922) / 32;
    auto const j  = n3 > 305;
    auto const y  = y_ + j;
    auto const m  = j ? m_ - 12 : m_;
    auto const d  = d_ + 1;
    return { year_t(y - 1467600), month_t(m), day_t(d) }; // adjusted to unsigned unix epoch
  }

} // namespace neri

namespace baum {

  // https://www.researchgate.net/publication/316558298_Date_Algorithms

  // Section 6.2.1/3
  date_t static constexpr
  to_date(rata_die_t rata_die) noexcept {
    auto const z  = std::uint32_t(rata_die) + 719469; // adjusted to unix epoch
    auto const h  = 100 * z - 25;
    auto const a  = h / 3652425;
    auto const b  = a - a / 4;
    auto const y_ = (100 * b + h) / 36525;
    auto const c  = b + z - 365 * y_ - y_ / 4;
    auto const m_ = (535 * c + 48950) / 16384;
    auto const d  = c - (979 * m_ - 2918) / 32;
    auto const j  = m_ > 12;
    auto const y  = y_ + j;
    auto const m  = j ? m_ - 12 : m_;
    return { year_t(y), month_t(m), day_t(d) };
  }

} // namespace baum

namespace boost {

  // https://github.com/boostorg/date_time/blob/4e1b7cde45edf8fdda73ec5c60053c9257138292/include/boost/date_time/gregorian_calendar.ipp#L109
  date_t constexpr
  to_date(rata_die_t dayNumber) noexcept {
    rata_die_t a = dayNumber + 32044;
    rata_die_t b = (4*a + 3)/146097;
    rata_die_t c = a-((146097*b)/4);
    rata_die_t d = (4*c + 3)/1461;
    rata_die_t e = c - (1461*d)/4;
    rata_die_t m = (5*e + 2)/153;
    day_t day = static_cast<day_t>(e - ((153*m + 2)/5) + 1);
    month_t month = static_cast<month_t>(m + 3 - 12 * (m/10));
    year_t year = static_cast<year_t>(100*b + d - 4800 + (m/10));
    return date_t{static_cast<year_t>(year),month,day};
  }

} // namespace boost

namespace dotnet {

  // https://github.com/dotnet/runtime/blob/master/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L102
  rata_die_t static constexpr s_daysToMonth365[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
  rata_die_t static constexpr s_daysToMonth366[] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

  // https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L64
  rata_die_t static constexpr DaysPerYear = 365;
  rata_die_t static constexpr DaysPer4Years = DaysPerYear * 4 + 1;       // 1461
  rata_die_t static constexpr DaysPer100Years = DaysPer4Years * 25 - 1;  // 36524
  rata_die_t static constexpr DaysPer400Years = DaysPer100Years * 4 + 1; // 146097

  // https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L938
  date_t static constexpr
  to_date(rata_die_t rata_die) noexcept {
      rata_die_t n = rata_die;
      rata_die_t y400 = n / DaysPer400Years;
      n -= y400 * DaysPer400Years;
      rata_die_t y100 = n / DaysPer100Years;
      if (y100 == 4) y100 = 3;
      n -= y100 * DaysPer100Years;
      rata_die_t y4 = n / DaysPer4Years;
      n -= y4 * DaysPer4Years;
      rata_die_t y1 = n / DaysPerYear;
      if (y1 == 4) y1 = 3;
      year_t year = y400 * 400 + y100 * 100 + y4 * 4 + y1 + 1;
      n -= y1 * DaysPerYear;
      bool leapYear = y1 == 3 && (y4 != 24 || y100 == 3);
      rata_die_t const* days = leapYear ? s_daysToMonth366 : s_daysToMonth365;
      month_t m = (n >> 5) + 1;
      while (n >= days[m]) m++;
      month_t month = m;
      day_t day = n - days[m - 1] + 1;
      return date_t{year, month, day};
  }

} // namespace dotnet

namespace glibc {

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l173
  unsigned short int static constexpr __mon_yday[2][13] =
    {
      /* Normal years.  */
      { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
      /* Leap years.  */
      { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
    };

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/time.h;h=015bc1c7f3b5d3db689f68de2a0c6ebbbc94f138#l179
  #define __isleap(year)	\
    ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/offtime.c;h=1415b1b4013834f39e4b0c11f0479dd866aab617#l24
  date_t static constexpr
  to_date(rata_die_t days) noexcept {

    rata_die_t y = 1970;
    #define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
    #define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

    while (days < 0 || days >= (__isleap (y) ? 366 : 365))
      {
        /* Guess a corrected year, assuming 365 days per year.  */
        rata_die_t yg = y + days / 365 - (days % 365 < 0);

        /* Adjust DAYS and Y to match the guessed year.  */
        days -= ((yg - y) * 365
                + LEAPS_THRU_END_OF (yg - 1)
                - LEAPS_THRU_END_OF (y - 1));
        y = yg;
      }

    auto ip = __mon_yday[__isleap(y)];
    rata_die_t m = 0;
    for (m = 11; days < (long int) ip[m]; --m)
      continue;
    days -= ip[m];
    return date_t{year_t(y), month_t(m + 1), day_t(days + 1)};
  }

} // namespace glibc

namespace hinnant {

  // https://github.com/llvm/llvm-project/blob/8e34be2f2511dfff7a8e3018bbd4188a93e446ea/libcxx/include/chrono#L2305
  date_t constexpr
  to_date(rata_die_t __d) noexcept {
    const int      __z = __d + 719468;
    const int      __era = (__z >= 0 ? __z : __z - 146096) / 146097;
    const unsigned __doe = static_cast<unsigned>(__z - __era * 146097);              // [0, 146096]
    const unsigned __yoe = (__doe - __doe/1460 + __doe/36524 - __doe/146096) / 365;  // [0, 399]
    const int      __yr = static_cast<int>(__yoe) + __era * 400;
    const unsigned __doy = __doe - (365 * __yoe + __yoe/4 - __yoe/100);              // [0, 365]
    const unsigned __mp = (5 * __doy + 2)/153;                                       // [0, 11]
    const unsigned __dy = __doy - (153 * __mp + 2)/5 + 1;                            // [1, 31]
    const unsigned __mth = __mp + (__mp < 10 ? 3 : -9);                              // [1, 12]
    return date_t{year_t(__yr + (__mth <= 2)), month_t(__mth), day_t(__dy)};
  }

} // namespace hinnant

//-------------------------------------------------------------------
// Benchmark data
//-------------------------------------------------------------------

auto const rata_dies = [](){
  std::uniform_int_distribution<rata_die_t> uniform_dist(-146097, 146096);
  std::mt19937 rng;
  std::array<std::int32_t, 65536> rata_dies;
  for (auto& rata_die : rata_dies)
    rata_die = uniform_dist(rng);
  return rata_dies;
}();

//-------------------------------------------------------------------
// Benchmark
//-------------------------------------------------------------------

void GLIBC(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& rata_die : rata_dies) {
      auto date = glibc::to_date(rata_die);
      benchmark::DoNotOptimize(date);
    }
  }
}
BENCHMARK(GLIBC);

void DotNet(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& rata_die : rata_dies) {
      auto date = dotnet::to_date(rata_die);
      benchmark::DoNotOptimize(date);
    }
  }
}
BENCHMARK(DotNet);

void Boost(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& rata_die : rata_dies) {
      auto date = boost::to_date(rata_die);
      benchmark::DoNotOptimize(date);
    }
  }
}
BENCHMARK(Boost);

void Hinnant(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& rata_die : rata_dies) {
      auto date = hinnant::to_date(rata_die);
      benchmark::DoNotOptimize(date);
    }
  }
}
BENCHMARK(Hinnant);

void Baum(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& rata_die : rata_dies) {
      auto date = baum::to_date(rata_die);
      benchmark::DoNotOptimize(date);
    }
  }
}
BENCHMARK(Baum);

void Neri(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& rata_die : rata_dies) {
      auto date = neri::to_date(rata_die);
      benchmark::DoNotOptimize(date);
    }
  }
}
BENCHMARK(Neri);
