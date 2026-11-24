#include "scenario.h"

#include <cstdint>
#include <format>
#include <iostream>

#include "bench/memory.h"
#include "bench/report.h"
#include "bench/runner.h"
#include "wrapper/fill.h"

namespace
{
// The number of runs of run() in one sample.
constexpr int ITERATIONS = 20;

// picobench takes only free functions, so the scenario and the world
// come to bench_run() through user_data.
template<typename WrapperT>
struct Job
{
    WorldScenario& scenario;
    WrapperT& wrapper;
};

template<typename WrapperT>
void bench_run(picobench::state& s)
{
    const auto& job = *reinterpret_cast<const Job<WrapperT>*>(s.benchmark_user_data());

    // The sums are taken outside the timer. entt and zecs must change the data
    // by the same amount. picobench compares the two results.
    const uint64_t before = job.wrapper.mutation_sum();
    for (picobench::iterate _ : s)
        job.scenario.run(job.wrapper);

    s.set_result(job.wrapper.mutation_sum() - before);
}
} // namespace

// The worlds of the previous size are destroyed before the new worlds
// are made, so only one pair of worlds is alive at a time.
bool WorldScenario::measure(Report& report, SizeBytes& bytes)
{
    report.table_begin(name, false);

    for (size_t s = 0; s < std::size(WORLD_SIZES); ++s)
    {
        const WorldSize& size = WORLD_SIZES[s];

        const uint64_t start = allocated_bytes();
        TestWorldEntt entt_wrapper;
        fill(entt_wrapper, size.entity_count, size.comp_count);
        const uint64_t middle = allocated_bytes();
        TestWorldZecs zecs_wrapper;
        fill(zecs_wrapper, size.entity_count, size.comp_count);
        const uint64_t end = allocated_bytes();

        if (WorldBytes{ middle - start, end - middle } != bytes[s])
        {
            std::cerr << std::format("Error: {}, {}: a world took a different amount of memory than in Create\n",
                name, size_text(size));
            return false;
        }

        prepare(entt_wrapper);
        prepare(zecs_wrapper);

        const Job<TestWorldEntt> entt_job{ *this, entt_wrapper };
        const Job<TestWorldZecs> zecs_job{ *this, zecs_wrapper };
        if (!measure_row(report, name, size, ITERATIONS,
                { bench_run<TestWorldEntt>, &entt_job }, { bench_run<TestWorldZecs>, &zecs_job }))
            return false;
    }

    report.table_end(note);
    return true;
}
