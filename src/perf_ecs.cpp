#include "perf_ecs.h"

#include <cassert>

#include "perf_traits.h"

namespace
{
template<typename Comp>
uint64_t comp_value_sum(entt::registry& reg)
{
    uint64_t sum = 0;
    for (entt::entity e : reg.view<Comp>())
        sum += uint64_t(reg.get<Comp>(e).value);
    return sum;
}

template<typename Comp>
uint64_t comp_value_sum(zecs::World& world)
{
    uint64_t sum = 0;
    for (zecs::Entity e : world.view_comp_group<Comp>())
        sum += uint64_t(world.comp_get<Comp>(e)->value);
    return sum;
}

// The body of run_each(). It reads the second component, so the body
// uses more than one of the components it is given.
template<typename First, typename Second, typename... Rest>
inline void each_body(int add, First& first, const Second& second, const Rest&...)
{
    first.value += add + second.value;
}

template<typename... Comp>
void each_run(entt::registry& reg, int add)
{
    reg.view<Comp...>().each([add](Comp&... comps) { each_body(add, comps...); });
}

template<typename... Comp>
void each_run(zecs::World& world, int add)
{
    const auto scope = world.query_scope<Comp...>();
    world.query_for_each(scope, [add](zecs::Entity, Comp&... comps) { each_body(add, comps...); });
}

// Наборы компонентов, матчующие пять view из run_views(); 0 — терминатор.
constexpr size_t VIEW_MATCH_COMPS[][9] = {
    { 1, 5, 0 },
    { 7, 11, 12, 0 },
    { 22, 33, 44, 45, 0 },
    { 3, 17, 23, 33, 43, 0 },
    { 4, 15, 24, 25, 34, 35, 44, 45, 0 },
};
} // namespace

void TestWorld::fill(size_t entities_count, size_t coms_count)
{
    for(size_t i = 0; i < entities_count; ++i)
    {
        TestEntityIndex index = entity_add();
        entity_fill(index, i % coms_count);

        // Каждая пятая энтити дополнительно получает набор компонентов одного
        // из view, иначе при coms_count < 45 большая часть view пуста и замер
        // превращается в стоимость построения пустого view.
        if (i % 5 == 0)
        {
            const size_t* view_comps = VIEW_MATCH_COMPS[(i / 5) % 5];
            for (size_t k = 0; view_comps[k] != 0; ++k)
                comp_add(index, view_comps[k]);
        }
    }
}

TestWorldEntt::TestWorldEntt()
{
    patched.bind(reg);
    patched.on_update<TestComp<8>>();
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
    dispatch::table[coms_count].entt_fill(reg, entities[index]);
}

TestCompBase* TestWorldEntt::comp_get(TestEntityIndex index, size_t comp_index)
{
    auto entity = entities[index];
    if (entity == entt::null)
        return nullptr;
    assert(comp_index <= TEST_COMPS_MAX);
    return dispatch::table[comp_index].entt_get(reg, entity);
}

void TestWorldEntt::comp_add(TestEntityIndex index, size_t comp_index)
{
    assert(comp_index >= 1 && comp_index <= TEST_COMPS_MAX);
    dispatch::table[comp_index].entt_add(reg, entities[index]);
}

void TestWorldZecs::entity_fill(TestEntityIndex index, size_t coms_count)
{
    assert(coms_count <= TEST_COMPS_MAX);
    dispatch::table[coms_count].zecs_fill(world, entities[index]);
}

TestCompBase* TestWorldZecs::comp_get(TestEntityIndex index, size_t comp_index)
{
    auto entity = entities[index];
    if (entity == zecs::Entity_INVALID)
        return nullptr;
    assert(comp_index <= TEST_COMPS_MAX);
    return dispatch::table[comp_index].zecs_get(world, entity);
}

void TestWorldZecs::comp_add(TestEntityIndex index, size_t comp_index)
{
    assert(comp_index >= 1 && comp_index <= TEST_COMPS_MAX);
    dispatch::table[comp_index].zecs_add(world, entities[index]);
}

void TestWorldEntt::run_views()
{
    for (entt::entity e : reg.view<TestComp<1>, TestComp<5>>())
    {
        reg.get<TestComp<1>>(e).value += 11;
        //reg.patch<TestComp<1>>(e);
    }

    for (entt::entity e : reg.view<TestComp<7>, TestComp<11>, TestComp<12>>())
    {
        reg.get<TestComp<7>>(e).value += 12;
        //reg.patch<TestComp<7>>(e);
    }

    for (entt::entity e : reg.view<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>())
    {
        reg.get<TestComp<22>>(e).value += 13;
        //reg.patch<TestComp<22>>(e);
    }

    for (entt::entity e : reg.view<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>())
    {
        reg.get<TestComp<3>>(e).value += 14;
        //reg.patch<TestComp<3>>(e);
    }

    for (entt::entity e : reg.view<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>())
    {
        reg.get<TestComp<4>>(e).value += 15;
        reg.patch<TestComp<4>>(e);
    }
}

