#pragma once

#include <cstddef>

// Adds entity_count entities to the world. Entity i gets the components
// TestComp<1>..TestComp<i % comp_count>. Every fifth entity also gets
// the components of the view of one query (see queries.h).
// WrapperT is TestWorldEntt or TestWorldZecs.
template<typename WrapperT>
void fill(WrapperT& wrapper, size_t entity_count, size_t comp_count);
