cmake_minimum_required(VERSION 3.25)

project("zecs_bench" VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Without a build type, Ninja and Makefiles build with no optimizations.
# Such numbers can't be used, so Release is the default.
# Visual Studio and Xcode choose the build type at build time.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    message(STATUS "zecs_bench: no build type is set, so Release is used")
endif()

# RelWithDebInfo builds the same code as Release and adds debug info.
# Then you can step through the assembly of the code that is measured.
# The CMake default isn't like this. gcc and clang use -O2, not -O3.
# MSVC inlines less code (/Ob1) and links incrementally, so calls go
# through extra jumps.
# These lines stand before add_subdirectory(zecs), so zecs gets the same flags.
# MSVC gets /Zi from CMAKE_MSVC_DEBUG_INFORMATION_FORMAT (policy CMP0141).
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")
if(MSVC)
    set(ZECS_BENCH_RELWITHDEBINFO_LINK "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${ZECS_BENCH_RELWITHDEBINFO_LINK}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${ZECS_BENCH_RELWITHDEBINFO_LINK}")
else()
    string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -g")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Takes all sources from src/ and its subfolders.
file(GLOB_RECURSE BENCH_SOURCES CONFIGURE_DEPENDS 
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.h")

source_group("src" FILES ${BENCH_SOURCES})

# entt and picobench are header-only, so they are interface libraries.
add_subdirectory(ext)
add_subdirectory(zecs)

add_executable(${PROJECT_NAME} ${BENCH_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE picobench entt zecs)

# perf_ecs.cpp has many entt and zecs templates. Without /bigobj its object
# file has more sections than the COFF format allows.
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif()

# Writes assembly listings next to the object files: .s with gcc and clang,
# .cod with MSVC. With them you can compare the code that the compiler makes
# for zecs and entt, and you don't need to run objdump by hand.
option(ZECS_BENCH_GENERATE_ASM "Emit assembly listings (.s) next to object files" OFF)
if(ZECS_BENCH_GENERATE_ASM)
    if(MSVC)
        set(ZECS_BENCH_ASM_FLAGS /FAcs)
    else()
        set(ZECS_BENCH_ASM_FLAGS -save-temps=obj -fverbose-asm)
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE ${ZECS_BENCH_ASM_FLAGS})
    target_compile_options(zecs PRIVATE ${ZECS_BENCH_ASM_FLAGS})
endif()
