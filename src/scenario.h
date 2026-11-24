#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
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
    Scenario(std::string name, const char* about) : name(std::move(name)), about(about) {}
    virtual ~Scenario() = default;

    // Measures the scenario on the worlds of all sizes and writes its table.
    // `bytes` is the memory of the worlds. ScenarioCreate fills it,
    // and the other scenarios check their worlds with it.
    virtual bool measure(Report& report, SizeBytes& bytes) = 0;

    const std::string name;
    const char* const about;
};

// The views of the queries that a WorldScenario takes.
enum class Views
{
    // The views have the component that the loop body reads.
    with_read,
    // The views don't have the component that the loop body reads.
    without_read,
    // Only the view of the one component that the loop body reads.
    only_read,
};

// A scenario that runs on a filled world. It can change components,
// but after a run the entities have the same components as before.
// measure() makes new worlds for every size. On every world it times
// run() for every query that the scenario takes, one row for a query.
struct WorldScenario : Scenario
{
    // `read` is READ_SMALL or READ_BIG. The scenario takes the queries that
    // read this component and have `views`. The name gets the kind of `read`.
    WorldScenario(const char* name, const char* about, size_t read, Views views);

    bool measure(Report& report, SizeBytes& bytes) override;

    // It is called after fill() and isn't timed. Here a scenario makes
    // the data that it needs for this world.
    virtual void prepare(TestWorldEntt&) {}
    virtual void prepare(TestWorldZecs&) {}
    // `query` is an index in QUERY_INFOS.
    virtual void run(TestWorldEntt& wrapper, size_t query) = 0;
    virtual void run(TestWorldZecs& wrapper, size_t query) = 0;

    const size_t read;
    const Views views;
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
    explicit ScenarioViewGet(size_t read)
        : WorldScenario("ViewGet",
            "A view, and the loop body takes a component of this view and changes it. A view of one component\n"
            "reads it in order. A wider view reads it by the index from the sparse array, because this component\n"
            "isn't in the lead pool. entt uses view.get(). zecs views have no such function yet, so zecs takes\n"
            "the component from the world with world.comp_get(), as in ViewGetOther.",
            read, Views::with_read)
    {
    }

    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;
};

struct ScenarioViewGetOther : WorldScenario
{
    explicit ScenarioViewGetOther(size_t read)
        : WorldScenario("ViewGetOther",
            "The same views as in ViewGet, but the loop body takes a component that isn't in the view\n"
            "and changes it. Some entities of the view don't have it. entt uses registry.try_get(), and zecs\n"
            "uses world.comp_get(). Both find the pool of the component type on every call.",
            read, Views::without_read)
    {
    }

    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;
};

struct ScenarioForEach : WorldScenario
{
    explicit ScenarioForEach(size_t read)
        : WorldScenario("ForEach",
            "The same views as in ViewGet, but the library calls the loop body and gives it all components\n"
            "of the view. The body changes one of them. entt uses view.each(), and zecs uses query_for_each().",
            read, Views::with_read)
    {
    }

    void run(TestWorldEntt& wrapper, size_t query) override;
    void run(TestWorldZecs& wrapper, size_t query) override;
};

struct ScenarioGet : WorldScenario
{
    explicit ScenarioGet(size_t read)
        : WorldScenario("Get",
            "Takes the component from every entity in a random order and changes it. Some entities\n"
            "don't have it. entt uses registry.try_get(), and zecs uses world.comp_get(). Both find\n"
            "the pool of the component type and then the component of the entity.",
            read, Views::only_read)
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
    explicit ScenarioAddRemove(size_t read)
        : WorldScenario("AddRemove",
            "Removes the component from every fourth entity that has it, then adds it to them again.\n"
            "After a run the entities have the same components, but the order of the components in the pool changes.\n"
            "entt uses remove() and emplace(), and zecs uses comp_remove() and comp_add().",
            read, Views::only_read)
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
    explicit ScenarioPatched(size_t read)
        : WorldScenario("Patched",
            "Patches the component on every fourth entity, then visits only the patched components.\n"
            "entt has no view of patched components, so it uses a reactive storage. It is a separate storage\n"
            "that the on_update signal fills, and the code clears it by hand. zecs keeps the list of patched\n"
            "components inside the pool. So the two libraries don't do exactly the same work.",
            read, Views::only_read)
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
