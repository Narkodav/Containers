#include <chrono>
#include <iostream>
#include <new>
#include <cstddef>
#include <vector>

#include "Containers/Utilities/Concepts.h"
//#include "TestFramework.h"

#include "Containers/Math/Tensors.h"

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

#include "Containers/PointerStorage/Vector.h"
#include "Containers/PointerStorage/Array.h"
#include "Containers/PointerStorage/ManualVector.h"
#include "Containers/Utilities/ReusableStorage.h"
#include "Containers/Lists/BidirectionalList.h"
#include "Containers/Sets/UnorderedSet.h"
#include "Containers/Maps/UnorderedMap.h"
#include "ContainerBenchmarker.h"
#include "Containers/Memory/ExternalMetadataAllocators/BuddyAllocator.h"
#include "Containers/Memory/ExternalMetadataAllocators/ListAllocator.h"

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

#include "Containers/Utilities/TemplateUnion.h"

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

// template<typename T>
// void foo(TensorExpression<T> expr) {

// }

#include <iostream>

struct Base {
    template<int N>
    void f() { std::cout << "Parent" << std::endl; }
};

struct Derived : Base {
    using Base::f;

    template<char C>
    void f() { std::cout << "Child" << std::endl; }
};

int main() {
    Derived d;
    // d.f<static_cast<int>(1)>();
    // d.f<static_cast<char>(1)>();
}

// int main()
// {
//     using namespace Containers::Math;

//     //--------------------------------------------------------------------------
//     // Construction
//     //--------------------------------------------------------------------------

//     Tensor<int, 2,3,4> t1;
//     Tensor<int, 2,3,4> t2;

//     TensorView<int, 2,3,4> v1 = t1;
//     TensorView<int, 2,3,4> v2 = t2;

//     auto aa = t1[0];

//     TensorView<int, 3,4> v11 = t1[0];
//     TensorView<int, 3,4> v12 = t2[0];

//     TensorView<int, 4> v21 = v11[0];
//     TensorView<int, 4> v22 = v12[0];

//     int& x1 = v21[0];
//     int& x2 = v22[0];

//     int scalar;

//     //--------------------------------------------------------------------------
//     // Copy construction
//     //--------------------------------------------------------------------------

//     Tensor<int, 2,3,4> ct1 = t1;
//     Tensor<int, 2,3,4> ct2(v1);

//     Tensor<int, 3,4> ct3 = v11;
//     Tensor<int, 3,4> ct4(v12);

//     Tensor<int, 4> ct5 = v21;
//     Tensor<int, 4> ct6(v22);

//     Tensor<int, 2,3,4> cccc = t1 + ct1 - ct1.view();

//     //--------------------------------------------------------------------------
//     // Assignment : tensor <- tensor
//     //--------------------------------------------------------------------------

//     ct1 = t2;
//     ct1 = ct2;

//     //--------------------------------------------------------------------------
//     // Assignment : tensor <- view
//     //--------------------------------------------------------------------------

//     ct1 = v1;
//     ct3 = v11;
//     ct5 = v21;

//     //--------------------------------------------------------------------------
//     // Assignment : view <- tensor
//     //--------------------------------------------------------------------------

//     v1 = t1;
//     v11 = ct3;
//     v21 = ct5;

//     //--------------------------------------------------------------------------
//     // Assignment : view <- view
//     //--------------------------------------------------------------------------

//     v1 = v2;
//     v11 = v12;
//     v21 = v22;

//     //--------------------------------------------------------------------------
//     // Simple expressions
//     //--------------------------------------------------------------------------

//     auto e1 = v21 + v22;
//     auto e2 = v21 - v22;
//     auto e3 = v21 * v22;
//     auto e4 = v21 / v22;

//     //--------------------------------------------------------------------------
//     // Tensor construction from expression
//     //--------------------------------------------------------------------------

//     Tensor<int, 4> r1 = v21 + v22;
//     Tensor<int, 4> r2 = v21 - v22;
//     Tensor<int, 4> r3 = v21 * v22;
//     //Tensor<int, 4> r4 = v21 / v22;

//     //--------------------------------------------------------------------------
//     // Nested expressions
//     //--------------------------------------------------------------------------

//     Tensor<int, 4> r5 =
//         v21 + v22 + v21;

//     Tensor<int, 4> r6 =
//         v21 + v22 - v21;

//     Tensor<int, 4> r7 =
//         (v21 + v22) * v21;

//     // Tensor<int, 4> r8 =
//     //     (v21 + v22) / v21;

