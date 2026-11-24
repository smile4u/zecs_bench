#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <type_traits>

// Every component has `value`. The scenarios change it, and mutation_sum() adds it up.
struct TestCompBase
{
    int value;
};

// A small component. A pool can copy it as bytes, and it has no destructor.
struct TestCompSmall : TestCompBase
{
};

// A big component. A pool must call its constructors and its destructor
// because of std::string.
struct TestCompBig : TestCompBase
{
    std::string s;
    std::array<size_t, 16> arr;
};

// Odd N gives a small component, and even N gives a big one. Entity i gets
// TestComp<1>..TestComp<k>, so it has almost the same number of both kinds.
template<size_t N>
struct TestComp : std::conditional_t<N % 2 == 1, TestCompSmall, TestCompBig>
{
    TestComp() { this->value = N; }
};

static_assert(std::is_trivially_copyable_v<TestComp<1>> && std::is_trivially_destructible_v<TestComp<1>>);
static_assert(!std::is_trivially_copyable_v<TestComp<2>> && !std::is_trivially_destructible_v<TestComp<2>>);

// The largest N of TestComp<N>. The tables in world_entt.cpp and
// world_zecs.cpp have an entry for every N from 0 to this value.
inline constexpr size_t TEST_COMPS_MAX = 64;
