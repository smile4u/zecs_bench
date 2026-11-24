#pragma once

#include <picobench/picobench.hpp>

struct Row;

// Every row is measured SAMPLES times, and the median sample is reported.
// It is odd, so the median is one of the samples.
inline constexpr int SAMPLES = 5;

// The benchmark of one library and the data that it gets as user_data.
struct Bench
{
    picobench::benchmark_proc proc;
    const void* data;
};

// Runs the benchmarks of entt and zecs and writes their times to `row`.
// One sample makes `iterations` runs. picobench compares the results
// that the benchmarks set. Different results of entt and zecs are an error.
bool measure_row(const char* name, int iterations, Bench entt, Bench zecs, Row& row);
