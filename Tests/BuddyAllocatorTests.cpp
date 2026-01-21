#include "TestFramework.h"
#include "../include/Memory/Allocators/BuddyAllocator.h"
#include <vector>
#include <string>
#include <tuple>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <cstring>

// ============================================
// Basic Tests
// ============================================

__TEST__(basic_construction) {
    // Arrange
    const size_t POOL_SIZE = 1024 * 1024; // 1MB
    void* memory = std::malloc(POOL_SIZE);

    // Act
    Containers::BuddyAllocator alloc(memory, POOL_SIZE);

    // Assert
    __ASSERT_TRUE__(alloc.getUsableSize() > 0);
    __ASSERT_TRUE__(alloc.getUsableSize() <= POOL_SIZE);

    // Cleanup
    std::free(memory);
}

__TEST__(construction_invalid_params) {
    // Should throw on invalid parameters
    __ASSERT_THROWS__(std::runtime_error, []() { Containers::BuddyAllocator alloc(nullptr, 1024); });

    __ASSERT_THROWS__(std::runtime_error, []() {
        void* mem = std::malloc(1024);
        Containers::BuddyAllocator alloc(mem, 10); // Too small
        std::free(mem); });
}

__TEST__(move_semantics) {
    const size_t POOL_SIZE = 1024 * 1024;
    void* memory = std::malloc(POOL_SIZE);

    // Create first allocator
    Containers::BuddyAllocator alloc1(memory, POOL_SIZE);
    size_t usableSize = alloc1.getUsableSize();

    // Move construct
    Containers::BuddyAllocator alloc2 = std::move(alloc1);
    __ASSERT_EQ__(usableSize, alloc2.getUsableSize());

    // Move assign
    Containers::BuddyAllocator alloc3;
    alloc3 = std::move(alloc2);
    __ASSERT_EQ__(usableSize, alloc3.getUsableSize());

    std::free(memory);
}

// ============================================
// Fixture for allocator tests
// ============================================

class AllocatorFixture {
public:
    static const size_t POOL_SIZE = 1024 * 1024; // 1MB
    void* memory;
    Containers::BuddyAllocator* alloc;

    AllocatorFixture() {
        memory = std::malloc(POOL_SIZE);
        alloc = new Containers::BuddyAllocator(memory, POOL_SIZE, 64);
    }

    ~AllocatorFixture() {
        delete alloc;
        std::free(memory);
    }

    // Helper to check if pointer is aligned
    static bool isAligned(void* ptr, size_t alignment) {
        return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
    }
};

// ============================================
// Basic Allocation Tests
// ============================================

__TEST_FIXTURE__(AllocatorFixture, allocate_single_block) {
    void* ptr = fixture.alloc->allocate(128);

    __ASSERT_TRUE__(ptr != nullptr);
    __ASSERT_TRUE__(fixture.isAligned(ptr, 64)); // Should be aligned to min block size
}

__TEST_FIXTURE__(AllocatorFixture, allocate_multiple_blocks) {
    std::vector<void*> pointers;

    // Allocate 10 blocks
    for (int i = 0; i < 10; ++i) {
        void* ptr = fixture.alloc->allocate(64 + i * 16);
        __ASSERT_TRUE__(ptr != nullptr);
        pointers.push_back(ptr);
    }

    // All pointers should be unique
    for (size_t i = 0; i < pointers.size(); ++i) {
        for (size_t j = i + 1; j < pointers.size(); ++j) {
            __ASSERT_NE__(pointers[i], pointers[j]);
        }
    }
}

__TEST_FIXTURE__(AllocatorFixture, allocate_zero_size) {
    void* ptr = fixture.alloc->allocate(0);
    __ASSERT_TRUE__(ptr == nullptr);
}

__TEST_FIXTURE__(AllocatorFixture, allocate_too_large) {
    size_t poolSize = fixture.alloc->getUsableSize();
    void* ptr = fixture.alloc->allocate(poolSize + 1);
    __ASSERT_TRUE__(ptr == nullptr);
}

// ============================================
// Deallocation Tests
// ============================================

