#include "scenario.h"

#include <cstdint>
#include <format>
#include <iostream>

#include "bench/memory.h"
#include "bench/report.h"
#include "bench/runner.h"
#include "wrapper/fill.h"
#include "wrapper/queries.h"

namespace
{
// The number of runs of run() in one sample.
constexpr int ITERATIONS = 20;

// picobench takes only free functions, so the scenario, the world and
// the query come to bench_run() through user_data.
template<typename WrapperT>
struct Job
{
    WorldScenario& scenario;
    WrapperT& wrapper;
    size_t query;
};

template<typename WrapperT>
void bench_run(picobench::state& s)
{
    const auto& job = *reinterpret_cast<const Job<WrapperT>*>(s.benchmark_user_data());

    // The sums are taken outside the timer. entt and zecs must change the data
    // by the same amount. picobench compares the two results.
    const uint64_t before = job.wrapper.mutation_sum();
    for (picobench::iterate _ : s)
        job.scenario.run(job.wrapper, job.query);

    s.set_result(job.wrapper.mutation_sum() - before);
}
} // namespace

WorldScenario::WorldScenario(const char* name, const char* about, size_t read, Views views)
    : Scenario(std::format("{} {}", name, comp_kind(read)), about)
    , read(read)
    , views(views)
{
}

// The worlds of the previous size are destroyed before the new worlds
// are made, so only one pair of worlds is alive at a time.
bool WorldScenario::measure(Report& report, SizeBytes& bytes)
{
    // A table of one view of one component doesn't need the view column.
    report.table_begin(name.c_str(), about, views != Views::only_read, true, false);

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

        for (size_t q = 0; q < QUERY_COUNT; ++q)
        {
            const QueryInfo& query = QUERY_INFOS[q];
            const bool takes = query.read == read
                && (views == Views::with_read ? query.reads_view()
                    : views == Views::without_read ? !query.reads_view()
                    : query.reads_view() && query.view.size() == 1);
            if (!takes)
                continue;

            Row row{ size };
            row.view = query.view.size();
            row.matched = match_count(entt_wrapper, q);
            if (row.matched != match_count(zecs_wrapper, q))
            {
                std::cerr << std::format("Error: {}, {}: entt and zecs have a different number of entities in the view of query {}\n",
                    name, size_text(size), q);
                return false;
            }

            const Job<TestWorldEntt> entt_job{ *this, entt_wrapper, q };
            const Job<TestWorldZecs> zecs_job{ *this, zecs_wrapper, q };
            if (!measure_row(name.c_str(), ITERATIONS, { bench_run<TestWorldEntt>, &entt_job }, { bench_run<TestWorldZecs>, &zecs_job }, row))
                return false;

            report.row(row);
        }
    }

    return true;
}
