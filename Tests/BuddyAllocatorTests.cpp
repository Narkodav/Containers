#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <limits>

#include "TestFramework.h"
#include "Containers/Memory/ExternalMetadataAllocators/BuddyAllocator.h"

using namespace Memory::ExternalMetadataAllocators;

// ------------------------------------------------------------
// Basic alignment / sizing tests
// ------------------------------------------------------------

__TEST__(BuddyAllocator_AlignHeapSize)
{
    __ASSERT_EQ__(64ull, BuddyAllocatorBase::alignHeapSize(0));
    __ASSERT_EQ__(64ull, BuddyAllocatorBase::alignHeapSize(1));
    __ASSERT_EQ__(64ull, BuddyAllocatorBase::alignHeapSize(64));
    __ASSERT_EQ__(128ull, BuddyAllocatorBase::alignHeapSize(65));
    __ASSERT_EQ__(128ull, BuddyAllocatorBase::alignHeapSize(127));
    __ASSERT_EQ__(128ull, BuddyAllocatorBase::alignHeapSize(128));

    __ASSERT_EQ__(1024ull, BuddyAllocatorBase::alignHeapSize(777, 128));
}

__TEST__(BuddyAllocator_InvalidMinBlockSize)
{
    BuddyAllocatorBase allocator;

    bool thrown = false;

    try {
        allocator.assign(0x1000, 1024, 96);
    }
    catch(const std::runtime_error&) {
        thrown = true;
    }

    __ASSERT_TRUE__(thrown);
}

__TEST__(BuddyAllocator_InvalidBlockCount)
{
    BuddyAllocatorBase allocator;

    bool thrown = false;

    try {
        allocator.assign(0x1000, 1024 + 64, 64);
    }
    catch(const std::runtime_error&) {
        thrown = true;
    }

    __ASSERT_TRUE__(thrown);
}

// ------------------------------------------------------------
// Simple allocation patterns
// ------------------------------------------------------------

__TEST__(BuddyAllocator_SingleAllocation)
{
    constexpr uintptr_t HEAP = 0x100000;
    constexpr size_t SIZE = 4096;

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    uintptr_t ptr = allocator.allocate(1);

    __ASSERT_NE__(std::numeric_limits<uintptr_t>::max(), ptr);
    __ASSERT_EQ__(HEAP, ptr);

    allocator.deallocate(ptr);
}

__TEST__(BuddyAllocator_NonAlignedSizes)
{
    constexpr uintptr_t HEAP = 0x100000;
    constexpr size_t SIZE = 262144; // 2 ^ 18

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    std::vector<uintptr_t> allocations;

    for(size_t size = 1; size < 500; ++size)
    {
        uintptr_t ptr = allocator.allocate(size);

        __ASSERT_NE__(std::numeric_limits<uintptr_t>::max(), ptr);
        __ASSERT_EQ__(0ull, (ptr - HEAP) % 64ull);

        allocations.push_back(ptr);
    }

    for(auto ptr : allocations)
        allocator.deallocate(ptr);
}

__TEST__(BuddyAllocator_FillHeap)
{
    constexpr uintptr_t HEAP = 0x200000;
    constexpr size_t SIZE = 4096;

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    std::vector<uintptr_t> allocations;

    while(true)
    {
        auto ptr = allocator.allocate(64);

        if(ptr == std::numeric_limits<uintptr_t>::max())
            break;

        allocations.push_back(ptr);
    }

    __ASSERT_EQ__(SIZE / 64, allocations.size());

    for(auto ptr : allocations)
        allocator.deallocate(ptr);
}

__TEST__(BuddyAllocator_AllocateTooLarge)
{
    constexpr uintptr_t HEAP = 0x300000;
    constexpr size_t SIZE = 4096;

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    uintptr_t ptr = allocator.allocate(SIZE + 1);

    __ASSERT_EQ__(std::numeric_limits<uintptr_t>::max(), ptr);
}

// ------------------------------------------------------------
// Fragmentation / weird history tests
// ------------------------------------------------------------

__TEST__(BuddyAllocator_FragmentationPattern)
{
    constexpr uintptr_t HEAP = 0x400000;
    constexpr size_t SIZE = 8192;

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    std::vector<uintptr_t> ptrs;

    for(size_t i = 0; i < 32; ++i)
    {
        auto ptr = allocator.allocate(64);
        __ASSERT_NE__(std::numeric_limits<uintptr_t>::max(), ptr);

        ptrs.push_back(ptr);
    }

    // Free every second allocation
    for(size_t i = 0; i < ptrs.size(); i += 2)
        allocator.deallocate(ptrs[i]);

    // Allocate larger blocks
    std::vector<uintptr_t> large;

    for(size_t i = 0; i < 8; ++i)
    {
        auto ptr = allocator.allocate(128);

        if(ptr != std::numeric_limits<uintptr_t>::max())
            large.push_back(ptr);
    }

    for(size_t i = 1; i < ptrs.size(); i += 2)
        allocator.deallocate(ptrs[i]);

    for(auto ptr : large)
        allocator.deallocate(ptr);
}

