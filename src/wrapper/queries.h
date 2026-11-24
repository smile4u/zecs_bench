#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <tuple>
#include <utility>

struct TestWorldEntt;
struct TestWorldZecs;

// A query is the list of components that a view or a loop asks for.
template<size_t... N>
struct Query
{
    static constexpr size_t COMPS[] = { N... };
    // TestComp<N> with odd N is small, and with even N it is big (see test_comps.h).
    static constexpr const char* KIND = ((N % 2 == 1) && ...) ? "small" : ((N % 2 == 0) && ...) ? "big" : "mixed";
};

// All queries of the bench.
using Queries = std::tuple<
    Query<1>, Query<1, 3>, Query<1, 3, 5, 7>, Query<1, 3, 5, 7, 9, 11, 13, 15>,
    Query<2>, Query<2, 4>, Query<2, 4, 6, 8>, Query<2, 4, 6, 8, 10, 12, 14, 16>,
    Query<1, 2>, Query<1, 2, 3, 4>, Query<1, 2, 3, 4, 5, 6, 7, 8>>;

inline constexpr size_t QUERY_COUNT = std::tuple_size_v<Queries>;

// A query when its index is known only at run time.
struct QueryInfo
{
    const char* kind;
    std::span<const size_t> comps;
};

// Entry [q] is the query std::tuple_element_t<q, Queries>.
inline constexpr auto QUERY_INFOS = []<size_t... Q>(std::index_sequence<Q...>)
{
    return std::array<QueryInfo, QUERY_COUNT>{
        QueryInfo{ std::tuple_element_t<Q, Queries>::KIND, std::tuple_element_t<Q, Queries>::COMPS }... };
}(std::make_index_sequence<QUERY_COUNT>{});

// The scenarios change only the first component of a query. It must be
// TestComp<1> or TestComp<2>, because mutation_sum() adds up only these two.
static_assert(std::ranges::all_of(QUERY_INFOS, [](const QueryInfo& query) { return query.comps[0] == 1 || query.comps[0] == 2; }));

// The indices of all queries.
inline constexpr auto ALL_QUERIES = []<size_t... Q>(std::index_sequence<Q...>)
{
    return std::array<size_t, QUERY_COUNT>{ Q... };
}(std::make_index_sequence<QUERY_COUNT>{});

// The indices of the queries with one component. One of them is small,
// and the other one is big.
inline constexpr auto ONE_COMP_QUERIES = []
{
    std::array<size_t, 2> result{};
    size_t count = 0;
    for (size_t q = 0; q < QUERY_COUNT; ++q)
    {
        if (QUERY_INFOS[q].comps.size() == 1)
            result.at(count++) = q;
    }
    // It doesn't compile when the number of such queries isn't 2.
    if (count != result.size())
        throw "Queries must have two queries with one component";
    return result;
}();

// A table of functions for a query that is known only at run time.
// Entry [q] returns Run{}(wrapper, args..., query q). Run is a lambda
// without captures. A scenario gives its own data in `args`.
template<typename Result, typename WrapperT, typename... Args, typename Run>
constexpr auto query_table(Run)
{
    return []<size_t... Q>(std::index_sequence<Q...>)
    {
        return std::array<Result (*)(WrapperT&, Args...), QUERY_COUNT>{
            [](WrapperT& wrapper, Args... args) -> Result
            {
                return Run{}(wrapper, args..., std::tuple_element_t<Q, Queries>{});
            }... };
    }(std::make_index_sequence<QUERY_COUNT>{});
}

// The number of entities that have all components of query q.
size_t match_count(TestWorldEntt& wrapper, size_t query);
size_t match_count(TestWorldZecs& wrapper, size_t query);
