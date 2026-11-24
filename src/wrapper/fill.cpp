#include "wrapper/fill.h"

#include "wrapper/world_entt.h"
#include "wrapper/world_zecs.h"

namespace
{
// The component sets of the five views in run_view() (scenario_view.cpp).
// 0 marks the end of a set.
constexpr size_t VIEW_MATCH_COMPS[][9] = {
    { 1, 5, 0 },
    { 7, 11, 12, 0 },
    { 22, 33, 44, 45, 0 },
    { 3, 17, 23, 33, 43, 0 },
    { 4, 15, 24, 25, 34, 35, 44, 45, 0 },
};
} // namespace

template<typename WrapperT>
void fill(WrapperT& wrapper, size_t entity_count, size_t comp_count)
{
    for (size_t i = 0; i < entity_count; ++i)
    {
        const auto entity = wrapper.entity_add();
        wrapper.entity_fill(entity, i % comp_count);

        // Every fifth entity also gets the components of one view. Without them
        // most views are empty when comp_count < 45. Then the bench measures
        // only the cost of an empty view.
        if (i % 5 == 0)
        {
            const size_t* view_comps = VIEW_MATCH_COMPS[(i / 5) % 5];
            for (size_t k = 0; view_comps[k] != 0; ++k)
                wrapper.comp_add(entity, view_comps[k]);
        }
    }
}

template void fill(TestWorldEntt& wrapper, size_t entity_count, size_t comp_count);
template void fill(TestWorldZecs& wrapper, size_t entity_count, size_t comp_count);