//     //--------------------------------------------------------------------------
//     // Full tensor expressions
//     //--------------------------------------------------------------------------

//     Tensor<int, 2,3,4> r9 =
//         t1 + t2;

//     Tensor<int, 2,3,4> r10 =
//         t1 + v2;

//     Tensor<int, 2,3,4> r11 =
//         v1 + t2;

//     Tensor<int, 2,3,4> r12 =
//         v1 + v2;

//     //--------------------------------------------------------------------------
//     // Long expression chains
//     //--------------------------------------------------------------------------

//     Tensor<int, 2,3,4> r13 =
//         t1 + t2.view() + t2 + t2.view();

//     Tensor<int, 2,3,4> r14 =
//         t1 + v2 + t2 + v1;

//     Tensor<int, 2,3,4> r15 =
//         ((t1 + t2) - v1) + (v2 + t1);

//     //--------------------------------------------------------------------------
//     // Compound assignment
//     //--------------------------------------------------------------------------

//     r9 += v1;
//     r9 -= v1;
//     r9 *= v1;
//     //r9 /= v1;

//     //--------------------------------------------------------------------------
//     // Compound assignment chains
//     //--------------------------------------------------------------------------

//     r10 += (v1 -= t1);

//     r11 -= (v1 += t1);

//     r12 *= (v1 -= v2);

//     //r13 /= (v1 += v2);

//     //--------------------------------------------------------------------------
//     // Result of compound assignment
//     //--------------------------------------------------------------------------

//     Tensor<int, 2,3,4> r16 =
//         (r14 -= r15);

//     Tensor<int, 2,3,4> r17 =
//         (r14 += r15);

//     Tensor<int, 2,3,4> r18 =
//         (r14 *= r15);

//     // Tensor<int, 2,3,4> r19 =
//     //     (r14 /= r15);

//     //--------------------------------------------------------------------------
//     // Deep indexing
//     //--------------------------------------------------------------------------

//     auto a = t1[0];
//     auto b = t1[0][0];
//     auto c = t1[0][0][0];

//     auto d = v1[0];
//     auto e = v1[0][0];
//     auto f = v1[0][0][0];

//     //--------------------------------------------------------------------------
//     // Explicit view conversions
//     //--------------------------------------------------------------------------

//     TensorView<int, 2,3,4> vv1 = t1.view();
//     TensorView<const int, 2,3,4> vv2 = t1.view();

//     TensorView<int, 3,4> vv3 = t1[0];
//     TensorView<int, 4> vv4 = t1[0][0];

//     //--------------------------------------------------------------------------
//     // Scalar operations
//     //--------------------------------------------------------------------------

//     v1 = v1 + scalar;
//     v1 = scalar + v1;
    

//     static_assert(std::is_assignable_v<
//         Tensor<int,2,3,4>&,
//         TensorView<int,2,3,4>>);

//     static_assert(std::is_assignable_v<
//         TensorView<int,2,3,4>&,
//         Tensor<int,2,3,4>>);

//     static_assert(std::is_constructible_v<
//         Tensor<int,2,3,4>,
//         TensorView<int,2,3,4>>);


//     auto expr = t1 + t2;
//     Tensor<int,2,3,4> r = expr;
//     r = expr;

//     auto fff = (!t1[0]).eval();

//     Tensor<int, 3,4> vv5 = !t1[0];
    
//     TensorView<int, 3,4> vv6 = vv5++;
//     TensorView<int, 3,4> vv7 = ++vv5;
//     auto dfgh = vv7++;

//     Tensor<int, 3> cross1;
//     Tensor<int, 3> cross2;
//     Tensor<int, 3> cross3 = cross(cross1, cross2);

//     Tensor<int, 4, 2> aaa;
//     Tensor<int, 2, 3> bbb;

//     Tensor<int, 2> ccc;

//     auto res1 = Containers::Math::mul(aaa, bbb);

//     auto res2 = Containers::Math::mul(aaa, ccc);

//     Tensor<int, 2> ccc1({0, 1});
//     Tensor<int, 2> ccc2 = {0, 1};

//     Parent p;
//     Child ccccc;

//     std::unique_ptr<Parent> parent;
//     std::unique_ptr<Child> child;

//     parent = std::make_unique<Parent>();
//     child = std::make_unique<Child>();

//     parent->sub<Enum1::A>();
//     child->Parent::sub<Enum1::A>();
//     child->sub<Enum2::B>();




//     return 0;
// }
