#include "LinearStorage/Vector.h"
#include "LinearStorage/Span.h"
#include "LinearStorage/String.h"
#include "LinearStorage/StringView.h"
#include <vector>
#include <chrono>
#include <iostream>
#include <random>

class Timer {
    std::chrono::high_resolution_clock::time_point start;
public:
    Timer() : start(std::chrono::high_resolution_clock::now()) {}

    double elapsed() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

template<typename VectorType>
void benchmarkPushBack(const std::string& name, size_t count) {
    Timer timer;
    VectorType vec;

    for (size_t i = 0; i < count; ++i) {
        if constexpr (std::is_same_v<VectorType, std::vector<int>>) {
            vec.push_back(i);
        }
        else {
            vec.pushBack(i);
        }
    }

    std::cout << name << " push_back " << count << " elements: " << timer.elapsed() << " ms\n";
}

template<typename VectorType>
void benchmarkRandomAccess(const std::string& name, size_t count) {
    VectorType vec;

    // Fill vector
    for (size_t i = 0; i < count; ++i) {
        if constexpr (std::is_same_v<VectorType, std::vector<int>>) {
            vec.push_back(i);
        }
        else {
            vec.pushBack(i);
        }
    }

    // Random access benchmark
    std::mt19937 gen(42);
    std::uniform_int_distribution<size_t> dist(0, count - 1);

    Timer timer;
    volatile int sum = 0;
    for (size_t i = 0; i < count; ++i) {
        sum += vec[dist(gen)];
    }

    std::cout << name << " random access " << count << " elements: " << timer.elapsed() << " ms\n";
}

template<typename VectorType>
void benchmarkIteration(const std::string& name, size_t count) {
    VectorType vec;

    // Fill vector
    for (size_t i = 0; i < count; ++i) {
        if constexpr (std::is_same_v<VectorType, std::vector<int>>) {
            vec.push_back(i);
        }
        else {
            vec.pushBack(i);
        }
    }

    Timer timer;
    volatile int sum = 0;
    for (const auto& val : vec) {
        sum += val;
    }

    std::cout << name << " iteration " << count << " elements: " << timer.elapsed() << " ms\n";
}

class Test {
public:
    int someData = 0;

    Test(int data) : someData(data) { std::cout << "Constructed with data: " << data << std::endl; };

    Test() { std::cout << "Constructed" << std::endl; };
    ~Test() { std::cout << "Destructed" << std::endl; };

    Test(const Test& other) : someData(other.someData) { std::cout << "copyConstructed" << std::endl; };
    Test(Test&& other) : someData(other.someData) { std::cout << "moveConstructed" << std::endl; };

    Test& operator=(const Test& other) { 
        someData = other.someData;
        std::cout << "copyAssigned" << std::endl; return *this; 
    };
    Test& operator=(Test&& other) {
        someData = other.someData;
        std::cout << "moveAssigned" << std::endl; return *this; 
    };
};

int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        Containers::Vector<Test> vec;

        vec = { 0, 1, 2, 3 };

        vec.insert(vec.begin() + 1, { 0 });
        for (auto& val : vec)
            std::cout << val.someData << std::endl;

        Containers::Span<Test> span;

        span.assign(vec.begin(), vec.end());
        for (auto& val : span)
            std::cout << val.someData << std::endl;

        Containers::Array<Test, 2> arr;
        Containers::Span<Test, 2> spanArr(arr);

        for (auto& val : spanArr)
            std::cout << val.someData << std::endl;
        std::string_view;
        Containers::String str1 = "Hello World";
        char* g = new char[100];
        g[0] = 'H'; g[1] = '\0';
        str1 = "Hello World";
        str1 += str1 + "Hello World" + g + "Hello World";
        delete[] g;

        const size_t count = 1000000;

        //std::cout << "=== Vector Benchmark ===\n\n";

        //// Push back benchmark
        //benchmarkPushBack<Containers::Vector<int>>("Custom Vector", count);
        //benchmarkPushBack<std::vector<int>>("std::vector", count);
        //std::cout << "\n";

        //// Random access benchmark
        //benchmarkRandomAccess<Containers::Vector<int>>("Custom Vector", count);
        //benchmarkRandomAccess<std::vector<int>>("std::vector", count);
        //std::cout << "\n";

        //// Iteration benchmark
        //benchmarkIteration<Containers::Vector<int>>("Custom Vector", count);
        //benchmarkIteration<std::vector<int>>("std::vector", count);
    }

    if (_CrtDumpMemoryLeaks())
        __debugbreak();

    return 0;
}