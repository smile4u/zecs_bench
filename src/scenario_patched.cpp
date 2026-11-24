#include "scenario.h"

// entt has no built-in pass over patched components, so it uses a reactive
// storage. It is a separate storage that the on_update signal fills, and
// the code clears it by hand. In zecs the list of patched components is
// inside the pool (view_patched_comps<T>). So the two libraries don't do
// exactly the same work.

void ScenarioPatched::prepare(TestWorldEntt& wrapper)
{
    entt_patched.emplace();
    entt_patched->bind(wrapper.reg);
    entt_patched->on_update<TestComp<8>>();
}

void ScenarioPatched::run(TestWorldEntt& wrapper)
{
    entt::registry& reg = wrapper.reg;

    for (size_t i = 0; i < wrapper.entities.size(); i += 4)
    {
        if (reg.all_of<TestComp<8>>(wrapper.entities[i]))
            reg.patch<TestComp<8>>(wrapper.entities[i]);
    }

    for (entt::entity e : *entt_patched)
        reg.get<TestComp<8>>(e).value += 31;

    entt_patched->clear();
}

void ScenarioPatched::run(TestWorldZecs& wrapper)
{
    zecs::World& world = wrapper.world;

    // The view below gets every component that is patched after this tick.
    const zecs::TickContext ctx(world.tick_get());

    // comp_patch does nothing when the entity doesn't have the component.
    // So the loop doesn't check it first, as entt does with all_of.
    for (size_t i = 0; i < wrapper.entities.size(); i += 4)
        world.comp_patch<TestComp<8>>(wrapper.entities[i]);

    for (zecs::Entity e : world.view_patched_comps<TestComp<8>>(ctx))
        world.comp_get<TestComp<8>>(e)->value += 31;
}
