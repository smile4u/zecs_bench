#include "bench/runner.h"

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include "bench/report.h"

bool measure_row(Report& report, const char* name, const WorldSize& size, int iterations,
    Bench entt, Bench zecs, const WorldBytes* bytes)
{
    picobench::runner runner(/*local=*/true);
    runner.set_default_state_iterations({ iterations });
    runner.set_default_samples(SAMPLES);
    runner.set_compare_results_across_benchmarks(true);
    runner.set_suite(name);
    runner.add_benchmark("entt", entt.proc).user_data(reinterpret_cast<uintptr_t>(entt.data));
    runner.add_benchmark("zecs", zecs.proc).user_data(reinterpret_cast<uintptr_t>(zecs.data));
    runner.run_benchmarks();

    // One suite with one input. total_time_ns is the time of the best sample.
    const auto suite = runner.generate_report().suites.front();
    const double entt_ns = double(suite.find_benchmark("entt")->data.front().total_time_ns) / iterations;
    const double zecs_ns = double(suite.find_benchmark("zecs")->data.front().total_time_ns) / iterations;
    report.row(size, entt_ns, zecs_ns, bytes);

    return runner.error() == picobench::no_error;
}
