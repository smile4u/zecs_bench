#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>

// The size of a world. fill() gives entity i the components
// TestComp<1>..TestComp<i % comp_count>.
struct WorldSize
{
    size_t entity_count;
    size_t comp_count;
};

// Every scenario runs on worlds of all these sizes, in this order.
inline constexpr WorldSize WORLD_SIZES[] = {
    { 10000, 10 }, { 10000, 25 }, { 10000, 50 }, { 10000, 100 },
    { 50000, 10 }, { 50000, 25 }, { 50000, 50 }, { 50000, 100 },
    { 200000, 10 }, { 200000, 25 }, { 200000, 50 }, { 200000, 100 },
};

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
