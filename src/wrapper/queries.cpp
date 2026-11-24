#include "wrapper/queries.h"

#include "wrapper/world_entt.h"
#include "wrapper/world_zecs.h"

namespace
{
template<size_t Read, size_t... View>
size_t count(TestWorldEntt& wrapper, Query<Read, View...>)
{
    size_t visited = 0;
    for ([[maybe_unused]] entt::entity e : wrapper.reg.view<TestComp<View>...>())
        ++visited;
    return visited;
}

template<size_t Read, size_t... View>
size_t count(TestWorldZecs& wrapper, Query<Read, View...>)
{
    size_t visited = 0;
    for ([[maybe_unused]] zecs::Entity e : wrapper.world.view_comp_group<TestComp<View>...>())
        ++visited;
    return visited;
}
} // namespace

size_t match_count(TestWorldEntt& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<size_t, TestWorldEntt>([](TestWorldEntt& w, auto q) { return count(w, q); });
    return TABLE[query](wrapper);
}

size_t match_count(TestWorldZecs& wrapper, size_t query)
{
    static constexpr auto TABLE = query_table<size_t, TestWorldZecs>([](TestWorldZecs& w, auto q) { return count(w, q); });
    return TABLE[query](wrapper);
}
