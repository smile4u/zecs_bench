#include "bench/environment.h"

#include <format>

#include <entt/entt.hpp>
#include <picobench/picobench.hpp>

#if defined(__APPLE__)
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <fstream>
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <cstring>
#include <intrin.h>
#endif

// CMakeLists.txt sets these for this file only.
#ifndef ZECS_BENCH_ZECS_VERSION
#define ZECS_BENCH_ZECS_VERSION "unknown"
#endif
#ifndef ZECS_BENCH_BUILD_TYPE
#define ZECS_BENCH_BUILD_TYPE "unknown"
#endif
#ifndef ZECS_BENCH_SYSTEM
#define ZECS_BENCH_SYSTEM "unknown"
#endif

namespace
{
std::string compiler_name()
{
#if defined(__clang__) && defined(__apple_build_version__)
    return std::format("Apple clang {}", __clang_version__);
#elif defined(__clang__)
    return std::format("clang {}", __clang_version__);
#elif defined(__GNUC__)
    return std::format("gcc {}", __VERSION__);
#elif defined(_MSC_VER)
    return std::format("MSVC {}", _MSC_FULL_VER);
#else
    return "unknown";
#endif
}

std::string cpu_name()
{
#if defined(__APPLE__)
    char name[256] = {};
    size_t size = sizeof(name);
    if (sysctlbyname("machdep.cpu.brand_string", name, &size, nullptr, 0) == 0)
        return name;
#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    for (std::string line; std::getline(cpuinfo, line);)
    {
        if (line.starts_with("model name") && line.find(':') != std::string::npos)
            return line.substr(line.find(':') + 2);
    }
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    // The name is in three CPUID leaves, 16 bytes in each.
    char name[49] = {};
    for (int i = 0; i < 3; ++i)
    {
        int regs[4];
        __cpuid(regs, int(0x80000002u) + i);
        std::memcpy(name + 16 * i, regs, sizeof(regs));
    }
    return name;
#endif
    return "unknown";
}
} // namespace

std::string environment_text()
{
    return std::format("zecs {}, entt {}, picobench {} with picobench.patch\n"
                       "Build: {}, {}, {}\n"
                       "CPU: {}\n",
        ZECS_BENCH_ZECS_VERSION, ENTT_VERSION, PICOBENCH_VERSION_STR,
        ZECS_BENCH_BUILD_TYPE, compiler_name(), ZECS_BENCH_SYSTEM,
        cpu_name());
}