void TestWorldZecs::run_views()
{
    // world.comp_get<T>(e) re-resolves the component pool from World
    // (cuidIndex + comp_pools lookup) on every call; the view already holds
    // the pool's lookup arrays, so view.comp_get<T>(e) skips that.
    // Views are read-only; patching goes through world.comp_patch<T>(e).
    {
        auto view = world.view_comp_group<TestComp<1>, TestComp<5>>();
        for (zecs::Entity e : view)
        {
            world.comp_get<TestComp<1>>(e)->value += 11;
            //world.comp_patch<TestComp<1>>(e);
        }
    }

    {
        auto view = world.view_comp_group<TestComp<7>, TestComp<11>, TestComp<12>>();
        for (zecs::Entity e : view)
        {
            world.comp_get<TestComp<7>>(e)->value += 12;
            //world.comp_patch<TestComp<7>>(e);
        }
    }

    {
        auto view = world.view_comp_group<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>();
        for (zecs::Entity e : view)
        {
            world.comp_get<TestComp<22>>(e)->value += 13;
            //world.comp_patch<TestComp<22>>(e);
        }
    }

    {
        auto view = world.view_comp_group<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>();
        for (zecs::Entity e : view)
        {
            world.comp_get<TestComp<3>>(e)->value += 14;
            //world.comp_patch<TestComp<3>>(e);
        }
    }

    {
        auto view = world.view_comp_group<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>();
        for (zecs::Entity e : view)
        {
            world.comp_get<TestComp<4>>(e)->value += 15;
            //world.comp_patch<TestComp<4>>(e);
        }
    }
}

void TestWorldEntt::run_each()
{
    each_run<TestComp<1>, TestComp<5>>(reg, 41);
    each_run<TestComp<7>, TestComp<11>, TestComp<12>>(reg, 42);
    each_run<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>(reg, 43);
    each_run<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>(reg, 44);
    each_run<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>(reg, 45);
}

void TestWorldZecs::run_each()
{
    each_run<TestComp<1>, TestComp<5>>(world, 41);
    each_run<TestComp<7>, TestComp<11>, TestComp<12>>(world, 42);
    each_run<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>(world, 43);
    each_run<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>(world, 44);
    each_run<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>(world, 45);
}

void TestWorldEntt::run_comps()
{
    for (entt::entity e : reg.view<TestComp<2>>())
        reg.get<TestComp<2>>(e).value += 21;

    for (entt::entity e : reg.view<TestComp<6>>())
        reg.get<TestComp<6>>(e).value += 22;

    for (entt::entity e : reg.view<TestComp<9>>())
        reg.get<TestComp<9>>(e).value += 23;
}

void TestWorldZecs::run_comps()
{
    for (zecs::Entity e : world.view_comps<TestComp<2>>())
        world.comp_get<TestComp<2>>(e)->value += 21;

    for (zecs::Entity e : world.view_comps<TestComp<6>>())
        world.comp_get<TestComp<6>>(e)->value += 22;

    for (zecs::Entity e : world.view_comps<TestComp<9>>())
        world.comp_get<TestComp<9>>(e)->value += 23;
}

void TestWorldEntt::run_patched()
{
    for (size_t i = 0; i < entities.size(); i += 4)
    {
        if (reg.all_of<TestComp<8>>(entities[i]))
            reg.patch<TestComp<8>>(entities[i]);
    }

    for (entt::entity e : patched)
        reg.get<TestComp<8>>(e).value += 31;

    patched.clear();
}

void TestWorldZecs::run_patched()
{
    // Всё, что пропатчено после этого тика, попадёт в view ниже.
    const zecs::TickContext ctx(world.tick_get());

    // comp_patch сам ничего не делает, если компонента у энтити нет.
    for (size_t i = 0; i < entities.size(); i += 4)
        world.comp_patch<TestComp<8>>(entities[i]);

    for (zecs::Entity e : world.view_patched_comps<TestComp<8>>(ctx))
        world.comp_get<TestComp<8>>(e)->value += 31;
}

void TestWorldEntt::run_remove()
{
    for (entt::entity e : entities)
        reg.destroy(e);

    entities.clear();
}

void TestWorldZecs::run_remove()
{
    for (zecs::Entity e : entities)
        world.entity_remove(e);

    entities.clear();
}

uint64_t TestWorldEntt::mutation_sum()
{
    return comp_value_sum<TestComp<1>>(reg)
        + comp_value_sum<TestComp<7>>(reg)
        + comp_value_sum<TestComp<22>>(reg)
        + comp_value_sum<TestComp<3>>(reg)
        + comp_value_sum<TestComp<4>>(reg)
        + comp_value_sum<TestComp<2>>(reg)
        + comp_value_sum<TestComp<6>>(reg)
        + comp_value_sum<TestComp<9>>(reg)
        + comp_value_sum<TestComp<8>>(reg);
}

uint64_t TestWorldZecs::mutation_sum()
{
    return comp_value_sum<TestComp<1>>(world)
        + comp_value_sum<TestComp<7>>(world)
        + comp_value_sum<TestComp<22>>(world)
        + comp_value_sum<TestComp<3>>(world)
        + comp_value_sum<TestComp<4>>(world)
        + comp_value_sum<TestComp<2>>(world)
        + comp_value_sum<TestComp<6>>(world)
        + comp_value_sum<TestComp<9>>(world)
        + comp_value_sum<TestComp<8>>(world);
}
