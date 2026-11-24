#!/usr/bin/env python3
"""Draws the summary chart of zecs_bench for every CSV file.

    python tools/plot.py results/*.csv

Every CSV file is the result of one machine (zecs_bench --csv <file>).
The chart of results/macos.csv goes to results/charts/macos.svg.
It needs matplotlib: pip install matplotlib
"""

import argparse
import csv
import math
import pathlib

import matplotlib

matplotlib.use("svg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402
from matplotlib.patches import Patch  # noqa: E402

# The worlds of the top panels as (entities, comps). Every world gets its own panel.
# In the first one a big component stays in the cache, in the second one it doesn't.
WORLDS = [(50000, 25), (200000, 64)]

# These tables are about a whole world. The bottom panels show them for all worlds.
WORLD_TABLES = ["Create", "Memory", "Remove"]

# The same rule as the colors of the report (COLOR_MIN_DIFFERENCE in
# src/bench/report.cpp). A difference must be more than 10% and more than
# the noise of both libraries. A smaller difference gets grey.
COLOR_MIN_DIFFERENCE = 0.1
GREEN = "#3a9e4f"
RED = "#d0453a"
GREY = "#a0a0a0"

# The height of a row of the chart in inches.
ROW_HEIGHT = 0.22


def read_results(path):
    """Returns the name of the machine and the rows of a CSV file."""
    comments = []
    lines = []
    with open(path, newline="") as file:
        for line in file:
            (comments if line.startswith("#") else lines).append(line)

    # The comments are "Key: value" lines, see environment_text() in the bench.
    info = {}
    for comment in comments:
        key, _, value = comment[1:].strip().partition(": ")
        info[key] = value

    # "Build: <type>, <compiler>, <system> <processor>"
    build = info.get("Build", "").split(", ")
    compiler = build[1] if len(build) > 2 else ""
    machine = f"{build[-1]}, {info.get('CPU', 'unknown CPU')}, {compiler}"
    return machine, list(csv.DictReader(lines))


def world_bars(rows, world):
    """Returns (label, zecs/entt, noise) for the rows of the world with queries, in the order of the CSV."""
    entities, comps = world
    return [(row["table"] + (f", view {row['view']}" if row["view"] else ""),
             float(row["zecs_ns"]) / float(row["entt_ns"]),
             float(row["entt_error"]) + float(row["zecs_error"]))
            for row in rows
            if (int(row["entities"]), int(row["comps"])) == world and row["table"] not in WORLD_TABLES]


def table_bars(rows, table):
    """Returns (label, zecs/entt, noise) for every world of a table of WORLD_TABLES."""
    bars = []
    for row in rows:
        label = f"{int(row['entities']) // 1000}K, comps {row['comps']}"
        if table == "Memory" and row["entt_bytes"]:
            # The memory doesn't change from run to run, so it has no noise.
            bars.append((label, int(row["zecs_bytes"]) / int(row["entt_bytes"]), 0.0))
        elif row["table"] == table:
            bars.append((label, float(row["zecs_ns"]) / float(row["entt_ns"]),
                         float(row["entt_error"]) + float(row["zecs_error"])))
    return bars


def difference_text(ratio):
    """zecs/entt as the difference: 1.46 is "+46%", and 0.33 is "-67%"."""
    difference = round((ratio - 1.0) * 100.0)
    return f"{difference:+d}%" if difference != 0 else "0%"


def bar_color(ratio, noise):
    difference = max(COLOR_MIN_DIFFERENCE, noise)
    if ratio < 1.0 - difference:
        return GREEN
    if ratio > 1.0 + difference:
        return RED
    return GREY


def draw_panel(axis, title, bars):
    """Draws the bars on a panel and returns the length of the longest bar with its error bar."""
    # The axis is log2(zecs/entt), so "two times faster" (-50%) and "two times
    # slower" (+100%) have bars of the same length.
    longest = 0.0
    for i, (_, ratio, noise) in enumerate(bars):
        # The error bar is the noise of both libraries, the same value that
        # makes a bar grey. On the log2 axis it is log2(1 + noise).
        x = math.log2(ratio)
        error = math.log2(1.0 + noise)
        axis.barh(i, x, height=0.75, color=bar_color(ratio, noise),
                  xerr=error if noise > 0 else None,
                  error_kw={"ecolor": "black", "elinewidth": 0.8, "capsize": 2})
        end = abs(x) + error + 0.04
        axis.text(end if x >= 0 else -end, i, difference_text(ratio),
                  va="center", ha="left" if x >= 0 else "right", fontsize=7)
        longest = max(longest, abs(x) + error)

    axis.axvline(0, color="black", linewidth=0.8)
    axis.set_title(title, fontsize=11)
    axis.grid(axis="x", color="#e0e0e0", linewidth=0.6)
    axis.set_axisbelow(True)
    axis.set_yticks(range(len(bars)), [label for label, _, _ in bars], fontsize=9)
    axis.set_ylim(len(bars) - 0.5, -0.5)
    return longest


def draw(machine, rows, output):
    top = [world_bars(rows, world) for world in WORLDS]
    bottom = [table_bars(rows, table) for table in WORLD_TABLES]
    top_rows = len(top[0])
    bottom_rows = len(bottom[0])

    # The top panels share the rows of the queries, and the bottom panels
    # share the rows of the worlds. There is room for the titles of the bottom panels.
    figure = plt.figure(figsize=(14, ROW_HEIGHT * (top_rows + bottom_rows) + 2.6))
    grid = figure.add_gridspec(2, 6, height_ratios=[top_rows, bottom_rows + 2])
    top_axes = [figure.add_subplot(grid[0, 0:3]), figure.add_subplot(grid[0, 3:6])]
    bottom_axes = [figure.add_subplot(grid[1, 0:2]), figure.add_subplot(grid[1, 2:4]), figure.add_subplot(grid[1, 4:6])]

    limit = 0.0
    for axis, world, bars in zip(top_axes, WORLDS, top):
        limit = max(limit, draw_panel(axis, f"{world[0] // 1000}K entities, comps {world[1]}", bars))
    for axis, table, bars in zip(bottom_axes, WORLD_TABLES, bottom):
        limit = max(limit, draw_panel(axis, table, bars))

    # The same scale on all panels. The ticks are powers of two, and there is
    # room after the longest bar for its label. Only the first panel in a line
    # has the labels of the rows.
    ticks = range(-math.ceil(limit), math.ceil(limit) + 1)
    for axis in top_axes + bottom_axes:
        axis.set_xlim(-(limit + 0.35), limit + 0.35)
        axis.set_xticks(list(ticks), [difference_text(2.0 ** t) for t in ticks], fontsize=8)
    for axis in top_axes[1:] + bottom_axes[1:]:
        axis.tick_params(labelleft=False)

    # The machine in big letters at the top, the title under it,
    # the color legend at the bottom. The rooms are in inches.
    figure_height = figure.get_figheight()
    top_room = 0.75
    bottom_room = 0.55
    figure.text(0.5, 1.0, machine, ha="center", va="top", fontsize=15)
    figure.text(0.5, 1.0 - 0.38 / figure_height,
                "How much more (+) or less (-) time or memory zecs takes than entt. Left of 0: zecs is better.",
                ha="center", va="top", fontsize=11)
    figure.legend(handles=[
        Line2D([], [], color="black", linewidth=0.8, marker="|", markersize=8,
               label="the noise: err% of entt and zecs together"),
        Patch(color=GREEN, label="zecs is faster or uses less memory"),
        Patch(color=RED, label="entt is faster or uses less memory"),
        Patch(color=GREY, label="the difference is within 10% or within the noise"),
    ], loc="lower center", ncol=2, frameon=False, fontsize=9)
    figure.tight_layout(rect=(0, bottom_room / figure_height, 1, 1.0 - top_room / figure_height))

    output.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(output, bbox_inches="tight", metadata={"Date": None})
    plt.close(figure)
    print(f"{output} is written")


def main():
    parser = argparse.ArgumentParser(description="Draws the summary chart of zecs_bench for every CSV file.")
    parser.add_argument("csv", nargs="+", help="CSV files of zecs_bench, one for a machine")
    parser.add_argument("-o", "--output-dir", default="results/charts")
    args = parser.parse_args()

    # Without it the SVG gets new ids on every run, and git sees changes
    # in a chart that didn't change.
    plt.rcParams["svg.hashsalt"] = "zecs_bench"

    for path in args.csv:
        machine, rows = read_results(path)
        draw(machine, rows, pathlib.Path(args.output_dir) / (pathlib.Path(path).stem + ".svg"))


if __name__ == "__main__":
    main()
