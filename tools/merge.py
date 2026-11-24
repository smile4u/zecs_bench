#!/usr/bin/env python3
"""Merges several CSV files of zecs_bench into one.

    python tools/merge.py run1.csv run2.csv run3.csv -o results/macos.csv

For every row and every library:
- the time is the median of the times of the runs;
- the noise is the spread of these times between the runs, counted in the same way
  as err% of the bench: the median of |t - median| / t. If err% inside the runs is
  bigger, it is the noise.

One run can have a slow row because the system did some other work. The median
doesn't take it, and the noise shows how much the time changes from run to run.
All runs must come from the same build on the same machine.
"""

import argparse
import csv
import io
import statistics
import sys


def read_run(path):
    """Returns the comment lines, the header and the rows of a CSV file."""
    with open(path, newline="") as file:
        lines = file.read().splitlines()
    comments = [line for line in lines if line.startswith("#")]
    reader = csv.reader(line for line in lines if not line.startswith("#"))
    header = next(reader)
    return comments, header, list(reader)


def spread(times):
    """The median of |t - median| / t, as err% of the bench."""
    median = statistics.median(times)
    return statistics.median(abs(t - median) / t for t in times)


def main():
    parser = argparse.ArgumentParser(description="Merges CSV files of zecs_bench.")
    parser.add_argument("csv", nargs="+", help="CSV files of zecs_bench from the same build and machine")
    parser.add_argument("-o", "--output", required=True)
    args = parser.parse_args()

    runs = [read_run(path) for path in args.csv]
    comments, header, rows = runs[0]
    column = {name: header.index(name) for name in header}
    # The values that must be the same in every run. Only the times and their noise can differ.
    fixed = [column[name] for name in ("table", "entities", "comps", "view", "matched", "entt_bytes", "zecs_bytes")]

    for path, (run_comments, run_header, run_rows) in zip(args.csv, runs):
        same_rows = [[row[c] for c in fixed] for row in run_rows] == [[row[c] for c in fixed] for row in rows]
        if run_comments != comments or run_header != header or not same_rows:
            sys.exit(f"Error: {path} isn't from the same build and machine as {args.csv[0]}, or it has other rows")

    merged = []
    for i, row in enumerate(rows):
        row = list(row)
        for library in ("entt", "zecs"):
            times = [float(run_rows[i][column[f"{library}_ns"]]) for _, _, run_rows in runs]
            errors = [float(run_rows[i][column[f"{library}_error"]]) for _, _, run_rows in runs]
            row[column[f"{library}_ns"]] = f"{statistics.median(times):.1f}"
            row[column[f"{library}_error"]] = f"{max(spread(times), statistics.median(errors)):.4f}"
        merged.append(row)

    text = io.StringIO()
    for comment in comments:
        text.write(comment + "\n")
    text.write(f"# Merged from {len(runs)} runs. A time is the median of the runs. err% is the spread "
               f"between the runs, or err% inside the runs if it is bigger.\n")
    writer = csv.writer(text, lineterminator="\n")
    writer.writerow(header)
    writer.writerows(merged)

    with open(args.output, "w", newline="") as file:
        file.write(text.getvalue())
    print(f"{args.output} is written")


if __name__ == "__main__":
    main()
