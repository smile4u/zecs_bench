#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>

#include "wrapper/test_comps.h"

// The size of a world. fill() gives entity i the components
// TestComp<1>..TestComp<i % comp_count>.
struct WorldSize
{
    size_t entity_count;
    size_t comp_count;
};

// Every scenario runs on worlds of all these sizes, in this order.
inline constexpr WorldSize WORLD_SIZES[] = {
    { 10000, 10 }, { 10000, 25 }, { 10000, 50 }, { 10000, 64 },
    { 50000, 10 }, { 50000, 25 }, { 50000, 50 }, { 50000, 64 },
    { 200000, 10 }, { 200000, 25 }, { 200000, 50 }, { 200000, 64 },
};

// fill() takes components from TestComp<1> to TestComp<comp_count - 1>.
// The assert in entity_fill() is off in Release, so the sizes are checked here.
static_assert(std::ranges::all_of(WORLD_SIZES, [](const WorldSize& size) { return size.comp_count - 1 <= TEST_COMPS_MAX; }));

// The memory of the two worlds of one size. It is what operator new gave
// while a world was made and filled (see memory.h).
struct WorldBytes
{
    uint64_t entt = 0;
    uint64_t zecs = 0;

    bool operator==(const WorldBytes&) const = default;
};

// Entry [i] is the memory of the worlds of WORLD_SIZES[i].
using SizeBytes = std::array<WorldBytes, std::size(WORLD_SIZES)>;

// It gives the size as text for error messages.
inline std::string size_text(const WorldSize& size)
{
    return std::format("entities {}K, comps {}", size.entity_count / 1000, size.comp_count);
}
