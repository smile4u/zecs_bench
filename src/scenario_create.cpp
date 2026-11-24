#include "scenario.h"

#include <cstdint>

#include "bench/memory.h"
#include "bench/report.h"
#include "bench/runner.h"
#include "wrapper/fill.h"

namespace
{
struct Job
{
    const WorldSize& size;
    // The memory of the filled world. It is the same in every sample.
    // Every WorldScenario checks it again when it makes a world of this size.
    uint64_t& bytes;
};

template<typename WrapperT>
void bench_create(picobench::state& s)
{
    const auto& job = *reinterpret_cast<const Job*>(s.benchmark_user_data());

    const uint64_t start = allocated_bytes();
    s.start_timer();
    WrapperT wrapper;
    fill(wrapper, job.size.entity_count, job.size.comp_count);
    s.stop_timer();
    job.bytes = allocated_bytes() - start;

    // The sum is the same for entt and zecs only when both worlds got
    // the same components. picobench compares the two results.
    s.set_result(wrapper.mutation_sum());
}
} // namespace

bool ScenarioCreate::measure(Report& report, SizeBytes& bytes)
{
    report.table_begin(name.c_str(), about, false, false, true);

    for (size_t s = 0; s < std::size(WORLD_SIZES); ++s)
    {
        const Job entt_job{ WORLD_SIZES[s], bytes[s].entt };
        const Job zecs_job{ WORLD_SIZES[s], bytes[s].zecs };
        Row row{ WORLD_SIZES[s] };
        row.bytes = &bytes[s];
        if (!measure_row(name.c_str(), 1, { bench_create<TestWorldEntt>, &entt_job }, { bench_create<TestWorldZecs>, &zecs_job }, row))
            return false;

        report.row(row);
    }

    return true;
}
