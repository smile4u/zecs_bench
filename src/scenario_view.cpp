#include "scenario.h"

#include "wrapper/queries.h"

namespace
{
// The loop body of View and ForEach. It reads every component of the query
// and changes the first one. So a wide query costs more than a narrow one,
// and View and ForEach do the same work.
template<typename First, typename... Rest>
inline void body(First& first, const Rest&... rest)
{
    first.value += 1 + (0 + ... + rest.value);
}

template<size_t First, size_t... Rest>
void view_run(TestWorldEntt& wrapper, Query<First, Rest...>)
{
    entt::registry& reg = wrapper.reg;
    for (entt::entity e : reg.view<TestComp<First>, TestComp<Rest>...>())
        body(reg.get<TestComp<First>>(e), reg.get<TestComp<Rest>>(e)...);
}

// The components are taken from the world, as in entt above.
// zecs views have no comp_get, so world.comp_get<T>(e) finds the pool on every call.
// TODO(zecs): add view.comp_get<T>(e). Then the loop doesn't need to find the pool.
template<size_t First, size_t... Rest>
void view_run(TestWorldZecs& wrapper, Query<First, Rest...>)
{
    zecs::World& world = wrapper.world;

    // For one component type zecs has its own view.
    if constexpr (sizeof...(Rest) == 0)
    {
        for (zecs::Entity e : world.view_comps<TestComp<First>>())
            body(*world.comp_get<TestComp<First>>(e));
    }
    else
    {
        for (zecs::Entity e : world.view_comp_group<TestComp<First>, TestComp<Rest>...>())
            body(*world.comp_get<TestComp<First>>(e), *world.comp_get<TestComp<Rest>>(e)...);
    }
}

template<size_t... N>
void each_run(TestWorldEntt& wrapper, Query<N...>)
{
    wrapper.reg.view<TestComp<N>...>().each([](TestComp<N>&... comps) { body(comps...); });
}

template<size_t... N>
void each_run(TestWorldZecs& wrapper, Query<N...>)
{
    zecs::World& world = wrapper.world;
    world.query_for_each(world.query_scope<TestComp<N>...>(), [](zecs::Entity, TestComp<N>&... comps) { body(comps...); });
}
} // namespace

void ScenarioView::run(TestWorldEntt& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldEntt>([](TestWorldEntt& w, auto q) { view_run(w, q); });
    TABLE[query](wrapper);
}

void ScenarioView::run(TestWorldZecs& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldZecs>([](TestWorldZecs& w, auto q) { view_run(w, q); });
    TABLE[query](wrapper);
}

void ScenarioForEach::run(TestWorldEntt& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldEntt>([](TestWorldEntt& w, auto q) { each_run(w, q); });
    TABLE[query](wrapper);
}

void ScenarioForEach::run(TestWorldZecs& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldZecs>([](TestWorldZecs& w, auto q) { each_run(w, q); });
    TABLE[query](wrapper);
}
