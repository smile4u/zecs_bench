#include <iostream>

#include "bench/report.h"
#include "scenario.h"

int main()
{
#ifndef NDEBUG
    std::cout << "WARNING: NDEBUG isn't defined, so asserts are on. Build Release or RelWithDebInfo to get correct numbers.\n";
#endif

    Report report(std::cout);
    report.legend();

    ScenarioCreate create;
    ScenarioRemove remove;
    ScenarioViewGet view_get;
    ScenarioViewWorldGet view_world_get;
    ScenarioForEach for_each;
    ScenarioGet get;
    ScenarioAddRemove add_remove;
    ScenarioPatched patched;

    // The scenarios run in this order. Create goes first: it measures
    // the memory that the other scenarios check.
    Scenario* const scenarios[] = { &create, &remove, &view_get, &view_world_get, &for_each, &get, &add_remove, &patched };

    SizeBytes bytes;
    for (Scenario* scenario : scenarios)
    {
        if (!scenario->measure(report, bytes))
            return 1;
    }

    return 0;
}
