#pragma once

#include <cstddef>
#include <iosfwd>

#include "bench/grid.h"

// The time of one library in one row.
struct RowTime
{
    // The median time of one run.
    double ns;
    // The median absolute percent error of the samples (see picobench.patch).
    // 0.05 means that half of the samples are within 5% of the median.
    double error;
};

// A row of a table. A table writes only the columns that it has (see Report::table_begin()).
struct Row
{
    WorldSize size;
    // The number of components in the view of the query.
    size_t view = 0;
    // The number of entities in the view of the query.
    size_t matched = 0;
    RowTime entt = {};
    RowTime zecs = {};
    // The memory of the worlds.
    const WorldBytes* bytes = nullptr;
};

// Writes the results as markdown, one table for every scenario.
// Every row is written at once, so the output shows the progress.
struct Report
{
    // With `colors` a row is green when zecs is faster and red when entt is
    // faster (see Report::row()). Use it only for a terminal, see console.h.
    Report(std::ostream& out, bool colors);

    // The text before all tables.
    void legend();

    // Writes the name of a scenario, the text about it and the header of its table.
    // The columns of the table are kept for row() until the next table.
    void table_begin(const char* name, const char* about, bool with_view, bool with_matched, bool with_memory);
    void row(const Row& row);

private:
    std::ostream& out;
    const bool colors;
    bool with_view = false;
    bool with_matched = false;
    bool with_memory = false;
};
