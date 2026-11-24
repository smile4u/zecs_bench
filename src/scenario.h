#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

#include "bench/grid.h"
#include "wrapper/queries.h"
#include "wrapper/world_entt.h"
#include "wrapper/world_zecs.h"

struct Report;

// A scenario of the bench. main.cpp runs the scenarios one by one.
struct Scenario
{
    // `about` tells what the scenario measures. It is written before the table.
    Scenario(const char* name, const char* about) : name(name), about(about) {}
    virtual ~Scenario() = default;

    // Measures the scenario on the worlds of all sizes and writes its table.
    // `bytes` is the memory of the worlds. ScenarioCreate fills it,
    // and the other scenarios check their worlds with it.
    virtual bool measure(Report& report, SizeBytes& bytes) = 0;

    const char* const name;
    const char* const about;
};

// A scenario that runs on a filled world. It can change components,
// but after a run the entities have the same components as before.
// measure() makes new worlds for every size. On every world it times
// run() for every query of `queries`, one row for a query.
struct WorldScenario : Scenario
{
    // `queries` are indices in QUERY_INFOS.
    WorldScenario(const char* name, const char* about, std::span<const size_t> queries)
        : Scenario(name, about), queries(queries) {}

    bool measure(Report& report, SizeBytes& bytes) override;

    // It is called after fill() and isn't timed. Here a scenario makes
    // the data that it needs for this world.
    virtual void prepare(TestWorldEntt&) {}
    virtual void prepare(TestWorldZecs&) {}
    virtual void run(TestWorldEntt& wrapper, size_t query) = 0;
    virtual void run(TestWorldZecs& wrapper, size_t query) = 0;

    const std::span<const size_t> queries;
};

// ------------------------------------------
// The scenarios. Every one has its own scenario_*.cpp file.
// ------------------------------------------

struct ScenarioCreate : Scenario
{
    ScenarioCreate()
        : Scenario("Create",
            "Makes a world and fills it with entities and components. The destructor of the world isn't timed.\n"
            "It is the only scenario that measures memory. It is what operator new gave while the world was made.")
    {
    }

    bool measure(Report& report, SizeBytes& bytes) override;
};

struct ScenarioRemove : Scenario
{
    ScenarioRemove()
        : Scenario("Remove",
            "Removes all entities of a filled world one by one. entt calls destroy(), and zecs calls entity_remove().\n"
            "Every sample fills a new world before the timer starts.")
    {
    }

    bool measure(Report& report, SizeBytes& bytes) override;
};

struct ScenarioViewGet : WorldScenario
{
    ScenarioViewGet()
        : WorldScenario("ViewGet",
            "A view for every query. The loop body takes every component of the query from the view,\n"
            "reads all of them and changes the first one. entt uses view.get(). zecs views have no such\n"
            "function yet, so zecs takes the components from the world with world.comp_get(), as in ViewWorldGet.",
            ALL_QUERIES)
    {
    }

    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;
};

struct ScenarioViewWorldGet : WorldScenario
{
    ScenarioViewWorldGet()
        : WorldScenario("ViewWorldGet",
            "The same views and the same loop body as in ViewGet, but both libraries take the components\n"
            "from the world. entt uses registry.get(), and zecs uses world.comp_get(). Both find the pool\n"
            "of the component type on every call.",
            ALL_QUERIES)
    {
    }

    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;
};

struct ScenarioForEach : WorldScenario
{
    ScenarioForEach()
        : WorldScenario("ForEach",
            "The same queries and the same loop body as in ViewGet, but the library calls the body and gives it\n"
            "the components. entt uses view.each(), and zecs uses query_for_each().",
            ALL_QUERIES)
    {
    }

    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;
};

struct ScenarioGet : WorldScenario
{
    ScenarioGet()
        : WorldScenario("Get",
            "Takes the component of the query from every entity in a random order and changes it.\n"
            "Some entities don't have it. entt uses registry.try_get(), and zecs uses world.comp_get().\n"
            "Both find the pool of the component type and then the component of the entity.",
            ONE_COMP_QUERIES)
    {
    }

    void prepare(TestWorldEntt& wrapper) override;
    void prepare(TestWorldZecs& wrapper) override;
    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;

private:
    // All entities of the world in a random order. The order is the same for entt and zecs.
    std::vector<entt::entity> entt_order;
    std::vector<zecs::Entity> zecs_order;
};

struct ScenarioAddRemove : WorldScenario
{
    ScenarioAddRemove()
        : WorldScenario("AddRemove",
            "Removes the component of the query from every fourth entity that has it, then adds it to them again.\n"
            "After a run the entities have the same components, but the order of the components in the pool changes.\n"
            "entt uses remove() and emplace(), and zecs uses comp_remove() and comp_add().",
            ONE_COMP_QUERIES)
    {
    }

    void prepare(TestWorldEntt& wrapper) override;
    void prepare(TestWorldZecs& wrapper) override;
    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;

private:
    // The entities that a run has removed the component from.
    // prepare() reserves the memory, so the run doesn't allocate it.
    std::vector<entt::entity> entt_removed;
    std::vector<zecs::Entity> zecs_removed;
};

struct ScenarioPatched : WorldScenario
{
    ScenarioPatched()
        : WorldScenario("Patched",
            "Patches the component of the query on every fourth entity, then visits only the patched components.\n"
            "entt has no view of patched components, so it uses a reactive storage. It is a separate storage\n"
            "that the on_update signal fills, and the code clears it by hand. zecs keeps the list of patched\n"
            "components inside the pool. So the two libraries don't do exactly the same work.",
            ONE_COMP_QUERIES)
    {
    }

    using WorldScenario::prepare;
    void prepare(TestWorldEntt& wrapper) override;
    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;

private:
    // Keeps the entities whose component was patched. prepare() makes it
    // again for every new world. Its destructor doesn't use the registry,
    // so it can be destroyed after the registry of the old world.
    std::optional<entt::reactive_mixin<entt::storage<void>>> entt_patched;
};
