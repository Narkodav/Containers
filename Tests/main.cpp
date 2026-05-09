#include <chrono>
#include <iostream>
#include <new>
#include <cstddef>
#include <vector>

#include "Utilities/Concepts.h"
#include "TestFramework.h"

constexpr size_t ARRAY_SIZE = 100'000;

struct NonTrivial
{
    const int g = 1;
    int x;
    NonTrivial() : x(42) {}
    NonTrivial(int d) : x(d) {}
    NonTrivial(const NonTrivial &other) : x(other.x) {}
    NonTrivial(NonTrivial &&other) noexcept : x(other.x) { other.x = 0; }
    NonTrivial &operator=(const NonTrivial &other)
    {
        x = other.x;
        return *this;
    }
    NonTrivial &operator=(NonTrivial &&other) noexcept
    {
        x = other.x;
        other.x = 0;
        return *this;
    }
    ~NonTrivial() { x = -1; }

    bool operator==(const NonTrivial& other) const noexcept {
        return x == other.x;
    }

    
    bool operator!=(const NonTrivial& other) const noexcept {
        return x != other.x;
    }

    struct Hasher {
        inline auto operator()(const NonTrivial& key) const {
            return std::hash<int>()(key.x);
        }
    };
};

template <typename T>
void benchmark_static_array()
{
    auto start = std::chrono::high_resolution_clock::now();

    // allocate and default-construct
    T *arr = new T[ARRAY_SIZE]; // automatically default-constructed

    auto mid = std::chrono::high_resolution_clock::now();

    delete[] arr; // automatically destroyed

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Static array: "
              << std::chrono::duration<double, std::nano>(mid - start).count()
              << " ns construct, "
              << std::chrono::duration<double, std::nano>(end - mid).count()
              << " ns destroy\n";
}

template <typename T>
void benchmark_raw_bytes()
{
    Containers::LifetimeManager<T> life;
    auto start = std::chrono::high_resolution_clock::now();

    // allocate raw memory
    alignas(T) std::byte *buffer = new std::byte[ARRAY_SIZE * sizeof(T)];
    T *arr = reinterpret_cast<T *>(buffer);

    // manually default-construct
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
        life.construct(arr + i);

    auto mid = std::chrono::high_resolution_clock::now();

    // manually destroy
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
        life.destroy(arr + i);

    auto end = std::chrono::high_resolution_clock::now();

    delete[] buffer;

    std::cout << "Raw byte array: "
              << std::chrono::duration<double, std::nano>(mid - start).count()
              << " ns construct, "
              << std::chrono::duration<double, std::nano>(end - mid).count()
              << " ns destroy\n";
}

// Benchmark: static typed array on stack
template <typename T>
void benchmark_static_array_stack()
{
    auto start = std::chrono::high_resolution_clock::now();

    T arr[ARRAY_SIZE]; // default-constructed

    auto mid = std::chrono::high_resolution_clock::now();

    // destructor automatically called on stack unwind
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Stack static array: "
              << std::chrono::duration<double, std::nano>(mid - start).count()
              << " ns construct, "
              << std::chrono::duration<double, std::nano>(end - mid).count()
              << " ns destroy\n";
}

// Benchmark: raw byte array on stack + placement new
template <typename T>
void benchmark_raw_bytes_stack()
{
    Containers::LifetimeManager<T> life;
    auto start = std::chrono::high_resolution_clock::now();

    alignas(T) std::byte buffer[ARRAY_SIZE * sizeof(T)];
    T *arr = reinterpret_cast<T *>(buffer);

    // manually default-construct
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
        life.construct(arr + i);

    auto mid = std::chrono::high_resolution_clock::now();

    // manually destroy
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
        life.destroy(arr + i);

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Stack raw byte array: "
              << std::chrono::duration<double, std::nano>(mid - start).count()
              << " ns construct, "
              << std::chrono::duration<double, std::nano>(end - mid).count()
              << " ns destroy\n";
}

#include "PointerStorage/Vector.h"
#include "PointerStorage/Array.h"
#include "PointerStorage/ManualVector.h"
#include "Utilities/ReusableStorage.h"
#include "Lists/BidirectionalList.h"
#include "Sets/UnorderedSet.h"
#include "Maps/UnorderedMap.h"
#include "ContainerBenchmarker.h"
#include "ContainerBenchmarker.h"
#include "Memory/ExternalMetadataAllocators/BuddyAllocator.h"
#include "Memory/ExternalMetadataAllocators/ListAllocator.h"

#include <array>
#include <chrono>
#include <iostream>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>

template <typename Func>
auto benchmark(Func f, size_t iterations = 1000000)
{
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        f();
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::nano>(end - start).count() / iterations;
}

#include "Utilities/TemplateUnion.h"

union Un
{
    std::vector<int> vec;
    std::array<std::byte, sizeof(std::vector<int>)> bytes;
};

template<typename T>
struct Wrapper {
    union storage_t {
        T t;
    } storage_;

    // Conditionally trivial default constructor
    Wrapper() = default;
    Wrapper() requires (!std::is_trivially_default_constructible_v<T>) {
        // ... non-trivial initialization logic here if needed
        std::cout << "Non-trivial construction for T\n";
    }
    ~Wrapper() = default;
    ~Wrapper() requires (!std::is_trivially_destructible_v<T>) {
        std::cout << "Non-trivial destructor for T\n";
    }
};

struct Trivial {
    Trivial() = default;
};

// static_assert(std::is_trivially_default_constructible_v<Wrapper<Trivial>>);

// static_assert(std::is_default_constructible_v<Wrapper<std::vector<int>>>);

// static constexpr Containers::TemplateUnion<int, double> var = [](){
//     Containers::TemplateUnion<int, double> v;
//     std::variant<int, double> vs;
//     vs.emplace<1>(1);
//     v.set<0>(1);

//     //auto& bytes = v.getBytes();
//     return v;
// }();

int main()
{
    TestFramework::runAllTests();

    // Memory::ExternalMetadataAllocators::ListAllocatorBase alloc;
    // alloc.assign(0, 1024);

    // auto ptr1 = alloc.allocate(123);


    // Memory::ExternalMetadataAllocators::BuddyBlockAllocator::Tree<uint64_t> tree;

    // tree.init(12);

    // tree.set(10, 123, Memory::ExternalMetadataAllocators::BuddyBlockAllocator::Tree<uint64_t>::NodeState::Free);
    // tree.set(10, 122, Memory::ExternalMetadataAllocators::BuddyBlockAllocator::Tree<uint64_t>::NodeState::Free);
    // tree.set(10, 120, Memory::ExternalMetadataAllocators::BuddyBlockAllocator::Tree<uint64_t>::NodeState::Free);
    // auto index = tree.findFreeAtLevel(0);

    // if(index != 123) __debugbreak();

    
    

    return 0;
}