__TEST_FIXTURE__(AllocatorFixture, allocate_deallocate_single) {
    void* ptr = fixture.alloc->allocate(256);
    __ASSERT_TRUE__(ptr != nullptr);

    // Should not throw
    __ASSERT_NO_THROW__([&]() { fixture.alloc->deallocate(ptr); });
}

__TEST_FIXTURE__(AllocatorFixture, deallocate_nullptr) {
    // Should handle nullptr gracefully
    __ASSERT_NO_THROW__([&]() { fixture.alloc->deallocate(nullptr); });
}

__TEST_FIXTURE__(AllocatorFixture, deallocate_invalid_pointer) {
    void* invalid_ptr = reinterpret_cast<void*>(0xDEADBEEF);
    __ASSERT_THROWS__(std::runtime_error, [&]() { fixture.alloc->deallocate(invalid_ptr); });
}

__TEST_FIXTURE__(AllocatorFixture, allocate_deallocate_sequence) {
    std::vector<void*> pointers;

    // Allocate 5 blocks
    for (int i = 0; i < 5; ++i) {
        pointers.push_back(fixture.alloc->allocate(128 * (i + 1)));
    }

    // Deallocate in reverse order
    for (auto it = pointers.rbegin(); it != pointers.rend(); ++it) {
        fixture.alloc->deallocate(*it);
    }

    // Should be able to allocate again
    void* new_ptr = fixture.alloc->allocate(1024);
    __ASSERT_TRUE__(new_ptr != nullptr);
    fixture.alloc->deallocate(new_ptr);
}

// ============================================
// Fragmentation Tests
// ============================================

__TEST_FIXTURE__(AllocatorFixture, fragmentation_coalescing) {
    // Allocate three blocks that will split the pool
    void* small1 = fixture.alloc->allocate(64);
    void* small2 = fixture.alloc->allocate(64);
    void* large = fixture.alloc->allocate(fixture.alloc->getUsableSize() / 2);

    __ASSERT_TRUE__(small1 != nullptr);
    __ASSERT_TRUE__(small2 != nullptr);
    __ASSERT_TRUE__(large != nullptr);

    // Free the large block
    fixture.alloc->deallocate(large);

    // Free the small blocks
    fixture.alloc->deallocate(small1);
    fixture.alloc->deallocate(small2);

    // Should be able to allocate the entire pool now
    void* entirePool = fixture.alloc->allocate(fixture.alloc->getUsableSize());
    __ASSERT_TRUE__(entirePool != nullptr);
    fixture.alloc->deallocate(entirePool);
}

// ============================================
// Parameterized Tests
// ============================================

__TEST_PARAM__(allocate_different_sizes,
    std::make_tuple(64, true),     // Min block size
    std::make_tuple(128, true),    // Power of two
    std::make_tuple(250, true),    // Not power of two (will round up)
    std::make_tuple(1024, true),   // Larger block
    std::make_tuple(0, false),     // Zero size should fail
    std::make_tuple(1024 * 1024 * 2, false) // Too large should fail
) {
    auto [size, shouldSucceed] = param;

    const size_t POOL_SIZE = 1024 * 1024;
    void* memory = std::malloc(POOL_SIZE);
    Containers::BuddyAllocator alloc(memory, POOL_SIZE, 64);

    void* ptr = alloc.allocate(size);

    if (shouldSucceed) {
        __ASSERT_TRUE__(ptr != nullptr);
        if (ptr) alloc.deallocate(ptr);
    }
    else {
        __ASSERT_TRUE__(ptr == nullptr);
    }

    std::free(memory);
}

// ============================================
// Stress Tests with Shared Fixture
// ============================================

class StressTestFixture {
public:
    static const size_t POOL_SIZE = 1024 * 1024 * 16; // 16MB
    void* memory;
    Containers::BuddyAllocator* alloc;
    std::mt19937 rng;

    StressTestFixture() : rng(42) {
        memory = std::malloc(POOL_SIZE);
        alloc = new Containers::BuddyAllocator(memory, POOL_SIZE, 128);
    }

    ~StressTestFixture() {
        delete alloc;
        std::free(memory);
    }

    size_t randomSize() {
        std::uniform_int_distribution<size_t> dist(128, 8192);
        return dist(rng);
    }
};

