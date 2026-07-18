#pragma once

#include <array>
#include <cstddef>
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
    virtual void run_views() = 0;

    virtual ~TestWorld() = default;
};

struct TestWorldEntt : public TestWorld
{
    TestEntityIndex entity_add() override;
    void entity_rem(TestEntityIndex index) override;
    void entity_fill(TestEntityIndex index, size_t coms_count) override;
    TestCompBase* comp_get(TestEntityIndex index, size_t comp_index) override;
    void run_views() override;

    entt::registry reg;
    std::vector<entt::entity> entities;
};

struct TestWorldZecs : public TestWorld
{
    TestEntityIndex entity_add() override;
    void entity_rem(TestEntityIndex index) override;
    void entity_fill(TestEntityIndex index, size_t coms_count) override;
    TestCompBase* comp_get(TestEntityIndex index, size_t comp_index) override;
    void run_views() override;

    zecs::World world;
    std::vector<zecs::Entity> entities;
};
