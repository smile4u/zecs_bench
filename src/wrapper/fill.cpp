#include "wrapper/fill.h"

#include "wrapper/queries.h"
#include "wrapper/world_entt.h"
#include "wrapper/world_zecs.h"

template<typename WrapperT>
void fill(WrapperT& wrapper, size_t entity_count, size_t comp_count)
{
    for (size_t i = 0; i < entity_count; ++i)
    {
        const auto entity = wrapper.entity_add();
        wrapper.entity_fill(entity, i % comp_count);

        // Every fifth entity also gets the components of the view of one query. Without them
        // the wide views are empty when comp_count is small.
        if (i % 5 == 0)
        {
            for (size_t comp : QUERY_INFOS[(i / 5) % QUERY_COUNT].view)
                wrapper.comp_add(entity, comp);
        }
    }
}

template void fill(TestWorldEntt& wrapper, size_t entity_count, size_t comp_count);
template void fill(TestWorldZecs& wrapper, size_t entity_count, size_t comp_count);
