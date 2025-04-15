#include "Sets/Set.h"
#include "Sets/UnorderedSet.h"
#include "Maps/Map.h"
#include "Maps/UnorderedMap.h"
#include "Trees/AVLTree.h"
#include "ContainerBenchmarker.h"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <thread>

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

int main() {

    //Run benchmarks with different sizes
    std::cout << "Testing Set..." << std::endl;
    std::cout << "Running small benchmark, 100 operations...\n";
    ContainerBenchmarker::compareContainers<
        int, UnorderedSet<int, Hash>, SetOpenAdress<int, Hash>>
        (100, "std::unordered_set", "UnorderedSet");

    std::cout << "\nRunning medium benchmark, 10000 operations...\n";
    ContainerBenchmarker::compareContainers<
        int, UnorderedSet<int, Hash>, SetOpenAdress<int, Hash>>
        (10000, "std::unordered_set", "UnorderedSet");

    std::cout << "\nRunning large benchmark, 1000000 operations...\n";
    ContainerBenchmarker::compareContainers<
        int, UnorderedSet<int, Hash>, SetOpenAdress<int, Hash>>
        (1000000, "std::unordered_set", "UnorderedSet");

    return 0;
}