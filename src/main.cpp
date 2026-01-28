#include <chrono>
#include <iostream>
#include <new>
#include <cstddef>
#include <vector>

#include "../include/Utilities/Concepts.h"

constexpr size_t ARRAY_SIZE = 100'000;

struct NonTrivial
{
    const int g;
    int x;
    NonTrivial() : x(42), g(1) {}
    NonTrivial(int d, const int f) : g(f), x(d) {}
    NonTrivial(const NonTrivial &other) : g(other.g), x(other.x) {}
    NonTrivial(NonTrivial &&other) noexcept : g(other.g), x(other.x) { other.x = 0; }
    // NonTrivial &operator=(const NonTrivial &other)
    // {
    //     x = other.x;
    //     return *this;
    // }
    // NonTrivial &operator=(NonTrivial &&other) noexcept
    // {
    //     x = other.x;
    //     other.x = 0;
    //     return *this;
    // }
    ~NonTrivial() { x = -1; }
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

#include <array>
#include <chrono>
#include <iostream>
#include <vector>
#include <list>



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
    Containers::Vector<NonTrivial> data;
    data.resize(10, {1, 1});
    for(auto it : data)
    std::cout << it.g << ", ";
    std::cout<< std::endl;
    auto* raw = data.data();
    data.clear();
    data.resize(10, {2, 2});
    for(size_t i = 0; i < data.size(); ++i)
    {
            std::cout << data[i].g << ", ";
            std::cout << raw[i].g << ", ";
    }
    std::cout << std::endl;

    Containers::BidirectionalList<int> list;
    for(size_t i = 0; i < 10; ++i)
        list.pushBack(i);
    std::cout << list.size() << std::endl;

    for(auto it : list)
        std::cout << it << ", ";
    std::cout << std::endl;
    for(size_t i = 0; i < 5; ++i)
        list.popFront();

    std::cout << list.size() << std::endl;
    for(auto it : list)
        std::cout << it << ", ";
    return 0;
}
