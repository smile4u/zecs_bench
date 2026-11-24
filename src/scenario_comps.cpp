#include "scenario.h"

void ScenarioComps::run(TestWorldEntt& wrapper)
{
    entt::registry& reg = wrapper.reg;

    for (entt::entity e : reg.view<TestComp<2>>())
        reg.get<TestComp<2>>(e).value += 21;

    for (entt::entity e : reg.view<TestComp<6>>())
        reg.get<TestComp<6>>(e).value += 22;

    for (entt::entity e : reg.view<TestComp<9>>())
        reg.get<TestComp<9>>(e).value += 23;
}

void ScenarioComps::run(TestWorldZecs& wrapper)
{
    zecs::World& world = wrapper.world;

    for (zecs::Entity e : world.view_comps<TestComp<2>>())
        world.comp_get<TestComp<2>>(e)->value += 21;

    for (zecs::Entity e : world.view_comps<TestComp<6>>())
        world.comp_get<TestComp<6>>(e)->value += 22;

    for (zecs::Entity e : world.view_comps<TestComp<9>>())
        world.comp_get<TestComp<9>>(e)->value += 23;
}
