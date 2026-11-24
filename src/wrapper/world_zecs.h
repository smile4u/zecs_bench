#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <zecs/world.h>

#include "wrapper/test_comps.h"

// zecs with the same functions as TestWorldEntt (see world_entt.h).
struct TestWorldZecs
{
    zecs::Entity entity_add();
    void entity_fill(zecs::Entity entity, size_t comp_count);
    void comp_add(zecs::Entity entity, size_t comp_index);
    uint64_t mutation_sum();

    zecs::World world;
    std::vector<zecs::Entity> entities;
};
