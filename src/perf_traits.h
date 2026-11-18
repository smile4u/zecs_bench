#pragma once

// Tables of functions for TestComp<N> when N is known only at run time.
// Entry [n] works with TestComp<n>: one call through the table does
// the work of a switch with a case for every component type.

#include <array>
#include <cstddef>
#include <utility>

#include "perf_ecs.h"

namespace dispatch
{
// Adds TestComp<1>..TestComp<I> to the entity.
// fill<I> calls fill<I - 1> and adds one component. So every function has
// one add inside, not I of them: the file compiles faster and the code is smaller.
template<size_t I>
void entt_fill(entt::registry& reg, entt::entity entity)
{
    if constexpr (I > 0)
    {
        entt_fill<I - 1>(reg, entity);
        reg.emplace<TestComp<I>>(entity);
    }
}

template<size_t I>
void zecs_fill(zecs::World& world, zecs::Entity entity)
{
    if constexpr (I > 0)
    {
        zecs_fill<I - 1>(world, entity);
        world.comp_add<TestComp<I>>(entity);
    }
}

template<size_t I>
TestCompBase* entt_get(entt::registry& reg, entt::entity entity)
{
    return &reg.get<TestComp<I>>(entity);
}

template<size_t I>
TestCompBase* zecs_get(zecs::World& world, zecs::Entity entity)
{
    return world.comp_get<TestComp<I>>(entity);
}

// Adds TestComp<I> if the entity does not have it yet.
template<size_t I>
void entt_add(entt::registry& reg, entt::entity entity)
{
    (void)reg.get_or_emplace<TestComp<I>>(entity);
}

template<size_t I>
void zecs_add(zecs::World& world, zecs::Entity entity)
{
    world.comp_get_or_add<TestComp<I>>(entity);
}

// The functions of one component type.
struct CompFuncs
{
    void (*entt_fill)(entt::registry&, entt::entity);
    TestCompBase* (*entt_get)(entt::registry&, entt::entity);
    void (*entt_add)(entt::registry&, entt::entity);

    void (*zecs_fill)(zecs::World&, zecs::Entity);
    TestCompBase* (*zecs_get)(zecs::World&, zecs::Entity);
    void (*zecs_add)(zecs::World&, zecs::Entity);
};

// table[n] has the functions of TestComp<n>, for n from 0 to TEST_COMPS_MAX.
constexpr auto table = []<size_t... Is>(std::index_sequence<Is...>)
{
    return std::array<CompFuncs, sizeof...(Is)>{ CompFuncs{
        &entt_fill<Is>, &entt_get<Is>, &entt_add<Is>,
        &zecs_fill<Is>, &zecs_get<Is>, &zecs_add<Is> }... };
}(std::make_index_sequence<TEST_COMPS_MAX + 1>{});
} // namespace dispatch
