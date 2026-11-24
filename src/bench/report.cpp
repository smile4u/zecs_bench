#include "bench/report.h"

#include <format>
#include <ostream>

#include "bench/runner.h"
#include "wrapper/queries.h"

Report::Report(std::ostream& out)
    : out(out)
{
}

void Report::legend()
{
    out << "comps: entity i gets TestComp<1>..TestComp<i % comps>. Every fifth entity also gets\n"
        << "the components of one query. TestComp<N> with odd N is small (" << sizeof(TestCompSmall) << " bytes),\n"
        << "and with even N it is big (" << sizeof(TestCompBig) << " bytes, with std::string).\n"
        << "query: the kind of its components and their number.\n"
        << "matched: the number of entities that have all components of the query.\n"
        << "ns/op: the median of " << SAMPLES << " samples.\n"
        << "err%: half of the samples are within this distance from the median.\n"
        << "zecs/entt: the time of zecs as a part of the time of entt.\n"
        << std::flush;
}

void Report::table_begin(const char* name, const char* about, bool with_query, bool with_memory)
{
    out << std::format("\n## {}\n\n{}\n\n", name, about)
        << "| entities | comps |"
        << (with_query ? "   query | matched |" : "")
        << "     entt ns/op |  err% |     zecs ns/op |  err% | zecs/entt |"
        << (with_memory ? " entt MB | zecs MB |" : "")
        << "\n|---------:|------:|"
        << (with_query ? "--------:|--------:|" : "")
        << "---------------:|------:|---------------:|------:|----------:|"
        << (with_memory ? "--------:|--------:|" : "")
        << "\n" << std::flush;
}

void Report::row(const Row& row)
{
    out << std::format("| {:>7}K | {:>5} |", row.size.entity_count / 1000, row.size.comp_count);

    if (row.query)
        out << std::format(" {:>5} {} | {:>7} |", row.query->kind, row.query->comps.size(), row.matched);

    out << std::format(" {:>14.0f} | {:>4.1f}% | {:>14.0f} | {:>4.1f}% | {:>8.1f}% |",
        row.entt.ns, row.entt.error * 100.0, row.zecs.ns, row.zecs.error * 100.0, row.zecs.ns / row.entt.ns * 100.0);

    constexpr double MB = 1024.0 * 1024.0;
    if (row.bytes)
        out << std::format(" {:>7.1f} | {:>7.1f} |", double(row.bytes->entt) / MB, double(row.bytes->zecs) / MB);

    out << "\n" << std::flush;
}
