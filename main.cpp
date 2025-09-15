#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//#include "Sets/Set.h"
//#include "Sets/UnorderedSet.h"
//#include "Maps/Map.h"
//#include "Maps/UnorderedMap.h"
//#include "Trees/AVLTree.h"
//#include "ContainerBenchmarker.h"
//#include "LinearStorage/Vector.h"
//#include "Memory/Memory.h"
//#include "Memory/MemoryPool.h"
//#include "Memory/MemorySlabbed.h"
//#include "LinearStorage/Array.h"

#include "LinearStorage/Vector.h"
#include "LinearStorage/Array.h"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <iostream>
#include <array>
#include <vector>

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

class Test
{
public:
    int x, y;

    Test(int x, int y) : x(x), y(y) {};
    Test(const Test&) = delete;
    Test& operator=(const Test&) = delete;
    Test(Test&&) = delete;
    Test& operator=(Test&&) = delete;
};

int main() {

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        Containers::Vector<int> vector = { 1, 2, 3, 4, 5 };
        std::vector<int> stdVector = { 1, 2, 3, 4, 5 };
        auto data = vector.release();

        vector.emplace(vector.end() - 1, 10);

        for (auto& it : vector)
            std::cout << it << std::endl;

        
        for (size_t i = 0; i < data.size; ++i)
        {
            data.allocator.destroy(data.ptr + i);
        }
        data.allocator.deallocate(data.ptr);

        Containers::Array<int, 10> array = { 1, 2, 3, 4, 5 };
        std::cout << *std::find(array.begin(), array.end(), 1) << std::endl;

        for(auto& i : array)
            std::cout << i << std::endl;
    }

    if (_CrtDumpMemoryLeaks())
        __debugbreak();

    return 0;
}