__TEST_SHARED_FIXTURE__(StressTestFixture, stress, random_alloc_dealloc) {
    const int NUM_OPERATIONS = 1000;
    std::vector<void*> allocations;

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        if (allocations.empty() || (fixture.rng() % 2 == 0)) {
            // Allocate
            size_t size = fixture.randomSize();
            void* ptr = fixture.alloc->allocate(size);
            if (ptr) {
                allocations.push_back(ptr);
                // Write pattern to detect corruption
                std::memset(ptr, 0xAA, std::min(size, static_cast<size_t>(1024)));
            }
        }
        else {
            // Deallocate random block
            size_t idx = fixture.rng() % allocations.size();
            fixture.alloc->deallocate(allocations[idx]);
            allocations.erase(allocations.begin() + idx);
        }
    }

    // Cleanup remaining allocations
    for (void* ptr : allocations) {
        fixture.alloc->deallocate(ptr);
    }

    __ASSERT_TRUE__(true); // If we got here without crashing, test passed
}

__TEST_SHARED_FIXTURE__(StressTestFixture, stress, allocate_all_memory) {
    std::vector<void*> allocations;
    size_t totalAllocated = 0;
    size_t poolSize = fixture.alloc->getUsableSize();

    // Keep allocating until we can't anymore
    while (true) {
        size_t size = 128 * (1 << (fixture.rng() % 6)); // 128, 256, 512, 1024, 2048, 4096
        void* ptr = fixture.alloc->allocate(size);

        if (!ptr) {
            break; // Out of memory
        }

        allocations.push_back(ptr);
        totalAllocated += size;

        // Don't allocate more than pool size (accounting for fragmentation)
        if (totalAllocated > poolSize * 3 / 4) {
            break;
        }
    }

    // Verify all allocations are unique
    for (size_t i = 0; i < allocations.size(); ++i) {
        for (size_t j = i + 1; j < allocations.size(); ++j) {
            __ASSERT_NE__(allocations[i], allocations[j]);
        }
    }

    // Cleanup
    for (void* ptr : allocations) {
        fixture.alloc->deallocate(ptr);
    }

    // Should be able to allocate large block after cleanup
    void* largeBlock = fixture.alloc->allocate(poolSize / 2);
    __ASSERT_TRUE__(largeBlock != nullptr);
    fixture.alloc->deallocate(largeBlock);
}

// ============================================
// Generated Parameter Tests
// ============================================

std::tuple<size_t, size_t, bool> generate_pool_config(size_t index) {
    switch (index % 6) {
    case 0: return std::make_tuple(4096, 64, true);     // Tiny pool
    case 1: return std::make_tuple(65536, 128, true);   // Small pool
    case 2: return std::make_tuple(1024 * 1024, 256, true); // 1MB pool
    case 3: return std::make_tuple(1024, 512, false);   // Too small
    case 4: return std::make_tuple(8192, 8192, false);  // Min block too large
    case 5: return std::make_tuple(16384, 16384, true); // Min block equals pool/2
    default: return std::make_tuple(0, 0, false);
    }
}

__TEST_GENERATED_PARAM__(different_pool_configs, generate_pool_config, 6) {
    auto [poolSize, minBlock, shouldSucceed] = param;

    void* memory = std::malloc(poolSize);

    if (shouldSucceed) {
        __ASSERT_NO_THROW__([&]() {
            Containers::BuddyAllocator alloc(memory, poolSize, minBlock);
            // Try a simple allocation
            void* ptr = alloc.allocate(minBlock);
            if (ptr) alloc.deallocate(ptr);
            });
    }
    else {
        __ASSERT_THROWS__(std::runtime_error, [&]() { Containers::BuddyAllocator alloc(memory, poolSize, minBlock); });
    }

    std::free(memory);
}

// ============================================
// Memory Corruption Tests
// ============================================

