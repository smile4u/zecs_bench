#include "bench/report.h"

#include <format>
#include <ostream>

#include "bench/runner.h"

Report::Report(std::ostream& out)
    : out(out)
{
}

void Report::legend()
{
    out << "comps: entity i gets TestComp<1>..TestComp<i % comps>. Every fifth entity also gets\n"
        << "the components of one view. TestComp<N> with odd N is small (" << sizeof(TestCompSmall) << " bytes),\n"
        << "and with even N it is big (" << sizeof(TestCompBig) << " bytes, with std::string).\n"
        << "ns/op: the median of " << SAMPLES << " samples.\n"
        << "err%: half of the samples are within this distance from the median.\n"
        << "zecs/entt: the time of zecs as a part of the time of entt.\n"
        << std::flush;
}

void Report::table_begin(const char* name, bool with_memory)
{
    out << std::format("\n## {}\n\n", name)
        << "| entities | comps |     entt ns/op |  err% |     zecs ns/op |  err% | zecs/entt |"
        << (with_memory ? " entt MB | zecs MB |" : "")
        << "\n|---------:|------:|---------------:|------:|---------------:|------:|----------:|"
        << (with_memory ? "--------:|--------:|" : "")
        << "\n" << std::flush;
}

void Report::row(const WorldSize& size, const RowTime& entt, const RowTime& zecs, const WorldBytes* bytes)
{
    out << std::format("| {:>7}K | {:>5} | {:>14.0f} | {:>4.1f}% | {:>14.0f} | {:>4.1f}% | {:>8.1f}% |",
        size.entity_count / 1000, size.comp_count,
        entt.ns, entt.error * 100.0, zecs.ns, zecs.error * 100.0, zecs.ns / entt.ns * 100.0);

    constexpr double MB = 1024.0 * 1024.0;
    if (bytes)
        out << std::format(" {:>7.1f} | {:>7.1f} |", double(bytes->entt) / MB, double(bytes->zecs) / MB);

    out << "\n" << std::flush;
}

void Report::table_end(const char* note)
{
    if (note)
        out << "\n" << note << "\n" << std::flush;
}
