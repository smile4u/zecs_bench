#pragma once

#include <cstdint>

// Bytes that are allocated with operator new and not freed yet.
// memory.cpp replaces the global operator new and operator delete to count them.
// Both entt and zecs take their memory there.
//
// Why not the memory of the process: the heap keeps the pages of the worlds
// that are already destroyed. Then almost all memory goes to the world
// that is created second, and the order of the worlds changes the result.
//
// Limit: only memory allocated inside the bench executable is counted.
// A zecs built as a DLL is not counted.
uint64_t allocated_bytes();
