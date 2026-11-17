#include <cstddef>
#include <cstdint>
#include <cstdio>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include "perf_ecs.h"

namespace
{
// picobench принимает только свободные функции (без захвата), поэтому
// мир передаётся через user_data бенчмарка.
// Run — какой из сценариев TestWorld замеряем (run_views, run_comps, ...).
template<typename WorldT, void (TestWorld::*Run)()>
void bench_run(picobench::state& s)
{
    auto& world = *reinterpret_cast<WorldT*>(s.benchmark_user_data());

    // Контрольные суммы до/после — вне таймера. Дельта за сэмпл одинакова
    // у всех сэмплов и обоих миров, если семантика совпадает, и уходит в
    // set_result для сверки entt vs zecs (см. main).
    const uint64_t before = world.mutation_sum();
    for (auto _ : s)
    {
        (world.*Run)();
    }
    s.set_result(world.mutation_sum() - before);
}

void print_table_header()
{
    std::printf("\n");
    std::printf("| config                       |     entt ns/op |     zecs ns/op | relative\n");
    std::printf("|------------------------------|---------------:|---------------:|--------:\n");
    std::fflush(stdout);
}

// Строка сводной таблицы по одному отработавшему сьюиту.
// У каждого бенчмарка один вход, total_time_ns — время лучшего сэмпла.
void print_table_row(const picobench::report::suite& suite)
{
    const auto* entt = suite.find_benchmark("entt");
    const auto* zecs = suite.find_benchmark("zecs");
    if (!entt || !zecs || entt->data.empty() || zecs->data.empty())
        return;

    const double entt_ns = double(entt->data[0].total_time_ns) / entt->data[0].dimension;
    const double zecs_ns = double(zecs->data[0].total_time_ns) / zecs->data[0].dimension;
    std::printf("| %-28s | %14.0f | %14.0f | %7.1f%%\n",
        suite.name, entt_ns, zecs_ns, zecs_ns / entt_ns * 100.0);
    std::fflush(stdout);
}
} // namespace

int main()
{
    // Аналог экспериментов Celero: View_10K/50K/200K с числом компонентов {10, 25, 50, 100}.
    const size_t entity_counts[] = { 10000, 50000, 200000 };
    const size_t comp_counts[] = { 10, 25, 50, 100 };

    // Сценарии: группы компонентов, один компонент, пропатченные компоненты.
    // Звёздочка у Patched: в entt нет встроенного обхода пропатченных
    // компонентов, вместо него замеряется reactive storage (см. сноску под таблицей).
    struct Scenario
    {
        const char* name;
        picobench::benchmark_proc entt;
        picobench::benchmark_proc zecs;
    };
    const Scenario scenarios[] = {
        { "View", bench_run<TestWorldEntt, &TestWorld::run_views>, bench_run<TestWorldZecs, &TestWorld::run_views> },
        { "Comps", bench_run<TestWorldEntt, &TestWorld::run_comps>, bench_run<TestWorldZecs, &TestWorld::run_comps> },
        { "Patched*", bench_run<TestWorldEntt, &TestWorld::run_patched>, bench_run<TestWorldZecs, &TestWorld::run_patched> },
    };

    print_table_header();

    for (size_t entities_count : entity_counts)
    {
        for (size_t max_comps : comp_counts)
        {
            // Мир заполняется один раз на конфигурацию и переиспользуется всеми
            // сценариями и сэмплами: fill всё равно вне таймера, а run_*() мутируют
            // только значения, не структуру. Миры должны жить до конца
            // run_benchmarks(): на них ссылается user_data бенчмарков.
            TestWorldEntt entt_world;
            TestWorldZecs zecs_world;
            entt_world.fill(entities_count, max_comps);
            zecs_world.fill(entities_count, max_comps);

            for (const Scenario& scenario : scenarios)
            {
                char title[64];
                std::snprintf(title, sizeof(title), "%s_%zuK, max %zu comps", scenario.name, entities_count / 1000, max_comps);

                // Свежий локальный runner на каждый сьюит: прогоняем сценарий
                // и сразу печатаем его строку, не дожидаясь остальных.
                picobench::runner runner(/*local=*/true);
                runner.set_default_state_iterations({ 20 });
                runner.set_default_samples(5);
                // Сверка контрольных сумм entt vs zecs: при расхождении — ошибка.
                runner.set_compare_results_across_benchmarks(true);
                runner.set_suite(title);
                runner.add_benchmark("entt", scenario.entt)
                    .baseline()
                    .user_data(reinterpret_cast<uintptr_t>(&entt_world));
                runner.add_benchmark("zecs", scenario.zecs)
                    .user_data(reinterpret_cast<uintptr_t>(&zecs_world));

                runner.run_benchmarks();
                const auto rpt = runner.generate_report();
                if (!rpt.suites.empty())
                    print_table_row(rpt.suites.front());

                if (runner.error() != picobench::no_error)
                    return runner.error();
            }
        }
    }

    std::printf("\n* entt has no patched view: its reactive storage is measured instead.\n");
    std::printf("  zecs keeps the patch list inside the pool; entt fills a separate storage from a signal.\n");

    return 0;
}
