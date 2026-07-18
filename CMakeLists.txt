cmake_minimum_required(VERSION 3.25)

project("zecs_bench" VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Рекурсивно собираем все исходники
file(GLOB_RECURSE BENCH_SOURCES CONFIGURE_DEPENDS 
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.h")

source_group("src" FILES ${BENCH_SOURCES})

# Интерфейсные библиотеки entt, nb (nanobench) и picobench
add_subdirectory(ext)
add_subdirectory(zecs)

add_executable(${PROJECT_NAME} ${BENCH_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE nb picobench entt zecs)
