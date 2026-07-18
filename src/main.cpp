#include <entt/entt.hpp>

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench/nanobench.h>

int main()
{
    entt::registry registry;

    ankerl::nanobench::Bench().run("create + destroy", [&] {
        auto entity = registry.create();
        registry.destroy(entity);
    });

    return 0;
}