__TEST__(BuddyAllocator_MergeAfterFree)
{
    constexpr uintptr_t HEAP = 0x500000;
    constexpr size_t SIZE = 4096;

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    auto a = allocator.allocate(64);
    auto b = allocator.allocate(64);

    __ASSERT_NE__(std::numeric_limits<uintptr_t>::max(), a);
    __ASSERT_NE__(std::numeric_limits<uintptr_t>::max(), b);

    allocator.deallocate(a);
    allocator.deallocate(b);

    // Should merge into larger block
    auto big = allocator.allocate(128);

    __ASSERT_NE__(std::numeric_limits<uintptr_t>::max(), big);

    allocator.deallocate(big);
}

__TEST__(BuddyAllocator_AllocateFreeAllocateDifferentSizes)
{
    constexpr uintptr_t HEAP = 0x600000;
    constexpr size_t SIZE = 16384;

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    auto a = allocator.allocate(70);
    auto b = allocator.allocate(130);
    auto c = allocator.allocate(500);
    auto d = allocator.allocate(33);

    allocator.deallocate(b);
    allocator.deallocate(d);

    auto e = allocator.allocate(64);
    auto f = allocator.allocate(256);

    __ASSERT_NE__(std::numeric_limits<uintptr_t>::max(), e);
    __ASSERT_NE__(std::numeric_limits<uintptr_t>::max(), f);

    allocator.deallocate(a);
    allocator.deallocate(c);
    allocator.deallocate(e);
    allocator.deallocate(f);
}

// ------------------------------------------------------------
// Stress test
// ------------------------------------------------------------

__TEST__(BuddyAllocator_StressRandomized)
{
    constexpr uintptr_t HEAP = 0x700000;
    constexpr size_t SIZE = 1 << 20; // 1MB

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    struct Allocation
    {
        uintptr_t ptr;
        size_t size;
    };

    std::vector<Allocation> active;

    std::mt19937 rng(1337);

    std::uniform_int_distribution<int> opDist(0, 99);
    std::uniform_int_distribution<int> sizeDist(1, 4096);

    constexpr size_t OPERATIONS = 100000;

    for(size_t i = 0; i < OPERATIONS; ++i)
    {
        bool doAlloc = active.empty() || opDist(rng) < 70;

        if(doAlloc)
        {
            size_t size = static_cast<size_t>(sizeDist(rng));

            uintptr_t ptr = allocator.allocate(size);

            if(ptr != std::numeric_limits<uintptr_t>::max())
            {
                // alignment
                __ASSERT_EQ__(0ull, (ptr - HEAP) % 64ull);

                // overlap check
                for(const auto& alloc : active)
                {
                    uintptr_t a0 = ptr;
                    uintptr_t a1 = ptr + size;

                    uintptr_t b0 = alloc.ptr;
                    uintptr_t b1 = alloc.ptr + alloc.size;

                    bool overlap = !(a1 <= b0 || a0 >= b1);

                    __ASSERT_FALSE__(overlap);
                }

                active.push_back({ptr, size});
            }
        }
        else
        {
            std::uniform_int_distribution<size_t> freeDist(0, active.size() - 1);

            size_t idx = freeDist(rng);

            allocator.deallocate(active[idx].ptr);

            active.erase(active.begin() + idx);
        }
    }

    std::shuffle(active.begin(), active.end(), rng);

    for(const auto& alloc : active)
        allocator.deallocate(alloc.ptr);
}

// ------------------------------------------------------------
// Exhaustive weird-size pattern
// ------------------------------------------------------------

__TEST__(BuddyAllocator_WeirdAllocationHistory)
{
    constexpr uintptr_t HEAP = 0x800000;
    constexpr size_t SIZE = 32768;

    BuddyAllocatorBase allocator;
    allocator.assign(HEAP, SIZE, 64);

    std::vector<uintptr_t> ptrs;

    // Prime-ish awkward sizes
    const std::vector<size_t> sizes = {
        3, 5, 7, 11, 13, 17,
        31, 67, 127, 251,
        509, 1021
    };

    for(size_t round = 0; round < 20; ++round)
    {
        ptrs.clear();

        for(auto size : sizes)
        {
            auto ptr = allocator.allocate(size);

            if(ptr != std::numeric_limits<uintptr_t>::max())
                ptrs.push_back(ptr);
        }

        // weird free order
        for(size_t i = ptrs.size(); i > 0; --i)
        {
            size_t idx = (i * 7) % ptrs.size();
            allocator.deallocate(ptrs[idx]);
        }
    }
}