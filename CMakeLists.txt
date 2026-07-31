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

# Генерация ассемблерных листингов (.s) рядом с .o — для сравнения кода,
# который компилятор генерит для zecs и entt, без ручного objdump.
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
