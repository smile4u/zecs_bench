#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <entt/entt.hpp>

#include "wrapper/test_comps.h"

// entt with the functions that fill() and the bench use.
// TestWorldZecs has the same functions.
struct TestWorldEntt
{
    // Makes an entity and adds it to `entities`.
    entt::entity entity_add();
    // Adds TestComp<1>..TestComp<comp_count> to the entity.
    void entity_fill(entt::entity entity, size_t comp_count);
    // Adds TestComp<comp_index> to the entity if the entity doesn't have it yet.
    void comp_add(entt::entity entity, size_t comp_index);
    // The sum of value over the components that the scenarios change.
    // They are the first components of the queries (see queries.h) and
    // TestComp<8> of ScenarioPatched. entt and zecs must give the same sum.
    // It shows that both worlds do the same work.
    uint64_t mutation_sum();

    entt::registry reg;
    // All entities in the order in which they were made.
    std::vector<entt::entity> entities;
};
