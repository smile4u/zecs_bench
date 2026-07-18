#include <cstddef>
#include <cstdint>
#include <cstdio>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include "perf_ecs.h"

namespace
{
// picobench принимает только свободные функции (без захвата), поэтому
// параметры прогона передаются через user_data бенчмарка.
struct BenchConfig
{
    size_t entities_count;
    size_t max_comps;
};

template<typename WorldT>
void bench_views(picobench::state& s)
{
    const auto& config = *reinterpret_cast<const BenchConfig*>(s.benchmark_user_data());

    // Создание и заполнение мира вне замера: таймер picobench
    // работает только внутри цикла for по state.
    WorldT test_world;
    test_world.fill(config.entities_count, config.max_comps);

    for (auto _ : s)
    {
        test_world.run_views();
    }
}

void print_table_header()
{
    std::printf("\n");
    std::printf("| config                   |     entt ns/op |     zecs ns/op | relative\n");
    std::printf("|--------------------------|---------------:|---------------:|--------:\n");
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
    std::printf("| %-24s | %14.0f | %14.0f | %7.1f%%\n",
        suite.name, entt_ns, zecs_ns, zecs_ns / entt_ns * 100.0);
    std::fflush(stdout);
}
} // namespace

int main()
{
    // Аналог экспериментов Celero: View_10K/50K/200K с числом компонентов {10, 25, 50, 100}.
    const size_t entity_counts[] = { 10000, 50000, 200000 };
    const size_t comp_counts[] = { 10, 25, 50, 100 };

    print_table_header();

    for (size_t entities_count : entity_counts)
    {
        for (size_t max_comps : comp_counts)
        {
            char title[64];
            std::snprintf(title, sizeof(title), "Entitie %zuK, Comps max %zu", entities_count / 1000, max_comps);

            // Свежий локальный runner на каждый сьюит: прогоняем конфигурацию
            // и сразу печатаем её строку, не дожидаясь остальных.
            // title и config должны жить до конца run_benchmarks(): на них
            // ссылаются имя сьюита и user_data бенчмарков.
            const BenchConfig config{ entities_count, max_comps };
            const uintptr_t data = reinterpret_cast<uintptr_t>(&config);

            picobench::runner runner(/*local=*/true);
            runner.set_default_state_iterations({ 20 });
            runner.set_default_samples(5);
            runner.set_suite(title);
            runner.add_benchmark("entt", bench_views<TestWorldEntt>).baseline().user_data(data);
            runner.add_benchmark("zecs", bench_views<TestWorldZecs>).user_data(data);

            runner.run_benchmarks();
            const auto rpt = runner.generate_report();
            if (!rpt.suites.empty())
                print_table_row(rpt.suites.front());

            if (runner.error() != picobench::no_error)
                return runner.error();
        }
    }

    return 0;
}
