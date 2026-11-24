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
    ScenarioView view;
    ScenarioForEach for_each;
    ScenarioPatched patched;

    // The scenarios run in this order. Create goes first: it measures
    // the memory that the other scenarios check.
    Scenario* const scenarios[] = { &create, &remove, &view, &for_each, &patched };

    SizeBytes bytes;
    for (Scenario* scenario : scenarios)
    {
        if (!scenario->measure(report, bytes))
            return 1;
    }

    return 0;
}
