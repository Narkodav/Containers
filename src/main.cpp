#include <chrono>
#include <iostream>
#include <new>
#include <cstddef>
#include <vector>

#include "../include/Utilities/Concepts.h"

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

#include "../include/PointerStorage/Vector.h"
#include "../include/PointerStorage/Array.h"
#include "../include/PointerStorage/ManualVector.h"
#include "../include/Utilities/ReusableStorage.h"
#include "../include/Lists/BidirectionalList.h"
#include "../include/Sets/UnorderedSet.h"
#include "../include/Maps/UnorderedMap.h"
#include "../ContainerBenchmarker.h"

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


int main()
{
    ContainerBenchmarker::compareContainers<NonTrivial, Containers::UnorderedSet<NonTrivial, NonTrivial::Hasher>, 
    std::unordered_set<NonTrivial, NonTrivial::Hasher>>(
        100000, std::string("Custom set"), std::string("std set"), 
        [](size_t i) -> NonTrivial { return NonTrivial(i); }
    );

    return 0;
}
