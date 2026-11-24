#include "scenario.h"

// The sets of the views are also in VIEW_MATCH_COMPS (wrapper/fill.cpp).

void ScenarioView::run(TestWorldEntt& wrapper)
{
    entt::registry& reg = wrapper.reg;

    for (entt::entity e : reg.view<TestComp<1>, TestComp<5>>())
        reg.get<TestComp<1>>(e).value += 11;

    for (entt::entity e : reg.view<TestComp<7>, TestComp<11>, TestComp<12>>())
        reg.get<TestComp<7>>(e).value += 12;

    for (entt::entity e : reg.view<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>())
        reg.get<TestComp<22>>(e).value += 13;

    for (entt::entity e : reg.view<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>())
        reg.get<TestComp<3>>(e).value += 14;

    for (entt::entity e : reg.view<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>())
        reg.get<TestComp<4>>(e).value += 15;
}

void ScenarioView::run(TestWorldZecs& wrapper)
{
    zecs::World& world = wrapper.world;

    // The component is taken from the world, as in entt above.
    // zecs views have no comp_get, so world.comp_get<T>(e) finds the pool on every call.
    // TODO(zecs): add view.comp_get<T>(e). Then the loop doesn't need to find the pool.

    for (zecs::Entity e : world.view_comp_group<TestComp<1>, TestComp<5>>())
        world.comp_get<TestComp<1>>(e)->value += 11;

    for (zecs::Entity e : world.view_comp_group<TestComp<7>, TestComp<11>, TestComp<12>>())
        world.comp_get<TestComp<7>>(e)->value += 12;

    for (zecs::Entity e : world.view_comp_group<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>())
        world.comp_get<TestComp<22>>(e)->value += 13;

    for (zecs::Entity e : world.view_comp_group<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>())
        world.comp_get<TestComp<3>>(e)->value += 14;

    for (zecs::Entity e : world.view_comp_group<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>())
        world.comp_get<TestComp<4>>(e)->value += 15;
}

namespace
{
// The body of ScenarioForEach. It reads the second component, so the body
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
} // namespace

void ScenarioForEach::run(TestWorldEntt& wrapper)
{
    entt::registry& reg = wrapper.reg;
    each_run<TestComp<1>, TestComp<5>>(reg, 41);
    each_run<TestComp<7>, TestComp<11>, TestComp<12>>(reg, 42);
    each_run<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>(reg, 43);
    each_run<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>(reg, 44);
    each_run<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>(reg, 45);
}

void ScenarioForEach::run(TestWorldZecs& wrapper)
{
    zecs::World& world = wrapper.world;
    each_run<TestComp<1>, TestComp<5>>(world, 41);
    each_run<TestComp<7>, TestComp<11>, TestComp<12>>(world, 42);
    each_run<TestComp<22>, TestComp<33>, TestComp<44>, TestComp<45>>(world, 43);
    each_run<TestComp<3>, TestComp<17>, TestComp<23>, TestComp<33>, TestComp<43>>(world, 44);
    each_run<TestComp<4>, TestComp<15>, TestComp<24>, TestComp<25>, TestComp<34>, TestComp<35>, TestComp<44>, TestComp<45>>(world, 45);
}
