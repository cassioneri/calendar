.PHONY: all clean

ALL      = is_leap_year last_day_of_month to_date to_rata_die to_time itoa

CXXFLAGS = -O3 -std=c++2a
LDLIBS   = -l benchmark -l benchmark_main

CSVs     = $(addsuffix .csv, $(ALL))

all : $(CSVs)

$(CSVs) : %.csv : %
	./$? --benchmark_out_format=csv --benchmark_out=$@

clean :
	rm -rf $(ALL) $(CSVs)
