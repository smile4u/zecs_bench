#pragma once

#include <iosfwd>

#include "bench/grid.h"

// Writes the results as markdown: one table for every scenario.
// Every row is written at once, so the output shows the progress.
struct Report
{
    explicit Report(std::ostream& out);

    // The text before all tables.
    void legend();

    void table_begin(const char* name, bool with_memory);
    // `bytes` is for a table that was started with `with_memory`.
    // For other tables it is nullptr.
    void row(const WorldSize& size, double entt_ns, double zecs_ns, const WorldBytes* bytes = nullptr);
    // `note` is written under the table. It can be nullptr.
    void table_end(const char* note);

private:
    std::ostream& out;
};
