#include "perf_ecs.h"

#include <cassert>

#include "perf_traits.h"

void TestWorld::fill(size_t entities_count, size_t coms_count)
{
    for(size_t i = 0; i < entities_count; ++i)
    {
        TestEntityIndex index = entity_add();
        entity_fill(index, i % coms_count);
    }
}

TestEntityIndex TestWorldEntt::entity_add()
{
    auto entity = reg.create();
    entities.push_back(entity);
    return entities.size() - 1;
}

void TestWorldEntt::entity_rem(TestEntityIndex index)
{
    auto entity = entities[index];
    reg.destroy(entity);
    entities[index] = entt::null;
}

TestEntityIndex TestWorldZecs::entity_add()
{
    auto entity = world.entity_add();
    entities.push_back(entity);
    return entities.size() - 1;
}

void TestWorldZecs::entity_rem(TestEntityIndex index)
{
    auto entity = entities[index];
    world.entity_remove(entity);
    entities[index] = zecs::Entity_INVALID;
}

void TestWorldEntt::entity_fill(TestEntityIndex index, size_t coms_count)
{
    assert(coms_count <= TEST_COMPS_MAX);
    dispatch::entt_fill_table[coms_count](reg, entities[index]);
}

TestCompBase* TestWorldEntt::comp_get(TestEntityIndex index, size_t comp_index)
{
    auto entity = entities[index];
    if (entity == entt::null)
        return nullptr;
    assert(comp_index <= TEST_COMPS_MAX);
    return dispatch::entt_get_table[comp_index](reg, entity);
}

void TestWorldZecs::entity_fill(TestEntityIndex index, size_t coms_count)
{
    assert(coms_count <= TEST_COMPS_MAX);
    dispatch::zecs_fill_table[coms_count](world, entities[index]);
}

TestCompBase* TestWorldZecs::comp_get(TestEntityIndex index, size_t comp_index)
{
    auto entity = entities[index];
    if (entity == zecs::Entity_INVALID)
        return nullptr;
    assert(comp_index <= TEST_COMPS_MAX);
    return dispatch::zecs_get_table[comp_index](world, entity);
}

void TestWorldEntt::run_views()
{
    for (entt::entity e : reg.view<TestComp<1>, TestComp<5>>())
    {
        reg.get<TestComp<1>>(e).value += 11;
        reg.patch<TestComp<1>>(e);
    }

    for (entt::entity e : reg.view<TestComp<7>, TestComp<11>, TestComp<12>>())
    {
        reg.get<TestComp<7>>(e).value += 12;
        reg.patch<TestComp<7>>(e);
    }

    for (entt::entity e : reg.view<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>())
    {
        reg.get<TestComp<22>>(e).value += 13;
        reg.patch<TestComp<22>>(e);
    }

    for (entt::entity e : reg.view<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>())
    {
        reg.get<TestComp<3>>(e).value += 14;
        reg.patch<TestComp<3>>(e);
    }

    for (entt::entity e : reg.view<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>())
    {
        reg.get<TestComp<4>>(e).value += 15;
        reg.patch<TestComp<4>>(e);
    }
}

void TestWorldZecs::run_views()
{
    for (zecs::Entity e : world.view_comp_group<TestComp<1>, TestComp<5>>())
    {
        world.comp_get<TestComp<1>>(e)->value += 11;
        world.comp_patch<TestComp<1>>(e);
    }

    for (zecs::Entity e : world.view_comp_group<TestComp<7>, TestComp<11>, TestComp<12>>())
    {
        world.comp_get<TestComp<7>>(e)->value += 12;
        world.comp_patch<TestComp<7>>(e);
    }

    for (zecs::Entity e : world.view_comp_group<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>())
    {
        world.comp_get<TestComp<22>>(e)->value += 13;
        world.comp_patch<TestComp<22>>(e);
    }

    for (zecs::Entity e : world.view_comp_group<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>())
    {
        world.comp_get<TestComp<3>>(e)->value += 14;
        world.comp_patch<TestComp<3>>(e);
    }

    for (zecs::Entity e : world.view_comp_group<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>())
    {
        world.comp_get<TestComp<4>>(e)->value += 15;
        world.comp_patch<TestComp<4>>(e);
    }
}
