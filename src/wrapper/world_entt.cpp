#include "wrapper/world_entt.h"

#include <array>
#include <cassert>
#include <utility>

namespace
{
// Adds TestComp<1>..TestComp<I> to the entity.
// comps_fill<I> calls comps_fill<I - 1> and adds one component. So every function
// has one add inside, not I of them. The file compiles faster, and the code is smaller.
template<size_t I>
void comps_fill(entt::registry& reg, entt::entity entity)
{
    if constexpr (I > 0)
    {
        comps_fill<I - 1>(reg, entity);
        reg.emplace<TestComp<I>>(entity);
    }
}

// Adds TestComp<I> if the entity doesn't have it yet.
template<size_t I>
void comp_add(entt::registry& reg, entt::entity entity)
{
    (void)reg.get_or_emplace<TestComp<I>>(entity);
}

// The functions of one component type.
struct CompFuncs
{
    void (*fill)(entt::registry&, entt::entity);
    void (*add)(entt::registry&, entt::entity);
};

// N is known only at run time. Entry [n] works with TestComp<n>, so one call
// through the table does the work of a switch with a case for every type.
constexpr auto COMP_FUNCS = []<size_t... Is>(std::index_sequence<Is...>)
{
    return std::array<CompFuncs, sizeof...(Is)>{ CompFuncs{ &comps_fill<Is>, &comp_add<Is> }... };
}(std::make_index_sequence<TEST_COMPS_MAX + 1>{});

template<typename Comp>
uint64_t comp_value_sum(entt::registry& reg)
{
    uint64_t sum = 0;
    for (entt::entity e : reg.view<Comp>())
        sum += uint64_t(reg.get<Comp>(e).value);
    return sum;
}
} // namespace

entt::entity TestWorldEntt::entity_add()
{
    return entities.emplace_back(reg.create());
}

void TestWorldEntt::entity_fill(entt::entity entity, size_t comp_count)
{
    assert(comp_count <= TEST_COMPS_MAX);
    COMP_FUNCS[comp_count].fill(reg, entity);
}

void TestWorldEntt::comp_add(entt::entity entity, size_t comp_index)
{
    assert(comp_index >= 1 && comp_index <= TEST_COMPS_MAX);
    COMP_FUNCS[comp_index].add(reg, entity);
}

uint64_t TestWorldEntt::mutation_sum()
{
    return comp_value_sum<TestComp<1>>(reg) + comp_value_sum<TestComp<2>>(reg);
}