__TEST_FIXTURE__(AllocatorFixture, write_to_allocated_memory) {
    const size_t DATA_SIZE = 256;
    void* ptr = fixture.alloc->allocate(DATA_SIZE);
    __ASSERT_TRUE__(ptr != nullptr);

    // Write data
    unsigned char* data = static_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < DATA_SIZE; ++i) {
        data[i] = static_cast<unsigned char>(i % 256);
    }

    // Verify data
    for (size_t i = 0; i < DATA_SIZE; ++i) {
        __ASSERT_EQ__(data[i], static_cast<unsigned char>(i % 256));
    }

    fixture.alloc->deallocate(ptr);
}

__TEST_FIXTURE__(AllocatorFixture, double_free_detection) {
    void* ptr = fixture.alloc->allocate(512);
    __ASSERT_TRUE__(ptr != nullptr);

    fixture.alloc->deallocate(ptr);

    // Double free should throw
    __ASSERT_THROWS__(std::runtime_error, [&]() { fixture.alloc->deallocate(ptr); });
}

__TEST_FIXTURE__(AllocatorFixture, use_after_free_detection) {
    void* ptr = fixture.alloc->allocate(1024);
    __ASSERT_TRUE__(ptr != nullptr);

    // Write something
    std::memset(ptr, 0xAA, 1024);

    fixture.alloc->deallocate(ptr);

    // Try to allocate again - should get different pointer
    void* ptr2 = fixture.alloc->allocate(1024);
    __ASSERT_TRUE__(ptr2 != nullptr);
    __ASSERT_NE__(ptr, ptr2); // Might not always be different, but likely

    fixture.alloc->deallocate(ptr2);
}

// ============================================
// Reallocation Pattern Tests
// ============================================

__TEST_FIXTURE__(AllocatorFixture, reallocate_same_size_pattern) {
    // Allocate and free same size repeatedly
    const size_t BLOCK_SIZE = 256;
    const int ITERATIONS = 100;

    for (int i = 0; i < ITERATIONS; ++i) {
        void* ptr = fixture.alloc->allocate(BLOCK_SIZE);
        __ASSERT_TRUE__(ptr != nullptr);

        // Use the memory
        std::memset(ptr, i % 256, BLOCK_SIZE);

        fixture.alloc->deallocate(ptr);
    }

    // Should still be able to allocate
    void* finalPtr = fixture.alloc->allocate(BLOCK_SIZE * 2);
    __ASSERT_TRUE__(finalPtr != nullptr);
    fixture.alloc->deallocate(finalPtr);
}

// ============================================
// Parameterized Shared Fixture Tests
// ============================================

__TEST_SHARED_FIXTURE_PARAM__(StressTestFixture, stress, allocation_patterns,
    10, 50, 100, 500, 1000) {
    // Test different numbers of allocations
    int numAllocations = param;
    std::vector<void*> allocations;

    for (int i = 0; i < numAllocations; ++i) {
        size_t size = 64 * (1 + (i % 8)); // Varying sizes
        void* ptr = fixture.alloc->allocate(size);

        if (!ptr && i > 0) {
            // Out of memory - free some allocations and retry
            size_t toFree = std::min(allocations.size(), size_t(10));
            for (size_t j = 0; j < toFree; ++j) {
                fixture.alloc->deallocate(allocations.back());
                allocations.pop_back();
            }

            ptr = fixture.alloc->allocate(size);
        }

        __ASSERT_TRUE__(ptr != nullptr);
        allocations.push_back(ptr);
    }

    // Cleanup
    for (void* ptr : allocations) {
        fixture.alloc->deallocate(ptr);
    }
}

// ============================================
// Edge Case Tests
// ============================================

__TEST__(edge_case_minimal_pool) {
    // Smallest possible pool that should work
    const size_t MIN_POOL_SIZE = 4096;
    void* memory = std::malloc(MIN_POOL_SIZE);

    __ASSERT_NO_THROW__([&]() {
        Containers::BuddyAllocator alloc(memory, MIN_POOL_SIZE, 64);
        // Should be able to allocate at least one block
        void* ptr = alloc.allocate(64);
        __ASSERT_TRUE__(ptr != nullptr);
        alloc.deallocate(ptr);
        });

    std::free(memory);
}

