#pragma once

#include <cstddef>
#include <optional>

#include "bench/grid.h"
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

// A scenario that runs on a filled world. It can change the values of
// components, but it doesn't add or remove components.
// measure() makes new worlds for every size and times run() on them.
struct WorldScenario : Scenario
{
    // With `with_queries` the table has a row for every query of queries.h
    // on every world, and run() gets the index of the query. Without it
    // the table has one row for a world, and the index is 0.
    WorldScenario(const char* name, const char* about, bool with_queries)
        : Scenario(name, about), with_queries(with_queries) {}

    bool measure(Report& report, SizeBytes& bytes) override;

    // It is called after fill() and isn't timed. Here a scenario makes
    // the data that it needs for this world.
    virtual void prepare(TestWorldEntt&) {}
    virtual void prepare(TestWorldZecs&) {}
    virtual void run(TestWorldEntt& wrapper, size_t query) = 0;
    virtual void run(TestWorldZecs& wrapper, size_t query) = 0;

    const bool with_queries;
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

struct ScenarioView : WorldScenario
{
    ScenarioView()
        : WorldScenario("View",
            "A view for every query. The loop body takes every component of the query from the world,\n"
            "reads all of them and changes the first one. entt uses registry.get(), and zecs uses world.comp_get().",
            true)
    {
    }

    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;
};

struct ScenarioForEach : WorldScenario
{
    ScenarioForEach()
        : WorldScenario("ForEach",
            "The same queries and the same loop body as in View, but the library calls the body and gives it\n"
            "the components. entt uses view.each(), and zecs uses query_for_each().",
            true)
    {
    }

    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;
};

struct ScenarioPatched : WorldScenario
{
    ScenarioPatched()
        : WorldScenario("Patched",
            "Patches TestComp<8> (a big component) of every fourth entity, then visits only the patched components.\n"
            "entt has no view of patched components, so it uses a reactive storage. It is a separate storage\n"
            "that the on_update signal fills, and the code clears it by hand. zecs keeps the list of patched\n"
            "components inside the pool. So the two libraries don't do exactly the same work.",
            false)
    {
    }

    using WorldScenario::prepare;
    void prepare(TestWorldEntt& wrapper) override;
    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;

private:
    // Keeps the entities whose TestComp<8> was patched. prepare() makes it
    // again for every new world. Its destructor doesn't use the registry,
    // so it can be destroyed after the registry of the old world.
    std::optional<entt::reactive_mixin<entt::storage<void>>> entt_patched;
};
