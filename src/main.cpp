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

// The size of the world that bench_remove makes.
struct WorldSize
{
    size_t entity_count;
    size_t comp_count;
};

// After the removal a world is empty, so this benchmark can not use the
// worlds of the other scenarios. Every sample makes and fills its own world.
// Only the removal is timed.
template<typename WorldT>
void bench_remove(picobench::state& s)
{
    const auto& size = *reinterpret_cast<const WorldSize*>(s.benchmark_user_data());

    WorldT world;
    world.fill(size.entity_count, size.comp_count);

    s.start_timer();
    world.run_remove();
    s.stop_timer();

    // No component is left, so the sum is 0 for both.
    s.set_result(world.mutation_sum());
}

struct Scenario
{
    const char* name;
    picobench::benchmark_proc entt;
    picobench::benchmark_proc zecs;
};

// Runs entt and zecs and prints one row of the table. `entt_data` and
// `zecs_data` go to the benchmarks as user_data; `iterations` is the number
// of operations in one sample.
// Every row has its own runner, so the row is printed at once.
picobench::error_t run_suite(const std::string& title, const Scenario& scenario,
    uintptr_t entt_data, uintptr_t zecs_data, int iterations, double entt_mb, double zecs_mb)
{
    picobench::runner runner(/*local=*/true);
    runner.set_default_state_iterations({ iterations });
    runner.set_default_samples(5);
    // Different results of entt and zecs are an error.
    runner.set_compare_results_across_benchmarks(true);
    runner.set_suite(title.c_str());
    runner.add_benchmark("entt", scenario.entt)
        .baseline()
        .user_data(entt_data);
    runner.add_benchmark("zecs", scenario.zecs)
        .user_data(zecs_data);

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

#ifndef NDEBUG
    std::cout << "WARNING: NDEBUG isn't defined, so asserts are on. Build Release or RelWithDebInfo to get correct numbers.\n";
#endif

    std::cout << "\n"
              << "| config                                 |     entt ns/op |     zecs ns/op | relative | entt MB | zecs MB\n"
              << "|----------------------------------------|----------------|----------------|----------|---------|--------\n"
              << std::flush;

    for (size_t entity_count : entity_counts)
    {
        for (size_t comp_count : comp_counts)
        {
            // The size stands first, so the rows of one size start the same way.
            const auto title = [&](const char* name)
            {
                return std::format("entities {}K, comps {} less, {}", entity_count / 1000, comp_count, name);
            };

            double entt_mb = 0.0;
            double zecs_mb = 0.0;

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
                entt_mb = double(middle - start) / MB;
                zecs_mb = double(end - middle) / MB;

                for (const Scenario& scenario : scenarios)
                {
                    if (picobench::error_t err = run_suite(title(scenario.name), scenario,
                            reinterpret_cast<uintptr_t>(&entt_world), reinterpret_cast<uintptr_t>(&zecs_world),
                            20, entt_mb, zecs_mb))
                        return err;
                }
            }

            // Remove runs after the shared worlds are destroyed. It makes its
            // own worlds, and three big worlds at once take too much memory.
            // One operation is the removal of all entities of a world.
            const WorldSize size{ entity_count, comp_count };
            const Scenario remove{ "Remove", bench_remove<TestWorldEntt>, bench_remove<TestWorldZecs> };

            if (picobench::error_t err = run_suite(title(remove.name), remove,
                    reinterpret_cast<uintptr_t>(&size), reinterpret_cast<uintptr_t>(&size),
                    1, entt_mb, zecs_mb))
                return err;
        }
    }

    std::cout << "\nPatched: entt has no patched view, its reactive storage is measured instead.\n"
              << "zecs keeps the patch list inside the pool; entt fills a separate storage from a signal.\n";

    return 0;
}
