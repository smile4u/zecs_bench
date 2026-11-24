#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <tuple>
#include <utility>

struct TestWorldEntt;
struct TestWorldZecs;

// A query is a view and the component that the loop body reads and changes.
// The component can be in the view or not.
template<size_t Read, size_t... View>
struct Query
{
    static constexpr size_t READ = Read;
    static constexpr size_t VIEW[] = { View... };
};

// The component is in the view. The views have 1, 2, 4 and 8 components.
// TestComp<1> and TestComp<2> are on almost all entities, so their pools
// are the biggest. So in a wider view the component isn't in the lead pool,
// and the loop reads it by the index from the sparse array.
using InViewQueries = std::tuple<
    Query<1, 1>, Query<1, 1, 3>, Query<1, 1, 3, 4, 5>, Query<1, 1, 3, 4, 5, 6, 7, 8, 9>,
    Query<2, 2>, Query<2, 2, 3>, Query<2, 2, 3, 4, 5>, Query<2, 2, 3, 4, 5, 6, 7, 8, 9>>;

// The component isn't in the view. These are the same views as above,
// but the loop body reads the other one of TestComp<1> and TestComp<2>.
using OtherQueries = std::tuple<
    Query<1, 2>, Query<1, 2, 3>, Query<1, 2, 3, 4, 5>, Query<1, 2, 3, 4, 5, 6, 7, 8, 9>,
    Query<2, 1>, Query<2, 1, 3>, Query<2, 1, 3, 4, 5>, Query<2, 1, 3, 4, 5, 6, 7, 8, 9>>;

// All queries of the bench.
using Queries = decltype(std::tuple_cat(std::declval<InViewQueries>(), std::declval<OtherQueries>()));

inline constexpr size_t QUERY_COUNT = std::tuple_size_v<Queries>;

// A query when its index is known only at run time.
struct QueryInfo
{
    size_t read;
    std::span<const size_t> view;

    // True when the component that the query reads is in its view.
    constexpr bool reads_view() const { return std::ranges::find(view, read) != view.end(); }
};

// Entry [q] is the query std::tuple_element_t<q, Queries>.
inline constexpr auto QUERY_INFOS = []<size_t... Q>(std::index_sequence<Q...>)
{
    return std::array<QueryInfo, QUERY_COUNT>{
        QueryInfo{ std::tuple_element_t<Q, Queries>::READ, std::tuple_element_t<Q, Queries>::VIEW }... };
}(std::make_index_sequence<QUERY_COUNT>{});

// The components that the queries read. The loop bodies change only them,
// so mutation_sum() adds up only these two.
inline constexpr size_t READ_SMALL = 1;
inline constexpr size_t READ_BIG = 2;

static_assert(std::ranges::all_of(QUERY_INFOS, [](const QueryInfo& query) { return query.read == READ_SMALL || query.read == READ_BIG; }));

// The entries of query_table(). Every pack expansion here has only one pack:
// MSVC can't expand Q and Args in one expansion inside a lambda.
template<typename Result, typename WrapperT, typename Run, typename... Args>
struct QueryTable
{
    template<size_t Q>
    static Result call(WrapperT& wrapper, Args... args)
    {
        return Run{}(wrapper, args..., std::tuple_element_t<Q, Queries>{});
    }

    template<size_t... Q>
    static constexpr std::array<Result (*)(WrapperT&, Args...), QUERY_COUNT> make(std::index_sequence<Q...>)
    {
        return { &call<Q>... };
    }
};

// A table of functions for a query that is known only at run time.
// Entry [q] returns Run{}(wrapper, args..., query q). Run is a lambda
// without captures. A scenario gives its own data in `args`.
template<typename Result, typename WrapperT, typename... Args, typename Run>
constexpr auto query_table(Run)
{
    return QueryTable<Result, WrapperT, Run, Args...>::make(std::make_index_sequence<QUERY_COUNT>{});
}

// The number of entities in the view of query q.
size_t match_count(TestWorldEntt& wrapper, size_t query);
size_t match_count(TestWorldZecs& wrapper, size_t query);
