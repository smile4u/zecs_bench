#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <entt/entt.hpp>
#include <zecs/world.h>

struct TestCompBase
{
    int value;
    std::string s;
    std::array<size_t, 16> arr;
};

template<size_t N>
struct TestComp : TestCompBase
{
    inline TestComp() { value = N; };
};

// Верхняя граница индексов компонентов — аналог максимального case
// в ранее генерируемых switch-case (0..100).
inline constexpr size_t TEST_COMPS_MAX = 100;

using TestEntityIndex = size_t;

struct TestWorld
{
    void fill(size_t entities_count, size_t coms_count);

    virtual TestEntityIndex entity_add() = 0;
    virtual void entity_rem(TestEntityIndex index) = 0;
    // Добавляет энтити компоненты TestComp<1>..TestComp<coms_count>.
    virtual void entity_fill(TestEntityIndex index, size_t coms_count) = 0;
    virtual TestCompBase* comp_get(TestEntityIndex index, size_t comp_index) = 0;
    // Добавляет энтити один компонент TestComp<comp_index>, если его ещё нет.
    virtual void comp_add(TestEntityIndex index, size_t comp_index) = 0;
    virtual void run_views() = 0;
    // The same groups as in run_views(), but the library calls the body
    // and gives it the components: entt view.each() against zecs query_for_each().
    virtual void run_each() = 0;
    // Обход одного компонента: entt view<T> против zecs view_comps<T>.
    virtual void run_comps() = 0;
    // Патчит компонент у каждой четвёртой энтити и обходит только пропатченные.
    // Сравнение не один в один: в entt нет встроенного обхода пропатченных
    // компонентов, поэтому там используется reactive storage — отдельное хранилище,
    // которое наполняется из сигнала on_update и очищается вручную. В zecs список
    // пропатченных живёт внутри пула (view_patched_comps<T>).
    virtual void run_patched() = 0;
    // Removes all entities one by one: entt destroy() against zecs entity_remove().
    // The world is empty after it, so this scenario gets its own world (see main.cpp).
    virtual void run_remove() = 0;
    // Сумма value по компонентам, мутируемым в run_*() — контрольная
    // сумма для сверки того, что оба мира делают одинаковую работу.
    virtual uint64_t mutation_sum() = 0;

    virtual ~TestWorld() = default;
};

struct TestWorldEntt : public TestWorld
{
    TestWorldEntt();

    TestEntityIndex entity_add() override;
    void entity_rem(TestEntityIndex index) override;
    void entity_fill(TestEntityIndex index, size_t coms_count) override;
    TestCompBase* comp_get(TestEntityIndex index, size_t comp_index) override;
    void comp_add(TestEntityIndex index, size_t comp_index) override;
    void run_views() override;
    void run_each() override;
    void run_comps() override;
    void run_patched() override;
    void run_remove() override;
    uint64_t mutation_sum() override;

    entt::registry reg;
    // Собирает энтити, у которых пропатчен TestComp<8>.
    entt::reactive_mixin<entt::storage<void>> patched;
    std::vector<entt::entity> entities;
};

struct TestWorldZecs : public TestWorld
{
    TestEntityIndex entity_add() override;
    void entity_rem(TestEntityIndex index) override;
    void entity_fill(TestEntityIndex index, size_t coms_count) override;
    TestCompBase* comp_get(TestEntityIndex index, size_t comp_index) override;
    void comp_add(TestEntityIndex index, size_t comp_index) override;
    void run_views() override;
    void run_each() override;
    void run_comps() override;
    void run_patched() override;
    void run_remove() override;
    uint64_t mutation_sum() override;

    zecs::World world;
    std::vector<zecs::Entity> entities;
};
