#include "scenario.h"

#include <tuple>

namespace
{
// query_table() makes a function for every query. ViewGet and ForEach take
// only InViewQueries, and their code compiles only when Read is in View.
// So for the other queries their functions are empty.

// ViewGet. The view already has its pools, so view.get() doesn't look for them.
template<size_t Read, size_t... View>
void view_get_run(TestWorldEntt& wrapper, Query<Read, View...>)
{
    if constexpr (((Read == View) || ...))
    {
        auto view = wrapper.reg.view<TestComp<View>...>();
        for (entt::entity e : view)
            view.template get<TestComp<Read>>(e).value += 1;
    }
}

// zecs views have no comp_get, so world.comp_get<T>(e) finds the pool on every call.
// TODO(zecs): add view.comp_get<T>(e). Then the loop doesn't need to find the pool.
template<size_t Read, size_t... View>
void view_get_run(TestWorldZecs& wrapper, Query<Read, View...>)
{
    zecs::World& world = wrapper.world;

    // For one component type zecs has its own view.
    if constexpr (sizeof...(View) == 1)
    {
        for (zecs::Entity e : world.view_comps<TestComp<View>...>())
            world.comp_get<TestComp<Read>>(e)->value += 1;
    }
    else
    {
        for (zecs::Entity e : world.view_comp_group<TestComp<View>...>())
            world.comp_get<TestComp<Read>>(e)->value += 1;
    }
}

// ViewGetOther. The component isn't in the view, so both libraries take it
// from the world. Some entities of the view don't have it.
template<size_t Read, size_t... View>
void view_get_other_run(TestWorldEntt& wrapper, Query<Read, View...>)
{
    entt::registry& reg = wrapper.reg;
    for (entt::entity e : reg.view<TestComp<View>...>())
    {
        if (auto* comp = reg.try_get<TestComp<Read>>(e))
            comp->value += 1;
    }
}

template<size_t Read, size_t... View>
void view_get_other_run(TestWorldZecs& wrapper, Query<Read, View...>)
{
    zecs::World& world = wrapper.world;

    if constexpr (sizeof...(View) == 1)
    {
        for (zecs::Entity e : world.view_comps<TestComp<View>...>())
        {
            if (auto* comp = world.comp_get<TestComp<Read>>(e))
                comp->value += 1;
        }
    }
    else
    {
        for (zecs::Entity e : world.view_comp_group<TestComp<View>...>())
        {
            if (auto* comp = world.comp_get<TestComp<Read>>(e))
                comp->value += 1;
        }
    }
}

// ForEach. The library gives the body all components of the view,
// and the body changes only the component of the query.
template<size_t Read, size_t... View>
void each_run(TestWorldEntt& wrapper, Query<Read, View...>)
{
    if constexpr (((Read == View) || ...))
    {
        wrapper.reg.view<TestComp<View>...>().each(
            [](TestComp<View>&... comps) { std::get<TestComp<Read>&>(std::tie(comps...)).value += 1; });
    }
}

template<size_t Read, size_t... View>
void each_run(TestWorldZecs& wrapper, Query<Read, View...>)
{
    if constexpr (((Read == View) || ...))
    {
        zecs::World& world = wrapper.world;
        world.query_for_each(world.query_scope<TestComp<View>...>(),
            [](zecs::Entity, TestComp<View>&... comps) { std::get<TestComp<Read>&>(std::tie(comps...)).value += 1; });
    }
}
} // namespace

void ScenarioViewGet::run(TestWorldEntt& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldEntt>([](TestWorldEntt& w, auto q) { view_get_run(w, q); });
    TABLE[query](wrapper);
}

void ScenarioViewGet::run(TestWorldZecs& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldZecs>([](TestWorldZecs& w, auto q) { view_get_run(w, q); });
    TABLE[query](wrapper);
}

void ScenarioViewGetOther::run(TestWorldEntt& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldEntt>([](TestWorldEntt& w, auto q) { view_get_other_run(w, q); });
    TABLE[query](wrapper);
}

void ScenarioViewGetOther::run(TestWorldZecs& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldZecs>([](TestWorldZecs& w, auto q) { view_get_other_run(w, q); });
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
