#pragma once

#include <cstddef>
#include <iosfwd>

#include "bench/grid.h"

struct QueryInfo;

// The time of one library in one row.
struct RowTime
{
    // The median time of one run.
    double ns;
    // The median absolute percent error of the samples (see picobench.patch).
    // 0.05 means that half of the samples are within 5% of the median.
    double error;
};

struct Row
{
    WorldSize size;
    // The query of the row. It is nullptr in a table without the query columns.
    const QueryInfo* query = nullptr;
    // The number of entities that have all components of the query.
    size_t matched = 0;
    RowTime entt = {};
    RowTime zecs = {};
    // The memory of the worlds. It is nullptr in a table without the memory columns.
    const WorldBytes* bytes = nullptr;
};

// Writes the results as markdown, one table for every scenario.
// Every row is written at once, so the output shows the progress.
struct Report
{
    explicit Report(std::ostream& out);

    // The text before all tables.
    void legend();

    // Writes the name of a scenario, the text about it and the header of its table.
    void table_begin(const char* name, const char* about, bool with_query, bool with_memory);
    void row(const Row& row);

private:
    std::ostream& out;
};
