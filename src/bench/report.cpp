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
        << "the components of one view. zecs/entt: the time of zecs as a part of the time of entt.\n"
        << "Every row shows the fastest of " << SAMPLES << " samples.\n"
        << std::flush;
}

void Report::table_begin(const char* name, bool with_memory)
{
    out << std::format("\n## {}\n\n", name)
        << "| entities | comps |     entt ns/op |     zecs ns/op | zecs/entt |"
        << (with_memory ? " entt MB | zecs MB |" : "")
        << "\n|---------:|------:|---------------:|---------------:|----------:|"
        << (with_memory ? "--------:|--------:|" : "")
        << "\n" << std::flush;
}

void Report::row(const WorldSize& size, double entt_ns, double zecs_ns, const WorldBytes* bytes)
{
    out << std::format("| {:>7}K | {:>5} | {:>14.0f} | {:>14.0f} | {:>8.1f}% |",
        size.entity_count / 1000, size.comp_count, entt_ns, zecs_ns, zecs_ns / entt_ns * 100.0);

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
