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
    // The work of a loop body with this component.
    void change() { value += 1; }
};

// A big component. A pool must call its constructors and its destructor
// because of std::string.
struct TestCompBig : TestCompBase
{
    std::string s;
    std::array<size_t, 16> arr{};

    // The work of a loop body with this component. It changes all data
    // of the component except the string, so it uses all cache lines of
    // the component. A change of the string can allocate memory, and then
    // the bench would measure the allocator.
    void change()
    {
        value += 1;
        for (size_t& x : arr)
            x += 1;
    }
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

// The kind of TestComp<n> for the report.
constexpr const char* comp_kind(size_t n)
{
    return n % 2 == 1 ? "small" : "big";
}

// The largest N of TestComp<N>. The tables in world_entt.cpp and
// world_zecs.cpp have an entry for every N from 0 to this value.
inline constexpr size_t TEST_COMPS_MAX = 64;
