#include <chrono>
#include <iostream>
#include <new>
#include <cstddef>
#include <vector>

#include "../include/Utilities/Concepts.h"

constexpr size_t ARRAY_SIZE = 100'000;

struct NonTrivial {
    int x;
    NonTrivial() : x(42) {}
    NonTrivial(const NonTrivial& other) : x(other.x) {}
    NonTrivial(NonTrivial&& other) noexcept : x(other.x) { other.x = 0; }
    NonTrivial& operator=(const NonTrivial& other) { x = other.x; return *this; }
    NonTrivial& operator=(NonTrivial&& other) noexcept { x = other.x; other.x = 0; return *this; }
    ~NonTrivial() { x = -1; }
};

template<typename T>
void benchmark_static_array() {
    auto start = std::chrono::high_resolution_clock::now();

    // allocate and default-construct
    T* arr = new T[ARRAY_SIZE]; // automatically default-constructed

    auto mid = std::chrono::high_resolution_clock::now();

    delete[] arr; // automatically destroyed

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Static array: "
        << std::chrono::duration<double, std::nano>(mid - start).count()
        << " ns construct, "
        << std::chrono::duration<double, std::nano>(end - mid).count()
        << " ns destroy\n";
}

template<typename T>
void benchmark_raw_bytes() {
    Containers::LifetimeManager<T> life;
    auto start = std::chrono::high_resolution_clock::now();

    // allocate raw memory
    alignas(T) std::byte* buffer = new std::byte[ARRAY_SIZE * sizeof(T)];
    T* arr = reinterpret_cast<T*>(buffer);

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
template<typename T>
void benchmark_static_array_stack() {
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
template<typename T>
void benchmark_raw_bytes_stack() {
    Containers::LifetimeManager<T> life;
    auto start = std::chrono::high_resolution_clock::now();

    alignas(T) std::byte buffer[ARRAY_SIZE * sizeof(T)];
    T* arr = reinterpret_cast<T*>(buffer);

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

#include <array>
#include <chrono>
#include <iostream>
#include <vector>

template <typename Func>
auto benchmark(Func f, size_t iterations = 1000000) {
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        f();
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::nano>(end - start).count() / iterations;
}

int main() {
    constexpr size_t N = 1024;

    //std::cout << "=== Trivial type (int) ===\n";

    //// Default construction
    //auto int_default_std = benchmark([&]() { std::array<int, N> arr{}; });
    //auto int_default_custom = benchmark([&]() { Containers::Array<int, N> arr; });
    //std::cout << "Default construct: std::array = " << int_default_std
    //    << " ns, Array = " << int_default_custom << " ns\n";

    //// Copy construction
    //std::array<int, N> arr_std{};
    //Containers::Array<int, N> arr_custom{};
    //auto int_copy_std = benchmark([&]() { auto a = arr_std; });
    //auto int_copy_custom = benchmark([&]() { auto a = arr_custom; });
    //std::cout << "Copy construct: std::array = " << int_copy_std
    //    << " ns, Array = " << int_copy_custom << " ns\n";

    //// Move construction
    //auto int_move_std = benchmark([&]() { auto a = std::array<int, N>{ arr_std }; });
    //auto int_move_custom = benchmark([&]() { auto a = Containers::Array<int, N>{ arr_custom }; });
    //std::cout << "Move construct: std::array = " << int_move_std
    //    << " ns, Array = " << int_move_custom << " ns\n";

    //// Copy assignment
    //auto int_copy_assign_std = benchmark([&]() { std::array<int, N> a; a = arr_std; });
    //auto int_copy_assign_custom = benchmark([&]() { Containers::Array<int, N> a; a = arr_custom; });
    //std::cout << "Copy assign: std::array = " << int_copy_assign_std
    //    << " ns, Array = " << int_copy_assign_custom << " ns\n";

    //// Move assignment
    //auto int_move_assign_std = benchmark([&]() { std::array<int, N> a; a = std::move(arr_std); });
    //auto int_move_assign_custom = benchmark([&]() { Containers::Array<int, N> a; a = std::move(arr_custom); });
    //std::cout << "Move assign: std::array = " << int_move_assign_std
    //    << " ns, Array = " << int_move_assign_custom << " ns\n";

    //std::cout << "\n=== Non-trivial type ===\n";

    //// Default construction
    //auto nt_default_std = benchmark([&]() { std::array<NonTrivial, N> arr{}; });
    //auto nt_default_custom = benchmark([&]() { Containers::Array<NonTrivial, N> arr; });
    //std::cout << "Default construct: std::array = " << nt_default_std
    //    << " ns, Array = " << nt_default_custom << " ns\n";

    //// Copy construction
    //std::array<NonTrivial, N> nt_arr_std{};
    //Containers::Array<NonTrivial, N> nt_arr_custom{};
    //auto nt_copy_std = benchmark([&]() { auto a = nt_arr_std; });
    //auto nt_copy_custom = benchmark([&]() { auto a = nt_arr_custom; });
    //std::cout << "Copy construct: std::array = " << nt_copy_std
    //    << " ns, Array = " << nt_copy_custom << " ns\n";

    //// Move construction
    //auto nt_move_std = benchmark([&]() { auto a = std::array<NonTrivial, N>{ nt_arr_std }; });
    //auto nt_move_custom = benchmark([&]() { auto a = Containers::Array<NonTrivial, N>{ nt_arr_custom }; });
    //std::cout << "Move construct: std::array = " << nt_move_std
    //    << " ns, Array = " << nt_move_custom << " ns\n";

    //// Copy assignment
    //auto nt_copy_assign_std = benchmark([&]() { std::array<NonTrivial, N> a; a = nt_arr_std; });
    //auto nt_copy_assign_custom = benchmark([&]() { Containers::Array<NonTrivial, N> a; a = nt_arr_custom; });
    //std::cout << "Copy assign: std::array = " << nt_copy_assign_std
    //    << " ns, Array = " << nt_copy_assign_custom << " ns\n";

    //// Move assignment
    //auto nt_move_assign_std = benchmark([&]() { std::array<NonTrivial, N> a; a = std::move(nt_arr_std); });
    //auto nt_move_assign_custom = benchmark([&]() { Containers::Array<NonTrivial, N> a; a = std::move(nt_arr_custom); });
    //std::cout << "Move assign: std::array = " << nt_move_assign_std
    //    << " ns, Array = " << nt_move_assign_custom << " ns\n";


    std::cout << "=== Trivial type (int) ===\n";

    // Default construction
    auto int_default_std = benchmark([&]() { std::vector<int> vec(N); });
    auto int_default_custom = benchmark([&]() { Containers::Vector<int> vec; vec.reserve(N); vec.resize(N); });
    std::cout << "Default construct: std::vector = " << int_default_std
        << " ns, Vector = " << int_default_custom << " ns\n";

    // Copy construction
    std::vector<int> vec_std(N);
    Containers::Vector<int> vec_custom;
    vec_custom.reserve(N);
    vec_custom.resize(N);
    auto int_copy_std = benchmark([&]() { auto a = vec_std; });
    auto int_copy_custom = benchmark([&]() { auto a = vec_custom; });
    std::cout << "Copy construct: std::vector = " << int_copy_std
        << " ns, Vector = " << int_copy_custom << " ns\n";

    // Move construction
    auto int_move_std = benchmark([&]() { auto a = std::vector<int>(vec_std); });
    auto int_move_custom = benchmark([&]() { auto a = Containers::Vector<int>(vec_custom); });
    std::cout << "Move construct: std::vector = " << int_move_std
        << " ns, Vector = " << int_move_custom << " ns\n";

    // Copy assignment
    auto int_copy_assign_std = benchmark([&]() { std::vector<int> a(N); a = vec_std; });
    auto int_copy_assign_custom = benchmark([&]() { Containers::Vector<int> a; a.reserve(N); a.resize(N); a = vec_custom; });
    std::cout << "Copy assign: std::vector = " << int_copy_assign_std
        << " ns, Vector = " << int_copy_assign_custom << " ns\n";

    // Move assignment
    auto int_move_assign_std = benchmark([&]() { std::vector<int> a(N); a = std::move(vec_std); });
    auto int_move_assign_custom = benchmark([&]() { Containers::Vector<int> a; a.reserve(N); a.resize(N); a = std::move(vec_custom); });
    std::cout << "Move assign: std::vector = " << int_move_assign_std
        << " ns, Vector = " << int_move_assign_custom << " ns\n";

    std::cout << "\n=== Non-trivial type ===\n";

    // Default construction
    auto nt_default_std = benchmark([&]() { std::vector<NonTrivial> vec(N); });
    Containers::Vector<NonTrivial> vec_nt_custom;
    vec_nt_custom.reserve(N);
    vec_nt_custom.resize(N);
    auto nt_default_custom = benchmark([&]() { Containers::Vector<NonTrivial> vec; vec.reserve(N); vec.resize(N); });
    std::cout << "Default construct: std::vector = " << nt_default_std
        << " ns, Vector = " << nt_default_custom << " ns\n";

    // Copy construction
    std::vector<NonTrivial> vec_nt_std(N);
    auto nt_copy_std = benchmark([&]() { auto a = vec_nt_std; });
    auto nt_copy_custom = benchmark([&]() { auto a = vec_nt_custom; });
    std::cout << "Copy construct: std::vector = " << nt_copy_std
        << " ns, Vector = " << nt_copy_custom << " ns\n";

    // Move construction
    auto nt_move_std = benchmark([&]() { auto a = std::vector<NonTrivial>(vec_nt_std); });
    auto nt_move_custom = benchmark([&]() { auto a = Containers::Vector<NonTrivial>(vec_nt_custom); });
    std::cout << "Move construct: std::vector = " << nt_move_std
        << " ns, Vector = " << nt_move_custom << " ns\n";

    // Copy assignment
    auto nt_copy_assign_std = benchmark([&]() { std::vector<NonTrivial> a(N); a = vec_nt_std; });
    auto nt_copy_assign_custom = benchmark([&]() { Containers::Vector<NonTrivial> a; a.reserve(N); a.resize(N); a = vec_nt_custom; });
    std::cout << "Copy assign: std::vector = " << nt_copy_assign_std
        << " ns, Vector = " << nt_copy_assign_custom << " ns\n";

    // Move assignment
    auto nt_move_assign_std = benchmark([&]() { std::vector<NonTrivial> a(N); a = std::move(vec_nt_std); });
    auto nt_move_assign_custom = benchmark([&]() { Containers::Vector<NonTrivial> a; a.reserve(N); a.resize(N); a = std::move(vec_nt_custom); });
    std::cout << "Move assign: std::vector = " << nt_move_assign_std
        << " ns, Vector = " << nt_move_assign_custom << " ns\n";

    return 0;
}
