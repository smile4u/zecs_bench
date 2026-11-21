#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include "perf_ecs.h"
#include "utils.h"

namespace
{
// picobench takes only free functions, so the world comes through user_data.
// Run is the scenario of TestWorld to measure: run_views, run_comps, ...
template<typename WorldT, void (TestWorld::*Run)()>
void bench_run(picobench::state& s)
{
    auto& world = *reinterpret_cast<WorldT*>(s.benchmark_user_data());

    // The sums are taken outside the timer. entt and zecs must change the data
    // by the same amount: picobench compares the two results (see run_suite).
    const uint64_t before = world.mutation_sum();
    for (auto _ : s)
        (world.*Run)();

    s.set_result(world.mutation_sum() - before);
}

struct Scenario
{
    const char* name;
    picobench::benchmark_proc entt;
    picobench::benchmark_proc zecs;
};

// Runs entt and zecs on their worlds and prints one row of the table.
// Every row has its own runner, so the row is printed at once.
picobench::error_t run_suite(const std::string& title, const Scenario& scenario,
    TestWorld& entt_world, TestWorld& zecs_world, double entt_mb, double zecs_mb)
{
    picobench::runner runner(/*local=*/true);
    runner.set_default_state_iterations({ 20 });
    runner.set_default_samples(5);
    // Different results of entt and zecs are an error.
    runner.set_compare_results_across_benchmarks(true);
    runner.set_suite(title.c_str());
    runner.add_benchmark("entt", scenario.entt)
        .baseline()
        .user_data(reinterpret_cast<uintptr_t>(&entt_world));
    runner.add_benchmark("zecs", scenario.zecs)
        .user_data(reinterpret_cast<uintptr_t>(&zecs_world));

    runner.run_benchmarks();

    const auto report = runner.generate_report();
    const auto* entt = report.suites.empty() ? nullptr : report.suites.front().find_benchmark("entt");
    const auto* zecs = report.suites.empty() ? nullptr : report.suites.front().find_benchmark("zecs");
    if (entt && zecs && !entt->data.empty() && !zecs->data.empty())
    {
        // A benchmark has one input. total_time_ns is the time of its best sample.
        const double entt_ns = double(entt->data[0].total_time_ns) / entt->data[0].dimension;
        const double zecs_ns = double(zecs->data[0].total_time_ns) / zecs->data[0].dimension;
        std::cout << std::format("| {:<38} | {:>14.0f} | {:>14.0f} | {:>7.1f}% | {:>7.1f} | {:>7.1f}\n",
            title, entt_ns, zecs_ns, zecs_ns / entt_ns * 100.0, entt_mb, zecs_mb) << std::flush;
    }

    return runner.error();
}
} // namespace

int main()
{
    // The same sizes as in the old Celero bench.
    const size_t entity_counts[] = { 10000, 50000, 200000 };
    const size_t comp_counts[] = { 10, 25, 50, 100 };

    const Scenario scenarios[] = {
        { "View", bench_run<TestWorldEntt, &TestWorld::run_views>, bench_run<TestWorldZecs, &TestWorld::run_views> },
        { "ForEach", bench_run<TestWorldEntt, &TestWorld::run_each>, bench_run<TestWorldZecs, &TestWorld::run_each> },
        { "Comps", bench_run<TestWorldEntt, &TestWorld::run_comps>, bench_run<TestWorldZecs, &TestWorld::run_comps> },
        { "Patched", bench_run<TestWorldEntt, &TestWorld::run_patched>, bench_run<TestWorldZecs, &TestWorld::run_patched> },
    };

    std::cout << "\n"
              << "| config                                 |     entt ns/op |     zecs ns/op | relative | entt MB | zecs MB\n"
              << "|----------------------------------------|----------------|----------------|----------|---------|--------\n"
              << std::flush;

    for (size_t entity_count : entity_counts)
    {
        for (size_t comp_count : comp_counts)
        {
            // The worlds are made once for a size and used by all scenarios:
            // run_*() change only values, not the set of components.
            // The memory of a world is what operator new gave while
            // the world was made and filled (see utils.h).
            const uint64_t start = allocated_bytes();
            TestWorldEntt entt_world;
            entt_world.fill(entity_count, comp_count);
            const uint64_t middle = allocated_bytes();
            TestWorldZecs zecs_world;
            zecs_world.fill(entity_count, comp_count);
            const uint64_t end = allocated_bytes();

            constexpr double MB = 1024.0 * 1024.0;

            for (const Scenario& scenario : scenarios)
            {
                // The size stands first, so the rows of one size start the same way.
                const std::string title = std::format("entities {}K, comps {} less, {}", entity_count / 1000, comp_count, scenario.name);

                if (picobench::error_t err = run_suite(title, scenario, entt_world, zecs_world,
                        double(middle - start) / MB, double(end - middle) / MB))
                    return err;
            }
        }
    }

    std::cout << "\nPatched: entt has no patched view, its reactive storage is measured instead.\n"
              << "zecs keeps the patch list inside the pool; entt fills a separate storage from a signal.\n";

    return 0;
}
