/********************************************************************
 *
 * to_rata_die : Copyright (C) 2020 Cassio Neri
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

  rata_die_t constexpr
  to_rata_die(date_t const& x) noexcept {
    using rata_die_t = std::make_unsigned_t<::rata_die_t>;
    auto const y  = rata_die_t(x.year) + 1467600; // adjusted to unsigned unix epoch
    auto const m  = rata_die_t(x.month);
    auto const d  = rata_die_t(x.day);
    auto const j  = rata_die_t(m < 3);
    auto const d_ = d - 1;
    auto const m_ = j ? m + 12 : m;
    auto const y_ = y - j;
    auto const c  = y_ / 100;
    return (1461 * y_ / 4 - c + c / 4) + (979 * m_ - 2922) / 32 + d_
      - 536749361; // adjusted to unsigned unix epoch
  }

} // namespace neri

namespace baum {

  // https://www.researchgate.net/publication/316558298_Date_Algorithms

  // Section 5.1
  rata_die_t static constexpr
  to_rata_die(date_t date) noexcept {
    auto const j = date.month < 3;
    auto const z = date.year - j;                    // step 1 / alternative 2
    auto const m = j ? date.month + 12 : date.month; // step 2 / alternative 3
    auto const f = (979 * m - 2918) / 32;            //
    return rata_die_t{date.day + f +                 // step 3 (adjusted to unix epoch)
      365 * z + z / 4 - z / 100 + z / 400 - 719469};
  }

} // namespace baum

namespace boost {

  // https://github.com/boostorg/date_time/blob/4e1b7cde45edf8fdda73ec5c60053c9257138292/include/boost/date_time/gregorian_calendar.ipp#L68
  rata_die_t constexpr
  to_rata_die(const date_t& ymd) noexcept {
    unsigned short a = static_cast<unsigned short>((14-ymd.month)/12);
    unsigned short y = static_cast<unsigned short>(ymd.year + 4800 - a);
    unsigned short m = static_cast<unsigned short>(ymd.month + 12*a - 3);
    unsigned long  d = ymd.day + ((153*m + 2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045;
    return static_cast<rata_die_t>(d);
  }

} // namespace boost

namespace dotnet {

  // https://github.com/dotnet/runtime/blob/master/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L102
  rata_die_t static constexpr s_daysToMonth365[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
  rata_die_t static constexpr s_daysToMonth366[] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

  //https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L1123
  bool static constexpr
  IsLeapYear(year_t year) noexcept {
    return (year & 3) == 0 && ((year & 15) == 0 || (year % 25) != 0);
  }

  //https://github.com/dotnet/runtime/blob/bddbb03b33162a758e99c14ae821665a647b77c7/src/libraries/System.Private.CoreLib/src/System/DateTime.cs#L625
  rata_die_t static constexpr
  to_rata_die(date_t date) noexcept {
    rata_die_t const* days = IsLeapYear(date.year) ? s_daysToMonth366 : s_daysToMonth365;
    rata_die_t y = date.year - 1;
    rata_die_t n = y * 365 + y / 4 - y / 100 + y / 400 + days[date.month - 1] + date.day - 1;
    return n;
  }

} // namespace dotnet

namespace glibc {

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l138
  rata_die_t static constexpr
  shr(rata_die_t a, int b) noexcept {
    rata_die_t one = 1;
    return (-one >> 1 == -1 ? a >> b : a / (one << b) - (a % (one << b) < 0));
  }

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l157
  #define EPOCH_YEAR 1970
  #define TM_YEAR_BASE 1900

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l161
  bool static constexpr
  leapyear (rata_die_t year) noexcept {
    return ((year & 3) == 0 && (year % 100 != 0 ||
      ((year / 100) & 3) == (- (TM_YEAR_BASE / 100) & 3)));
  }

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l173
  unsigned short int static constexpr __mon_yday[2][13] =
    {
      /* Normal years.  */
      { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
      /* Leap years.  */
      { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
    };

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l194
  rata_die_t static constexpr
  ydhms_diff (rata_die_t year1, rata_die_t yday1, rata_die_t year0) noexcept {
    rata_die_t a4 = shr (year1, 2) + shr (TM_YEAR_BASE, 2) - ! (year1 & 3);
    rata_die_t b4 = shr (year0, 2) + shr (TM_YEAR_BASE, 2) - ! (year0 & 3);
    rata_die_t a100 = a4 / 25 - (a4 % 25 < 0);
    rata_die_t b100 = b4 / 25 - (b4 % 25 < 0);
    rata_die_t a400 = shr (a100, 2);
    rata_die_t b400 = shr (b100, 2);
    rata_die_t intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);
    rata_die_t years = year1 - year0;
    rata_die_t days = 365 * years + yday1 + intervening_leap_days;
    return days;
  }

  // https://sourceware.org/git/?p=glibc.git;a=blob;f=time/mktime.c;h=63c82fc6a96848b1f1e34164e7ce696035635fc6;hb=HEAD#l312
  rata_die_t static constexpr
  to_rata_die(date_t date) noexcept {
    rata_die_t mday     = date.day;
    rata_die_t mon      = date.month - 1;
    rata_die_t year     = date.year - TM_YEAR_BASE;
    rata_die_t mon_yday = (__mon_yday[leapyear(year)][mon]) - 1;
    rata_die_t yday     = mon_yday + mday;
    rata_die_t t0       = ydhms_diff (year, yday, EPOCH_YEAR - TM_YEAR_BASE);
    return t0;
  }

} // namespace glibc

