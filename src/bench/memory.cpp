#include "bench/memory.h"

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace
{
std::atomic<uint64_t> allocated{ 0 };

// Stands right before the block that the caller gets.
// operator delete does not always know the size, so it is kept here.
struct Header
{
    void* raw;      // what malloc returned
    size_t size;    // how many bytes were asked for
};

void* allocate(size_t size, size_t align)
{
    if (align < alignof(std::max_align_t))
        align = alignof(std::max_align_t);

    // The block is aligned by hand: then one free() works for every block,
    // and no platform function for aligned memory is needed.
    void* raw = std::malloc(size + sizeof(Header) + align);
    if (!raw)
        throw std::bad_alloc();

    const uintptr_t block = (reinterpret_cast<uintptr_t>(raw) + sizeof(Header) + align - 1) & ~(uintptr_t(align) - 1);
    Header* header = reinterpret_cast<Header*>(block) - 1;
    header->raw = raw;
    header->size = size;

    allocated.fetch_add(size, std::memory_order_relaxed);
    return reinterpret_cast<void*>(block);
}

void* allocate_nothrow(size_t size, size_t align) noexcept
{
    try { return allocate(size, align); }
    catch (...) { return nullptr; }
}

void deallocate(void* block) noexcept
{
    if (!block)
        return;

    Header* header = static_cast<Header*>(block) - 1;
    allocated.fetch_sub(header->size, std::memory_order_relaxed);
    std::free(header->raw);
}
} // namespace

uint64_t allocated_bytes()
{
    return allocated.load(std::memory_order_relaxed);
}

// Every form is replaced. Which of them call each other by default
// depends on the standard library.
void* operator new(size_t size) { return allocate(size, 0); }
void* operator new[](size_t size) { return allocate(size, 0); }
void* operator new(size_t size, std::align_val_t align) { return allocate(size, size_t(align)); }
void* operator new[](size_t size, std::align_val_t align) { return allocate(size, size_t(align)); }
void* operator new(size_t size, const std::nothrow_t&) noexcept { return allocate_nothrow(size, 0); }
void* operator new[](size_t size, const std::nothrow_t&) noexcept { return allocate_nothrow(size, 0); }
void* operator new(size_t size, std::align_val_t align, const std::nothrow_t&) noexcept { return allocate_nothrow(size, size_t(align)); }
void* operator new[](size_t size, std::align_val_t align, const std::nothrow_t&) noexcept { return allocate_nothrow(size, size_t(align)); }

void operator delete(void* block) noexcept { deallocate(block); }
void operator delete[](void* block) noexcept { deallocate(block); }
void operator delete(void* block, size_t) noexcept { deallocate(block); }
void operator delete[](void* block, size_t) noexcept { deallocate(block); }
void operator delete(void* block, std::align_val_t) noexcept { deallocate(block); }
void operator delete[](void* block, std::align_val_t) noexcept { deallocate(block); }
void operator delete(void* block, size_t, std::align_val_t) noexcept { deallocate(block); }
void operator delete[](void* block, size_t, std::align_val_t) noexcept { deallocate(block); }
void operator delete(void* block, const std::nothrow_t&) noexcept { deallocate(block); }
void operator delete[](void* block, const std::nothrow_t&) noexcept { deallocate(block); }
void operator delete(void* block, std::align_val_t, const std::nothrow_t&) noexcept { deallocate(block); }
void operator delete[](void* block, std::align_val_t, const std::nothrow_t&) noexcept { deallocate(block); }
