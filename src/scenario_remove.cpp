#include "scenario.h"

#include "bench/report.h"
#include "bench/runner.h"
#include "wrapper/fill.h"

namespace
{
void remove_all(TestWorldEntt& wrapper)
{
    for (entt::entity e : wrapper.entities)
        wrapper.reg.destroy(e);
}

void remove_all(TestWorldZecs& wrapper)
{
    for (zecs::Entity e : wrapper.entities)
        wrapper.world.entity_remove(e);
}

template<typename WrapperT>
void bench_remove(picobench::state& s)
{
    const auto& size = *reinterpret_cast<const WorldSize*>(s.benchmark_user_data());

    WrapperT wrapper;
    fill(wrapper, size.entity_count, size.comp_count);

    s.start_timer();
    remove_all(wrapper);
    s.stop_timer();
}
} // namespace

bool ScenarioRemove::measure(Report& report, SizeBytes&)
{
    report.table_begin(name, about, false, false);

    for (const WorldSize& size : WORLD_SIZES)
    {
        Row row{ size };
        if (!measure_row(name, 1, { bench_remove<TestWorldEntt>, &size }, { bench_remove<TestWorldZecs>, &size }, row))
            return false;

        report.row(row);
    }

    return true;
}
