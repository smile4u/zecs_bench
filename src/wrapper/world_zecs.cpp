#include "wrapper/world_zecs.h"

#include <array>
#include <cassert>
#include <utility>

namespace
{
// The same tables as in world_entt.cpp. The comments are there.
template<size_t I>
void comps_fill(zecs::World& world, zecs::Entity entity)
{
    if constexpr (I > 0)
    {
        comps_fill<I - 1>(world, entity);
        world.comp_add<TestComp<I>>(entity);
    }
}

template<size_t I>
void comp_add(zecs::World& world, zecs::Entity entity)
{
    world.comp_get_or_add<TestComp<I>>(entity);
}

struct CompFuncs
{
    void (*fill)(zecs::World&, zecs::Entity);
    void (*add)(zecs::World&, zecs::Entity);
};

constexpr auto COMP_FUNCS = []<size_t... Is>(std::index_sequence<Is...>)
{
    return std::array<CompFuncs, sizeof...(Is)>{ CompFuncs{ &comps_fill<Is>, &comp_add<Is> }... };
}(std::make_index_sequence<TEST_COMPS_MAX + 1>{});

template<typename Comp>
uint64_t comp_value_sum(zecs::World& world)
{
    uint64_t sum = 0;
    for (zecs::Entity e : world.view_comp_group<Comp>())
        sum += uint64_t(world.comp_get<Comp>(e)->value);
    return sum;
}
} // namespace

zecs::Entity TestWorldZecs::entity_add()
{
    return entities.emplace_back(world.entity_add());
}

void TestWorldZecs::entity_fill(zecs::Entity entity, size_t comp_count)
{
    assert(comp_count <= TEST_COMPS_MAX);
    COMP_FUNCS[comp_count].fill(world, entity);
}

void TestWorldZecs::comp_add(zecs::Entity entity, size_t comp_index)
{
    assert(comp_index >= 1 && comp_index <= TEST_COMPS_MAX);
    COMP_FUNCS[comp_index].add(world, entity);
}

uint64_t TestWorldZecs::mutation_sum()
{
    return comp_value_sum<TestComp<1>>(world) + comp_value_sum<TestComp<2>>(world) + comp_value_sum<TestComp<8>>(world);
}