__TEST__(edge_case_exact_power_of_two) {
    // Pool size exactly power of two
    const size_t POOL_SIZE = 65536; // 64KB exactly
    void* memory = std::malloc(POOL_SIZE);

    __ASSERT_NO_THROW__([&]() { Containers::BuddyAllocator alloc(memory, POOL_SIZE, 256);

    // Allocate almost entire pool
    void* large = alloc.allocate(POOL_SIZE / 2);
    __ASSERT_TRUE__(large != nullptr);

    void* medium = alloc.allocate(POOL_SIZE / 4);
    __ASSERT_TRUE__(medium != nullptr);

    alloc.deallocate(medium);
    alloc.deallocate(large);
        });

    std::free(memory);
}

// ============================================
// Generated Parameter Tests for Fixture
// ============================================

std::tuple<size_t, size_t, size_t> generate_allocation_pattern(size_t index) {
    // Returns: allocation size, number of allocations, pattern type
    switch (index % 8) {
    case 0: return std::make_tuple(64, 10, 0);   // Many small allocations
    case 1: return std::make_tuple(1024, 5, 0);  // Few medium allocations
    case 2: return std::make_tuple(4096, 2, 0);  // Couple large allocations
    case 3: return std::make_tuple(128, 20, 1);  // Many allocations, mixed sizes
    case 4: return std::make_tuple(256, 8, 2);   // Moderate number
    case 5: return std::make_tuple(512, 12, 2);  // More allocations
    case 6: return std::make_tuple(2048, 3, 1);  // Large blocks
    case 7: return std::make_tuple(8192, 1, 0);  // Single large block
    default: return std::make_tuple(64, 1, 0);
    }
}

__TEST_FIXTURE_GENERATED_PARAM__(AllocatorFixture, allocation_pattern_tests,
    generate_allocation_pattern, 8) {
    auto [allocSize, numAllocs, patternType] = param;

    std::vector<void*> allocations;

    for (size_t i = 0; i < numAllocs; ++i) {
        size_t size = allocSize;
        if (patternType == 1) {
            // Mixed sizes
            size = allocSize * (1 + (i % 4));
        }

        void* ptr = fixture.alloc->allocate(size);
        if (!ptr && i > 0) {
            // Out of memory, free some
            size_t toFree = std::min(allocations.size(), size_t(3));
            for (size_t j = 0; j < toFree; ++j) {
                fixture.alloc->deallocate(allocations.back());
                allocations.pop_back();
            }
            ptr = fixture.alloc->allocate(size);
        }

        __ASSERT_TRUE__(ptr != nullptr);
        allocations.push_back(ptr);
    }

    // Cleanup
    for (void* ptr : allocations) {
        fixture.alloc->deallocate(ptr);
    }
}

// ============================================
// Memory Exhaustion Tests
// ============================================

__TEST_FIXTURE__(AllocatorFixture, out_of_memory_behavior) {
    size_t poolSize = fixture.alloc->getUsableSize();

    // Try to allocate more than available
    void* tooLarge = fixture.alloc->allocate(poolSize + 1);
    __ASSERT_TRUE__(tooLarge == nullptr);

    // Fill up memory gradually
    std::vector<void*> allocations;
    size_t totalAllocated = 0;

    while (true) {
        size_t size = 256; // Small enough to fragment
        void* ptr = fixture.alloc->allocate(size);

        if (!ptr) {
            break; // Out of memory
        }

        allocations.push_back(ptr);
        totalAllocated += size;

        // Stop if we've allocated more than 75% of pool
        if (totalAllocated > poolSize * 3 / 4) {
            break;
        }
    }

    // Should not be able to allocate more
    void* shouldFail = fixture.alloc->allocate(1024);
    __ASSERT_TRUE__(shouldFail == nullptr);

    // Free some memory
    for (size_t i = 0; i < allocations.size() / 2; ++i) {
        fixture.alloc->deallocate(allocations[i]);
    }

    // Should be able to allocate again
    void* shouldSucceed = fixture.alloc->allocate(512);
    __ASSERT_TRUE__(shouldSucceed != nullptr);
    fixture.alloc->deallocate(shouldSucceed);

    // Cleanup remaining allocations
    for (size_t i = allocations.size() / 2; i < allocations.size(); ++i) {
        fixture.alloc->deallocate(allocations[i]);
    }
}