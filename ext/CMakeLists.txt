cmake_minimum_required(VERSION 3.25)

add_library(picobench INTERFACE)
target_sources(picobench INTERFACE picobench/picobench.hpp)
target_include_directories(picobench INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_library(entt INTERFACE)
target_sources(entt INTERFACE entt/entt.hpp)
target_include_directories(entt INTERFACE ${CMAKE_CURRENT_LIST_DIR})