namespace hinnant {

  // https://github.com/llvm/llvm-project/blob/8e34be2f2511dfff7a8e3018bbd4188a93e446ea/libcxx/include/chrono#L2325
  rata_die_t constexpr
  to_rata_die(date_t date) noexcept {
    const int      __yr  = static_cast<int>(date.year) - (date.month <= 2);
    const unsigned __mth = static_cast<unsigned>(date.month);
    const unsigned __dy  = static_cast<unsigned>(date.day);
    const int      __era = (__yr >= 0 ? __yr : __yr - 399) / 400;
    const unsigned __yoe = static_cast<unsigned>(__yr - __era * 400);                // [0, 399]
    const unsigned __doy = (153 * (__mth + (__mth > 2 ? -3 : 9)) + 2) / 5 + __dy-1;  // [0, 365]
    const unsigned __doe = __yoe * 365 + __yoe/4 - __yoe/100 + __doy;                // [0, 146096]
    return rata_die_t{__era * 146097 + static_cast<int>(__doe) - 719468};
  }

} // namespace hinnant

//-------------------------------------------------------------------
// Benchmark data
//-------------------------------------------------------------------

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
  return { year_t(y - 1467600), month_t(m), day_t(d) }; // adjsted to unsigned unix epoch
}

auto const dates = [](){
  std::uniform_int_distribution<rata_die_t> uniform_dist(-146097, 146096);
  std::mt19937 rng;
  std::array<date_t, 65536> dates;
  for (auto& date : dates)
    date = to_date(uniform_dist(rng));
  return dates;
}();

//-------------------------------------------------------------------
// Benchmark
//-------------------------------------------------------------------

void GLIBC(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& date : dates) {
      auto day = glibc::to_rata_die(date);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(GLIBC);

void DotNet(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& date : dates) {
      auto day = dotnet::to_rata_die(date);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(DotNet);

void Boost(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& date : dates) {
      auto day = boost::to_rata_die(date);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(Boost);

void Hinnant(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& date : dates) {
      auto day = hinnant::to_rata_die(date);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(Hinnant);

void Baum(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& date : dates) {
      auto day = baum::to_rata_die(date);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(Baum);

void Neri(benchmark::State& state) {
  for (auto _ : state) {
    for (auto const& date : dates) {
      auto day = neri::to_rata_die(date);
      benchmark::DoNotOptimize(day);
    }
  }
}
BENCHMARK(Neri);
