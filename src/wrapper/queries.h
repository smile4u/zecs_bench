#pragma once

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

// All queries of the bench. The loop body changes the first component
// of a query. It is TestComp<1> or TestComp<2> in every query, so
// mutation_sum() needs only these two of them.
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

// A table of functions for a query that is known only at run time.
// Entry [q] returns Run{}(wrapper, query q). Run is a lambda without captures.
template<typename Result, typename WrapperT, typename Run>
constexpr auto query_table(Run)
{
    return []<size_t... Q>(std::index_sequence<Q...>)
    {
        return std::array<Result (*)(WrapperT&), QUERY_COUNT>{
            [](WrapperT& wrapper) -> Result { return Run{}(wrapper, std::tuple_element_t<Q, Queries>{}); }... };
    }(std::make_index_sequence<QUERY_COUNT>{});
}

// The number of entities that query q visits.
size_t match_count(TestWorldEntt& wrapper, size_t query);
size_t match_count(TestWorldZecs& wrapper, size_t query);
