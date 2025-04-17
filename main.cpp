#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


#include "Sets/Set.h"
#include "Sets/UnorderedSet.h"
#include "Maps/Map.h"
#include "Maps/UnorderedMap.h"
#include "Trees/AVLTree.h"
#include "ContainerBenchmarker.h"
#include "LinearStorage/Vector.h"
#include "LinearStorage/Memory.h"
#include "LinearStorage/MemoryPool.h"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <iostream>

class Complicated
{
    int data;
public:
    Complicated(int data) : data(data) {};

    Complicated(const Complicated&) = delete;
    Complicated& operator=(const Complicated&) = delete;
    Complicated(Complicated&&) = default;
    Complicated& operator=(Complicated&&) = default;

    int getData() const { return data; }

    bool operator==(const Complicated& other) const
    {
        return data == other.data;
    };
};

struct Hash
{
    size_t operator()(const Complicated& c) const
    {
        size_t hash = c.getData();
        // Do some unnecessary work
        for (int i = 0; i < 100; ++i) {
            hash = hash * 31 + i;
        }
        return hash;
    }
};

#pragma pack(push, 1)
struct StrangeAlign {
    char a;           // 1 byte
    char padding[5];  // 5 bytes padding to force total size of 6

    StrangeAlign(char val = 0) : a(val) {
        std::fill(padding, padding + 5, 0);
    }

    bool operator==(const StrangeAlign& other) const {
        return a == other.a;
    }
};
#pragma pack(pop)

template<>
struct std::alignment_of<StrangeAlign> {
    static constexpr size_t value = 6;
};

int main() {

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    ////Run benchmarks with different sizes
    //std::cout << "Testing Set..." << std::endl;
    //std::cout << "Running small benchmark, 100 operations...\n";
    //ContainerBenchmarker::compareContainers<
    //    int, std::vector<int>, Vector<int>>
    //    (100, "std::set", "AVLSet");

    //std::cout << "\nRunning medium benchmark, 10000 operations...\n";
    //ContainerBenchmarker::compareContainers<
    //    int, std::vector<int>, Vector<int>>
    //    (10000, "std::set", "AVLSet");

    //std::cout << "\nRunning large benchmark, 1000000 operations...\n";
    //ContainerBenchmarker::compareContainers<
    //    int, std::vector<int>, Vector<int>>
    //    (100000, "std::set", "AVLSet");

    //{
    //    Vector<StrangeAlign> vec;

    //    vec.reserve(3);
    //    for (int i = 0; i < 100; ++i)
    //    {
    //        vec.pushBack(i);
    //    }

    //    for (int i = 0; i < 100; ++i)
    //    {
    //        vec.erase(i);
    //    }

    //    vec.resize(10);
    //    vec.resize(10);
    //    vec.resize(20);
    //    vec.resize(10);
    //}

    //{
    //    Memory memory(Memory::GigaByte * 10);

    //    {
    //        int f = 2;
    //        auto alloc = memory.allocateArrayBestFit<int>(10, f);
    //        alloc[4] = 5;
    //        for(int i = 0; i < 10; ++i)
    //            std::cout << "Allocated: " << alloc[i] << std::endl;
    //    }
    //}

    {
        MemoryPool<int> memory(Memory::MegaByte);

        {
            int f = 2;
            auto alloc = memory.allocateArrayFirstFit(10, f);
            alloc[4] = 5;
            for (int i = 0; i < 10; ++i)
                std::cout << "Allocated: " << alloc[i] << std::endl;
        }
    }

    if (_CrtDumpMemoryLeaks())
        __debugbreak();

    return 0;
}