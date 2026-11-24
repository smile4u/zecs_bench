#include <fstream>
#include <iostream>
#include <string_view>

#include "bench/console.h"
#include "bench/report.h"
#include "scenario.h"

int main(int argc, char* argv[])
{
    // The only argument is "--csv <file>". With it the results also go to a CSV file.
    std::ofstream csv;
    if (argc == 3 && std::string_view(argv[1]) == "--csv")
    {
        csv.open(argv[2]);
        if (!csv)
        {
            std::cerr << "Error: can't open " << argv[2] << "\n";
            return 1;
        }
    }
    else if (argc != 1)
    {
        std::cerr << "Usage: zecs_bench [--csv <file>]\n";
        return 1;
    }

#ifndef NDEBUG
    std::cout << "WARNING: NDEBUG isn't defined, so asserts are on. Build Release or RelWithDebInfo to get correct numbers.\n";
#endif

    Report report(std::cout, csv.is_open() ? &csv : nullptr, console_has_colors());
    report.legend();

    ScenarioCreate create;
    ScenarioRemove remove;
    ScenarioViewGet view_get_small(READ_SMALL), view_get_big(READ_BIG);
    ScenarioViewGetOther view_get_other_small(READ_SMALL), view_get_other_big(READ_BIG);
    ScenarioForEach for_each_small(READ_SMALL), for_each_big(READ_BIG);
    ScenarioGet get_small(READ_SMALL), get_big(READ_BIG);
    ScenarioAddRemove add_remove_small(READ_SMALL), add_remove_big(READ_BIG);
    ScenarioPatched patched_small(READ_SMALL), patched_big(READ_BIG);

    // The scenarios run in this order. Create goes first: it measures
    // the memory that the other scenarios check.
    Scenario* const scenarios[] = {
        &create, &remove,
        &view_get_small, &view_get_big,
        &view_get_other_small, &view_get_other_big,
        &for_each_small, &for_each_big,
        &get_small, &get_big,
        &add_remove_small, &add_remove_big,
        &patched_small, &patched_big,
    };

    SizeBytes bytes;
    for (Scenario* scenario : scenarios)
    {
        if (!scenario->measure(report, bytes))
            return 1;
    }

    return 0;
}
