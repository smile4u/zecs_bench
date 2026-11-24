#pragma once

#include <picobench/picobench.hpp>

#include "bench/grid.h"

struct Report;

// Every row is measured SAMPLES times, and the median sample is reported.
// It is odd, so the median is one of the samples.
inline constexpr int SAMPLES = 5;

// The benchmark of one library and the data that it gets as user_data.
struct Bench
{
    picobench::benchmark_proc proc;
    const void* data;
};

// Runs the benchmarks of entt and zecs and writes one row of the table.
// One sample makes `iterations` runs. picobench compares the results
// that the benchmarks set. Different results of entt and zecs are an error.
// `bytes` goes to Report::row().
bool measure_row(Report& report, const char* name, const WorldSize& size, int iterations,
    Bench entt, Bench zecs, const WorldBytes* bytes = nullptr);
