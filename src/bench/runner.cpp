#include "bench/runner.h"

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include "bench/report.h"

bool measure_row(const char* name, int iterations, Bench entt, Bench zecs, Row& row)
{
    picobench::runner runner(/*local=*/true);
    runner.set_default_state_iterations({ iterations });
    runner.set_default_samples(SAMPLES);
    runner.set_compare_results_across_benchmarks(true);
    runner.set_suite(name);
    runner.add_benchmark("entt", entt.proc).user_data(reinterpret_cast<uintptr_t>(entt.data));
    runner.add_benchmark("zecs", zecs.proc).user_data(reinterpret_cast<uintptr_t>(zecs.data));
    runner.run_benchmarks();

    // One suite with one input. The median comes from picobench.patch.
    const auto suite = runner.generate_report().suites.front();
    const auto time = [&](const char* library)
    {
        const auto& data = suite.find_benchmark(library)->data.front();
        return RowTime{ double(data.median_time_ns) / iterations, data.median_error };
    };
    row.entt = time("entt");
    row.zecs = time("zecs");

    return runner.error() == picobench::no_error;
}
