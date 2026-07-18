#pragma once

// Compile-time dispatch tables for TestComp<N> component access.
//
// This header replaces the previously code-generated files
// (generated_entt.inl / generated_zecs.inl — hundreds of KB of switch-case
// blocks produced by a Python script). The generated code mapped a runtime
// component index onto concrete TestComp<N> template instantiations:
//
//   void entity_fill(index, coms_count) {
//       switch (coms_count) {
//       case 0: break;
//       case 1: add<TestComp<1>>(); break;
//       case 2: add<TestComp<1>>(); add<TestComp<2>>(); break;
//       ...
//       case 100: add<TestComp<1>>(); ... add<TestComp<100>>(); break;
//       default: assert(false);
//       }
//   }
//
// A switch over a runtime value whose cases differ only by a constant
// template argument is exactly what a table of function pointers expresses:
// entry [i] does the work of "case i". The only reason a generator script was
// needed at all is that every entry (i.e. every TestComp<N> instantiation)
// had to be spelled out in the source. std::index_sequence + fold expressions
// spell them out at compile time instead, so no generation step is required.
//
// Mechanics, step by step:
//
// 1. Worker function templates. entt_entity_fill<I>(reg, e) adds components
//    TestComp<1>..TestComp<I> to the entity. The expansion
//    `(r.emplace<TestComp<Js + 1>>(e), ...)` is a fold over the comma
//    operator; when I == 0 the pack is empty, which is well-formed for a
//    comma fold and evaluates to nothing — mirroring "case 0: break".
//
// 2. Table construction. std::make_index_sequence<TEST_COMPS_MAX + 1>{} yields
//    the constants 0..TEST_COMPS_MAX as a parameter pack Is.... An
//    immediately-invoked templated lambda takes the address of every worker
//    instantiation (&worker<Is>...) and packs the pointers into a std::array.
//    The array is constexpr: it is built entirely at compile time and lives
//    in read-only memory.
//
// 3. Runtime dispatch (see perf_ecs.cpp). entity_fill(index, coms_count)
//    becomes fill_table[coms_count](reg, entities[index]) — one indirect
//    call instead of a switch. Semantics are identical to the generated
//    code, including the valid index range 0..TEST_COMPS_MAX, which the
//    callers assert.
//
// Raising the component limit is now a one-line edit of TEST_COMPS_MAX in
// perf_ecs.h; no script has to be re-run.

#include <array>
#include <cstddef>
#include <utility>

#include "perf_ecs.h"

namespace dispatch
{
// ---------------------------------------------------------------------------
// Worker templates: each instantiation I plays the role of one switch case.
// ---------------------------------------------------------------------------

// Adds components TestComp<1>..TestComp<I> to the entity.
// For I == 0 adds nothing (the fold over an empty pack is a no-op).
template<size_t I>
void entt_entity_fill(entt::registry& reg, entt::entity entity)
{
    []<size_t... Js>(entt::registry& r, entt::entity e, std::index_sequence<Js...>)
    {
        (r.emplace<TestComp<Js + 1>>(e), ...);
    }(reg, entity, std::make_index_sequence<I>{});
}

// Returns a pointer to the entity's TestComp<I> component.
template<size_t I>
TestCompBase* entt_comp_get(entt::registry& reg, entt::entity entity)
{
    return &reg.get<TestComp<I>>(entity);
}

// Adds components TestComp<1>..TestComp<I> to the entity.
// For I == 0 adds nothing (the fold over an empty pack is a no-op).
template<size_t I>
void zecs_entity_fill(zecs::World& world, zecs::Entity entity)
{
    []<size_t... Js>(zecs::World& w, zecs::Entity e, std::index_sequence<Js...>)
    {
        (w.comp_add<TestComp<Js + 1>>(e), ...);
    }(world, entity, std::make_index_sequence<I>{});
}

// Returns a pointer to the entity's TestComp<I> component.
template<size_t I>
TestCompBase* zecs_comp_get(zecs::World& world, zecs::Entity entity)
{
    return world.comp_get<TestComp<I>>(entity);
}

// ---------------------------------------------------------------------------
// Dispatch tables: entry [i] points to the worker instantiation for case i.
// Built at compile time by expanding index_sequence<0..TEST_COMPS_MAX> into
// an initializer list of function pointers.
// ---------------------------------------------------------------------------

using EnttFillFn = void(entt::registry&, entt::entity);
using EnttGetFn = TestCompBase*(entt::registry&, entt::entity);
using ZecsFillFn = void(zecs::World&, zecs::Entity);
using ZecsGetFn = TestCompBase*(zecs::World&, zecs::Entity);

constexpr auto entt_fill_table = []<size_t... Is>(std::index_sequence<Is...>)
{
    return std::array<EnttFillFn*, sizeof...(Is)>{ &entt_entity_fill<Is>... };
}(std::make_index_sequence<TEST_COMPS_MAX + 1>{});

constexpr auto entt_get_table = []<size_t... Is>(std::index_sequence<Is...>)
{
    return std::array<EnttGetFn*, sizeof...(Is)>{ &entt_comp_get<Is>... };
}(std::make_index_sequence<TEST_COMPS_MAX + 1>{});

constexpr auto zecs_fill_table = []<size_t... Is>(std::index_sequence<Is...>)
{
    return std::array<ZecsFillFn*, sizeof...(Is)>{ &zecs_entity_fill<Is>... };
}(std::make_index_sequence<TEST_COMPS_MAX + 1>{});

constexpr auto zecs_get_table = []<size_t... Is>(std::index_sequence<Is...>)
{
    return std::array<ZecsGetFn*, sizeof...(Is)>{ &zecs_comp_get<Is>... };
}(std::make_index_sequence<TEST_COMPS_MAX + 1>{});
} // namespace dispatch
