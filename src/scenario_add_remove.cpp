#include "scenario.h"

namespace
{
// The scenario takes only the queries with one component, so N is the component of the query.
// A component that is added again has value N, as after fill(). So mutation_sum() doesn't change.
template<size_t N, size_t... Rest>
void add_remove_run(TestWorldEntt& wrapper, std::vector<entt::entity>& removed, Query<N, Rest...>)
{
    entt::registry& reg = wrapper.reg;

    removed.clear();
    for (size_t i = 0; i < wrapper.entities.size(); i += 4)
    {
        if (reg.remove<TestComp<N>>(wrapper.entities[i]) != 0)
            removed.push_back(wrapper.entities[i]);
    }

    for (entt::entity e : removed)
        reg.emplace<TestComp<N>>(e);
}

template<size_t N, size_t... Rest>
void add_remove_run(TestWorldZecs& wrapper, std::vector<zecs::Entity>& removed, Query<N, Rest...>)
{
    zecs::World& world = wrapper.world;

    removed.clear();
    for (size_t i = 0; i < wrapper.entities.size(); i += 4)
    {
        if (world.comp_remove<TestComp<N>>(wrapper.entities[i]))
            removed.push_back(wrapper.entities[i]);
    }

    for (zecs::Entity e : removed)
        world.comp_add<TestComp<N>>(e);
}
} // namespace

void ScenarioAddRemove::prepare(TestWorldEntt& wrapper)
{
    entt_removed.reserve(wrapper.entities.size() / 4 + 1);
}

void ScenarioAddRemove::prepare(TestWorldZecs& wrapper)
{
    zecs_removed.reserve(wrapper.entities.size() / 4 + 1);
}

void ScenarioAddRemove::run(TestWorldEntt& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldEntt, std::vector<entt::entity>&>(
        [](TestWorldEntt& w, std::vector<entt::entity>& removed, auto q) { add_remove_run(w, removed, q); });
    TABLE[query](wrapper, entt_removed);
}

void ScenarioAddRemove::run(TestWorldZecs& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<void, TestWorldZecs, std::vector<zecs::Entity>&>(
        [](TestWorldZecs& w, std::vector<zecs::Entity>& removed, auto q) { add_remove_run(w, removed, q); });
    TABLE[query](wrapper, zecs_removed);
}
