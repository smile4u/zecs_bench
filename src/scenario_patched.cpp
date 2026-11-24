#include "scenario.h"

namespace
{
using EnttPatched = entt::reactive_mixin<entt::storage<void>>;

// The scenario takes only the queries with one component, so N is the component that the query reads.
template<size_t N, size_t... Rest>
void patched_run(TestWorldEntt& wrapper, EnttPatched& patched, Query<N, Rest...>)
{
    entt::registry& reg = wrapper.reg;

    for (size_t i = 0; i < wrapper.entities.size(); i += 4)
    {
        if (reg.all_of<TestComp<N>>(wrapper.entities[i]))
            reg.patch<TestComp<N>>(wrapper.entities[i]);
    }

    for (entt::entity e : patched)
        reg.get<TestComp<N>>(e).change();

    patched.clear();
}

template<size_t N, size_t... Rest>
void patched_run(TestWorldZecs& wrapper, Query<N, Rest...>)
{
    zecs::World& world = wrapper.world;

    // The view below gets every component that is patched after this tick.
    const zecs::TickContext ctx(world.tick_get());

    // comp_patch does nothing when the entity doesn't have the component.
    // So the loop doesn't check it first, as entt does with all_of.
    for (size_t i = 0; i < wrapper.entities.size(); i += 4)
        world.comp_patch<TestComp<N>>(wrapper.entities[i]);

    for (zecs::Entity e : world.view_patched_comps<TestComp<N>>(ctx))
        world.comp_get<TestComp<N>>(e)->change();
}
} // namespace

// The queries with one component take TestComp<1> and TestComp<2> (see queries.h).
// A run patches only one of them, so one storage is enough for both.
void ScenarioPatched::prepare(TestWorldEntt& wrapper)
{
    entt_patched.emplace();
    entt_patched->bind(wrapper.reg);
    entt_patched->on_update<TestComp<1>>().on_update<TestComp<2>>();
}

void ScenarioPatched::run(TestWorldEntt& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldEntt, EnttPatched&>(
        [](TestWorldEntt& w, EnttPatched& patched, auto q) { patched_run(w, patched, q); });
    TABLE[query](wrapper, *entt_patched);
}

void ScenarioPatched::run(TestWorldZecs& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldZecs>([](TestWorldZecs& w, auto q) { patched_run(w, q); });
    TABLE[query](wrapper);
}
