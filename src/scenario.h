#pragma once

#include <optional>

#include "bench/grid.h"
#include "wrapper/world_entt.h"
#include "wrapper/world_zecs.h"

struct Report;

// A scenario of the bench. main.cpp runs the scenarios one by one.
struct Scenario
{
    explicit Scenario(const char* name) : name(name) {}
    virtual ~Scenario() = default;

    // Measures the scenario on the worlds of all sizes and writes its tables.
    // `bytes` is the memory of the worlds. ScenarioCreateRemove fills it,
    // and the other scenarios check their worlds with it.
    virtual bool measure(Report& report, SizeBytes& bytes) = 0;

    const char* const name;
};

// A scenario that runs on a filled world. It can change the values of
// components, but it doesn't add or remove components.
// measure() makes new worlds for every size and times run() on them.
struct WorldScenario : Scenario
{
    // `note` is written under the table. It can be nullptr.
    WorldScenario(const char* name, const char* note) : Scenario(name), note(note) {}

    bool measure(Report& report, SizeBytes& bytes) override;

    // It is called after fill() and isn't timed. Here a scenario makes
    // the data that it needs for this world.
    virtual void prepare(TestWorldEntt&) {}
    virtual void prepare(TestWorldZecs&) {}
    virtual void run(TestWorldEntt& wrapper) = 0;
    virtual void run(TestWorldZecs& wrapper) = 0;

    const char* const note;
};

// ------------------------------------------
// The scenarios. Every one has its own scenario_*.cpp file.
// ------------------------------------------

// Makes and fills a world. The destructor of the world isn't timed.
// It is the only scenario that measures the memory of the worlds.
struct ScenarioCreate : Scenario
{
    ScenarioCreate() : Scenario("Create") {}
    bool measure(Report& report, SizeBytes& bytes) override;
};

// Removes all entities of a filled world one by one. Every sample fills
// a new world before the timer starts.
struct ScenarioRemove : Scenario
{
    ScenarioRemove() : Scenario("Remove") {}
    bool measure(Report& report, SizeBytes& bytes) override;
};

// Five views with 2 to 8 components. The loop body takes a component
// from the world and changes it.
struct ScenarioView : WorldScenario
{
    ScenarioView() : WorldScenario("View", nullptr) {}
    void run(TestWorldEntt& wrapper) override;
    void run(TestWorldZecs& wrapper) override;
};

// The same views as in ScenarioView, but the library calls the body
// and gives it the components. entt uses view.each(), and zecs uses
// query_for_each().
struct ScenarioForEach : WorldScenario
{
    ScenarioForEach() : WorldScenario("ForEach", nullptr) {}
    void run(TestWorldEntt& wrapper) override;
    void run(TestWorldZecs& wrapper) override;
};

// A pass over one component type. entt uses view<T>, and zecs uses
// view_comps<T>.
struct ScenarioComps : WorldScenario
{
    ScenarioComps() : WorldScenario("Comps", nullptr) {}
    void run(TestWorldEntt& wrapper) override;
    void run(TestWorldZecs& wrapper) override;
};

// Patches the component of every fourth entity, then visits only
// the patched components.
struct ScenarioPatched : WorldScenario
{
    ScenarioPatched()
        : WorldScenario("Patched",
            "Patched: entt has no view of patched components, so its reactive storage is measured.\n"
            "zecs keeps the list of patched components inside the pool. entt fills a separate storage from a signal.")
    {
    }

    using WorldScenario::prepare;
    void prepare(TestWorldEntt& wrapper) override;
    void run(TestWorldEntt& wrapper) override;
    void run(TestWorldZecs& wrapper) override;

private:
    // Keeps the entities whose TestComp<8> was patched. prepare() makes it
    // again for every new world. Its destructor doesn't use the registry,
    // so it can be destroyed after the registry of the old world.
    std::optional<entt::reactive_mixin<entt::storage<void>>> entt_patched;
};
