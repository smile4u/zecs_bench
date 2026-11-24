#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <entt/entt.hpp>
#include <zecs/world.h>

struct TestCompBase
{
    int value;
    std::string s;
    std::array<size_t, 16> arr;
};

template<size_t N>
struct TestComp : TestCompBase
{
    inline TestComp() { value = N; };
};

// The largest N of TestComp<N>. The tables in perf_traits.h have
// an entry for every N from 0 to this value.
inline constexpr size_t TEST_COMPS_MAX = 100;

using TestEntityIndex = size_t;

struct TestWorld
{
    void fill(size_t entities_count, size_t coms_count);

    virtual TestEntityIndex entity_add() = 0;
    virtual void entity_rem(TestEntityIndex index) = 0;
    // Adds TestComp<1>..TestComp<coms_count> to the entity.
    virtual void entity_fill(TestEntityIndex index, size_t coms_count) = 0;
    virtual TestCompBase* comp_get(TestEntityIndex index, size_t comp_index) = 0;
    // Adds TestComp<comp_index> to the entity if the entity doesn't have it yet.
    virtual void comp_add(TestEntityIndex index, size_t comp_index) = 0;
    virtual void run_views() = 0;
    // The same groups as in run_views(), but the library calls the body
    // and gives it the components: entt view.each() against zecs query_for_each().
    virtual void run_each() = 0;
    // A pass over one component type: entt view<T> against zecs view_comps<T>.
    virtual void run_comps() = 0;
    // Patches the component of every fourth entity, then visits only
    // the patched components.
    // The two libraries don't do exactly the same work. entt has no built-in
    // pass over patched components, so it uses a reactive storage. It is
    // a separate storage that the on_update signal fills, and the code clears
    // it by hand. In zecs the list of patched components is inside the pool
    // (view_patched_comps<T>).
    virtual void run_patched() = 0;
    // Removes all entities one by one: entt destroy() against zecs entity_remove().
    // The world is empty after it, so this scenario gets its own world (see main.cpp).
    virtual void run_remove() = 0;
    // The sum of value over the components that run_*() change.
    // entt and zecs must give the same sum: it shows that both worlds
    // do the same work.
    virtual uint64_t mutation_sum() = 0;

    virtual ~TestWorld() = default;
};

struct TestWorldEntt : public TestWorld
{
    TestWorldEntt();

    TestEntityIndex entity_add() override;
    void entity_rem(TestEntityIndex index) override;
    void entity_fill(TestEntityIndex index, size_t coms_count) override;
    TestCompBase* comp_get(TestEntityIndex index, size_t comp_index) override;
    void comp_add(TestEntityIndex index, size_t comp_index) override;
    void run_views() override;
    void run_each() override;
    void run_comps() override;
    void run_patched() override;
    void run_remove() override;
    uint64_t mutation_sum() override;

    entt::registry reg;
    // Keeps the entities whose TestComp<8> was patched.
    entt::reactive_mixin<entt::storage<void>> patched;
    std::vector<entt::entity> entities;
};

struct TestWorldZecs : public TestWorld
{
    TestEntityIndex entity_add() override;
    void entity_rem(TestEntityIndex index) override;
    void entity_fill(TestEntityIndex index, size_t coms_count) override;
    TestCompBase* comp_get(TestEntityIndex index, size_t comp_index) override;
    void comp_add(TestEntityIndex index, size_t comp_index) override;
    void run_views() override;
    void run_each() override;
    void run_comps() override;
    void run_patched() override;
    void run_remove() override;
    uint64_t mutation_sum() override;

    zecs::World world;
    std::vector<zecs::Entity> entities;
};
