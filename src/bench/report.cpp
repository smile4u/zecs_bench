#include "bench/report.h"

#include <algorithm>
#include <format>
#include <ostream>
#include <sstream>
#include <string>

#include "bench/environment.h"
#include "bench/runner.h"

namespace
{
// A difference smaller than this is too small to show it with a color.
// tools/plot.py has the same value.
constexpr double COLOR_MIN_DIFFERENCE = 0.1;

const char* const GREEN = "\x1b[32m";
const char* const RED = "\x1b[31m";
const char* const RESET = "\x1b[0m";

// A number for a CSV line, or an empty field when the table has no such column.
std::string csv_field(bool has, uint64_t value)
{
    return has ? std::to_string(value) : std::string();
}
} // namespace

Report::Report(std::ostream& out, std::ostream* csv, bool colors)
    : out(out)
    , csv(csv)
    , colors(colors)
{
}

void Report::legend()
{
    const std::string environment = environment_text();
    if (csv)
    {
        std::istringstream lines(environment);
        for (std::string line; std::getline(lines, line);)
            *csv << "# " << line << "\n";
        *csv << "table,entities,comps,view,matched,entt_ns,entt_error,zecs_ns,zecs_error,entt_bytes,zecs_bytes\n" << std::flush;
    }

    out << "# zecs vs entt on " << platform_text() << "\n\n"
        << environment << "\n"
        << "comps: entity i gets TestComp<1>..TestComp<i % comps>. Every fifth entity also gets\n"
        << "the components of the view of one query. TestComp<N> with odd N is small (" << sizeof(TestCompSmall) << " bytes),\n"
        << "and with even N it is big (" << sizeof(TestCompBig) << " bytes, with std::string).\n"
        << "small or big in the name of a table: the kind of the component that the loop body reads and changes.\n"
        << "The body adds 1 to value of a small component. In a big one it also adds 1 to all 16 numbers\n"
        << "of its array, so it uses all cache lines of the component.\n"
        << "view: the number of components in the view. matched: the number of entities in the view.\n"
        << "ns/op: the median of " << SAMPLES << " samples.\n"
        << "err%: half of the samples are within this distance from the median.\n"
        << "zecs vs entt: how much more (+) or less (-) time zecs takes than entt.\n";

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
    table = name;

    out << std::format("\n## {}\n\n{}\n\n", name, about)
        << "| entities | comps |"
        << (with_view ? " view |" : "")
        << (with_matched ? " matched |" : "")
        << "     entt ns/op |  err% |     zecs ns/op |  err% | zecs vs entt |"
        << (with_memory ? " entt MB | zecs MB |" : "")
        << "\n|---------:|------:|"
        << (with_view ? "-----:|" : "")
        << (with_matched ? "--------:|" : "")
        << "---------------:|------:|---------------:|------:|-------------:|"
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

    out << std::format(" {:>14.0f} | {:>4.1f}% | {:>14.0f} | {:>4.1f}% | {:>+11.1f}% |",
        row.entt.ns, row.entt.error * 100.0, row.zecs.ns, row.zecs.error * 100.0, (ratio - 1.0) * 100.0);

    constexpr double MB = 1024.0 * 1024.0;
    if (with_memory)
        out << std::format(" {:>7.1f} | {:>7.1f} |", double(row.bytes->entt) / MB, double(row.bytes->zecs) / MB);

    if (color)
        out << RESET;

    out << "\n" << std::flush;

    if (csv)
    {
        const WorldBytes bytes = with_memory ? *row.bytes : WorldBytes{};
        *csv << std::format("{},{},{},{},{},{:.1f},{:.4f},{:.1f},{:.4f},{},{}\n",
            table, row.size.entity_count, row.size.comp_count,
            csv_field(with_view, row.view), csv_field(with_matched, row.matched),
            row.entt.ns, row.entt.error, row.zecs.ns, row.zecs.error,
            csv_field(with_memory, bytes.entt), csv_field(with_memory, bytes.zecs))
             << std::flush;
    }
}
