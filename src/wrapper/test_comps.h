#pragma once

#include <array>
#include <cstddef>
#include <string>

struct TestCompBase
{
    int value;
    std::string s;
    std::array<size_t, 16> arr;
};

template<size_t N>
struct TestComp : TestCompBase
{
    inline TestComp() { value = N; };
};

// The largest N of TestComp<N>. The tables in world_entt.cpp and
// world_zecs.cpp have an entry for every N from 0 to this value.
inline constexpr size_t TEST_COMPS_MAX = 100;
