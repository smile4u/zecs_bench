#include "scenario.h"

#include <algorithm>
#include <random>

namespace
{
template<typename Entity>
std::vector<Entity> shuffled(std::vector<Entity> entities)
{
    // The same seed gives the same order for entt and zecs, because
    // entity i is the same in both worlds.
    std::shuffle(entities.begin(), entities.end(), std::mt19937(1));
    return entities;
}

// The scenario takes only the queries with one component, so N is the component that the query reads.
template<size_t N, size_t... Rest>
void get_run(TestWorldEntt& wrapper, const std::vector<entt::entity>& order, Query<N, Rest...>)
{
    entt::registry& reg = wrapper.reg;
    for (entt::entity e : order)
    {
        if (auto* comp = reg.try_get<TestComp<N>>(e))
            comp->change();
    }
}

template<size_t N, size_t... Rest>
void get_run(TestWorldZecs& wrapper, const std::vector<zecs::Entity>& order, Query<N, Rest...>)
{
    zecs::World& world = wrapper.world;
    for (zecs::Entity e : order)
    {
        if (auto* comp = world.comp_get<TestComp<N>>(e))
            comp->change();
    }
}
} // namespace

void ScenarioGet::prepare(TestWorldEntt& wrapper)
{
    entt_order = shuffled(wrapper.entities);
}

void ScenarioGet::prepare(TestWorldZecs& wrapper)
{
    zecs_order = shuffled(wrapper.entities);
}

void ScenarioGet::run(TestWorldEntt& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldEntt, const std::vector<entt::entity>&>(
        [](TestWorldEntt& w, const std::vector<entt::entity>& order, auto q) { get_run(w, order, q); });
    TABLE[query](wrapper, entt_order);
}

void ScenarioGet::run(TestWorldZecs& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldZecs, const std::vector<zecs::Entity>&>(
        [](TestWorldZecs& w, const std::vector<zecs::Entity>& order, auto q) { get_run(w, order, q); });
    TABLE[query](wrapper, zecs_order);
}
