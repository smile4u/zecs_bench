#include "bench/report.h"

#include <algorithm>
#include <format>
#include <ostream>

#include "bench/runner.h"

namespace
{
// A difference smaller than this is too small to show it with a color.
constexpr double COLOR_MIN_DIFFERENCE = 0.1;

const char* const GREEN = "\x1b[32m";
const char* const RED = "\x1b[31m";
const char* const RESET = "\x1b[0m";
} // namespace

Report::Report(std::ostream& out, bool colors)
    : out(out)
    , colors(colors)
{
}

void Report::legend()
{
    out << "comps: entity i gets TestComp<1>..TestComp<i % comps>. Every fifth entity also gets\n"
        << "the components of the view of one query. TestComp<N> with odd N is small (" << sizeof(TestCompSmall) << " bytes),\n"
        << "and with even N it is big (" << sizeof(TestCompBig) << " bytes, with std::string).\n"
        << "small or big in the name of a table: the kind of the component that the loop body reads and changes.\n"
        << "view: the number of components in the view. matched: the number of entities in the view.\n"
        << "ns/op: the median of " << SAMPLES << " samples.\n"
        << "err%: half of the samples are within this distance from the median.\n"
        << "zecs/entt: the time of zecs as a part of the time of entt.\n";

    if (colors)
    {
        out << std::format("{}Green{} row: zecs is faster. {}Red{} row: entt is faster. The difference is more than {:.0f}%\n",
                GREEN, RESET, RED, RESET, COLOR_MIN_DIFFERENCE * 100.0)
            << "and more than err% of entt and zecs together. Smaller differences can be noise.\n";
    }

    out << std::flush;
}

void Report::table_begin(const char* name, const char* about, bool with_view, bool with_matched, bool with_memory)
{
    this->with_view = with_view;
    this->with_matched = with_matched;
    this->with_memory = with_memory;

    out << std::format("\n## {}\n\n{}\n\n", name, about)
        << "| entities | comps |"
        << (with_view ? " view |" : "")
        << (with_matched ? " matched |" : "")
        << "     entt ns/op |  err% |     zecs ns/op |  err% | zecs/entt |"
        << (with_memory ? " entt MB | zecs MB |" : "")
        << "\n|---------:|------:|"
        << (with_view ? "-----:|" : "")
        << (with_matched ? "--------:|" : "")
        << "---------------:|------:|---------------:|------:|----------:|"
        << (with_memory ? "--------:|--------:|" : "")
        << "\n" << std::flush;
}

void Report::row(const Row& row)
{
    // A difference within err% of both libraries can be noise, so it gets no color.
    const double ratio = row.zecs.ns / row.entt.ns;
    const double difference = std::max(COLOR_MIN_DIFFERENCE, row.entt.error + row.zecs.error);
    const char* color = nullptr;
    if (colors && ratio < 1.0 - difference)
        color = GREEN;
    else if (colors && ratio > 1.0 + difference)
        color = RED;

    if (color)
        out << color;

    out << std::format("| {:>7}K | {:>5} |", row.size.entity_count / 1000, row.size.comp_count);

    if (with_view)
        out << std::format(" {:>4} |", row.view);
    if (with_matched)
        out << std::format(" {:>7} |", row.matched);

    out << std::format(" {:>14.0f} | {:>4.1f}% | {:>14.0f} | {:>4.1f}% | {:>8.1f}% |",
        row.entt.ns, row.entt.error * 100.0, row.zecs.ns, row.zecs.error * 100.0, ratio * 100.0);

    constexpr double MB = 1024.0 * 1024.0;
    if (with_memory)
        out << std::format(" {:>7.1f} | {:>7.1f} |", double(row.bytes->entt) / MB, double(row.bytes->zecs) / MB);

    if (color)
        out << RESET;

    out << "\n" << std::flush;
}
