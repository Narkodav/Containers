#include "Sets/Set.h"
#include "Sets/UnorderedSet.h"
#include "Map.h"
#include "Trees/AVLTree.h"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>
#include <numeric>
#include <set>
#include <chrono>
#include <map>

//// Helper function to measure execution time
//template<typename Func>
//long long measureTime(Func&& func) {
//    auto start = std::chrono::high_resolution_clock::now();
//    func();
//    auto end = std::chrono::high_resolution_clock::now();
//    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
//}
//
//
//void runBenchmark(size_t numOperations) {
//    // Create vectors of random numbers for testing
//    std::vector<int> insertNumbers(numOperations);
//    std::vector<int> findNumbers(numOperations);
//    std::vector<int> deleteNumbers(numOperations);
//
//    // Initialize random number generator
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> dis(1, 1000000);
//
//    // Generate random numbers
//    for (size_t i = 0; i < numOperations; ++i) {
//        insertNumbers[i] = dis(gen);
//        findNumbers[i] = insertNumbers[i]; // Use same numbers for fair comparison
//    }
//    // Shuffle find numbers to simulate random access
//    std::shuffle(findNumbers.begin(), findNumbers.end(), gen);
//    deleteNumbers = findNumbers; // Use same shuffled numbers for deletion
//
//    // Test std::set
//    std::set<int> stdSet;
//    long long stdInsertTime = measureTime([&]() {
//        for (size_t i = 0; i < numOperations; ++i) {
//            stdSet.insert(insertNumbers[i]);
//        }
//        });
//
//    long long stdFindTime = measureTime([&]() {
//        for (size_t i = 0; i < numOperations; ++i) {
//            auto it = stdSet.find(findNumbers[i]);
//            if (it == stdSet.end()) {
//                // Prevent optimization
//                std::cerr << "Value not found in std::set" << std::endl;
//            }
//        }
//        });
//
//    long long stdDeleteTime = measureTime([&]() {
//        for (size_t i = 0; i < numOperations; ++i) {
//            stdSet.erase(deleteNumbers[i]);
//        }
//        });
//
//    // Test custom set
//    Set<int> customSet;
//    long long customInsertTime = measureTime([&]() {
//        for (size_t i = 0; i < numOperations; ++i) {
//            customSet.insert(insertNumbers[i]);
//        }
//        });
//
//    long long customFindTime = measureTime([&]() {
//        for (size_t i = 0; i < numOperations; ++i) {
//            if (customSet.find(findNumbers[i]) == nullptr) {
//                // Prevent optimization
//                std::cerr << "Value not found in custom Set" << std::endl;
//            }
//        }
//        });
//
//    long long customDeleteTime = measureTime([&]() {
//        for (size_t i = 0; i < numOperations; ++i) {
//            customSet.erase(deleteNumbers[i]);
//        }
//        });
//
//    // Print results
//    std::cout << "Benchmark results for " << numOperations << " operations:\n";
//    std::cout << "\nstd::set times (microseconds):\n";
//    std::cout << "Insert: " << stdInsertTime << "\n";
//    std::cout << "Find:   " << stdFindTime << "\n";
//    std::cout << "Delete: " << stdDeleteTime << "\n";
//    std::cout << "Total:  " << (stdInsertTime + stdFindTime + stdDeleteTime) << "\n";
//
//    std::cout << "\nCustom Set times (microseconds):\n";
//    std::cout << "Insert: " << customInsertTime << "\n";
//    std::cout << "Find:   " << customFindTime << "\n";
//    std::cout << "Delete: " << customDeleteTime << "\n";
//    std::cout << "Total:  " << (customInsertTime + customFindTime + customDeleteTime) << "\n";
//
//    // Calculate performance ratios
//    std::cout << "\nPerformance ratios (custom/std):\n";
//    std::cout << "Insert: " << static_cast<double>(customInsertTime) / stdInsertTime << "\n";
//    std::cout << "Find:   " << static_cast<double>(customFindTime) / stdFindTime << "\n";
//    std::cout << "Delete: " << static_cast<double>(customDeleteTime) / stdDeleteTime << "\n";
//    std::cout << "Total:  " << static_cast<double>(customInsertTime + customFindTime + customDeleteTime) /
//        (stdInsertTime + stdFindTime + stdDeleteTime) << "\n";
//}
//
//template <TreeType<int> Tree>
//void runDetailedBenchmark(size_t numOperations) {
//    std::vector<int> data(numOperations);
//
//    // Generate data patterns
//    // 1. Sequential
//    std::iota(data.begin(), data.end(), 0);
//
//    // 2. Reverse sequential
//    auto reverseData = data;
//    std::reverse(reverseData.begin(), reverseData.end());
//
//    // 3. Random
//    auto randomData = data;
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::shuffle(randomData.begin(), randomData.end(), gen);
//
//    // Test different patterns
//    std::cout << std::setw(15) << "Pattern"
//        << std::setw(15) << "Operation"
//        << std::setw(15) << "std::set"
//        << std::setw(15) << "Custom Set"
//        << std::setw(15) << "Ratio" << "\n";
//
//    auto runTest = [&](const std::vector<int>& testData, const std::string& pattern) {
//        std::set<int> stdSet;
//        Set<int, Tree> customSet;
//
//        // Insert test
//        auto stdInsertTime = measureTime([&]() {
//            for (const auto& val : testData) {
//                stdSet.insert(val);
//            }
//            });
//
//        auto customInsertTime = measureTime([&]() {
//            for (const auto& val : testData) {
//                customSet.insert(val);
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Insert"
//            << std::setw(15) << stdInsertTime
//            << std::setw(15) << customInsertTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(customInsertTime) / stdInsertTime << "\n";
//
//        // Find test (using shuffled data to ensure random access)
//        auto shuffledData = testData;
//        std::shuffle(shuffledData.begin(), shuffledData.end(), gen);
//
//        auto stdFindTime = measureTime([&]() {
//            for (const auto& val : shuffledData) {
//                auto it = stdSet.find(val);
//                if (it == stdSet.end()) std::cerr << "Error\n";
//            }
//            });
//
//        auto customFindTime = measureTime([&]() {
//            for (const auto& val : shuffledData) {
//                if (customSet.find(val) == nullptr) std::cerr << "Error\n";
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Find"
//            << std::setw(15) << stdFindTime
//            << std::setw(15) << customFindTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(customFindTime) / stdFindTime << "\n";
//
//        // Delete test (using shuffled data)
//        auto stdDeleteTime = measureTime([&]() {
//            for (const auto& val : shuffledData) {
//                stdSet.erase(val);
//            }
//            });
//
//        auto customDeleteTime = measureTime([&]() {
//            for (const auto& val : shuffledData) {
//                customSet.erase(val);
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Delete"
//            << std::setw(15) << stdDeleteTime
//            << std::setw(15) << customDeleteTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(customDeleteTime) / stdDeleteTime << "\n";
//        };
//
//    runTest(data, "Sequential");
//    runTest(reverseData, "Reverse");
//    runTest(randomData, "Random");
//}
//
//template <TreeType<MapPair<int, int>> Tree>
//void runMapBenchmark(size_t numOperations) {
//    std::vector<std::pair<int, int>> data(numOperations);
//
//    // Generate key-value pairs
//    for (size_t i = 0; i < numOperations; ++i) {
//        data[i] = { static_cast<int>(i), static_cast<int>(i * 2) }; // Simple value generation
//    }
//
//    // Reverse sequential
//    auto reverseData = data;
//    std::reverse(reverseData.begin(), reverseData.end());
//
//    // Random
//    auto randomData = data;
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::shuffle(randomData.begin(), randomData.end(), gen);
//
//    // Header
//    std::cout << std::setw(15) << "Pattern"
//        << std::setw(15) << "Operation"
//        << std::setw(15) << "std::map"
//        << std::setw(15) << "Custom Map"
//        << std::setw(15) << "Ratio" << "\n";
//
//    auto runTest = [&](const std::vector<std::pair<int, int>>& testData, const std::string& pattern) {
//        std::map<int, int> stdMap;
//        Map<int, int, Tree> customMap;
//
//        // Insert test
//        auto stdInsertTime = measureTime([&]() {
//            for (const auto& [key, value] : testData) {
//                stdMap.insert({ key, value });
//            }
//            });
//
//        auto customInsertTime = measureTime([&]() {
//            for (const auto& [key, value] : testData) {
//                customMap.insert(key, value);
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Insert"
//            << std::setw(15) << stdInsertTime
//            << std::setw(15) << customInsertTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(customInsertTime) / stdInsertTime << "\n";
//
//        // Find test (using shuffled data)
//        auto shuffledData = testData;
//        std::shuffle(shuffledData.begin(), shuffledData.end(), gen);
//
//        auto stdFindTime = measureTime([&]() {
//            for (const auto& [key, _] : shuffledData) {
//                auto it = stdMap.find(key);
//                if (it == stdMap.end() || it->second != stdMap.at(key)) {
//                    std::cerr << "Error in std::map find\n";
//                }
//            }
//            });
//
//        auto customFindTime = measureTime([&]() {
//            for (const auto& [key, _] : shuffledData) {
//                auto node = customMap.find(key);
//                if (node == nullptr || *node != *customMap.find(key)) {
//                    std::cerr << "Error in custom map find\n";
//                }
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Find"
//            << std::setw(15) << stdFindTime
//            << std::setw(15) << customFindTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(customFindTime) / stdFindTime << "\n";
//
//        // Delete test (using shuffled data)
//        auto stdDeleteTime = measureTime([&]() {
//            for (const auto& [key, _] : shuffledData) {
//                stdMap.erase(key);
//            }
//            });
//
//        auto customDeleteTime = measureTime([&]() {
//            for (const auto& [key, _] : shuffledData) {
//                customMap.erase(key);
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Delete"
//            << std::setw(15) << stdDeleteTime
//            << std::setw(15) << customDeleteTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(customDeleteTime) / stdDeleteTime << "\n";
//        };
//
//    runTest(data, "Sequential");
//    runTest(reverseData, "Reverse");
//    runTest(randomData, "Random");
//}
//
//void compareTreeImplementations(size_t numOperations) {
//    std::vector<int> data(numOperations);
//
//    // Generate data patterns
//    std::iota(data.begin(), data.end(), 0);
//
//    auto reverseData = data;
//    std::reverse(reverseData.begin(), reverseData.end());
//
//    auto randomData = data;
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::shuffle(randomData.begin(), randomData.end(), gen);
//
//    // Header
//    std::cout << std::setw(15) << "Pattern"
//        << std::setw(15) << "Operation"
//        << std::setw(15) << "AVL Tree"
//        << std::setw(15) << "RB Tree"
//        << std::setw(15) << "Ratio" << "\n";
//
//    auto runTest = [&](const std::vector<int>& testData, const std::string& pattern) {
//        Set<int, AVLTree<int>> avlSet;
//        Set<int, RedBlackTree<int>> rbSet;
//
//        // Insert test
//        auto avlInsertTime = measureTime([&]() {
//            for (const auto& val : testData) {
//                avlSet.insert(val);
//            }
//            });
//
//        auto rbInsertTime = measureTime([&]() {
//            for (const auto& val : testData) {
//                rbSet.insert(val);
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Insert"
//            << std::setw(15) << avlInsertTime
//            << std::setw(15) << rbInsertTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(avlInsertTime) / rbInsertTime << "\n";
//
//        // Find test
//        auto shuffledData = testData;
//        std::shuffle(shuffledData.begin(), shuffledData.end(), gen);
//
//        auto avlFindTime = measureTime([&]() {
//            for (const auto& val : shuffledData) {
//                if (avlSet.find(val) == nullptr) std::cerr << "Error\n";
//            }
//            });
//
//        auto rbFindTime = measureTime([&]() {
//            for (const auto& val : shuffledData) {
//                if (rbSet.find(val) == nullptr) std::cerr << "Error\n";
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Find"
//            << std::setw(15) << avlFindTime
//            << std::setw(15) << rbFindTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(avlFindTime) / rbFindTime << "\n";
//
//        // Delete test
//        auto avlDeleteTime = measureTime([&]() {
//            for (const auto& val : shuffledData) {
//                avlSet.erase(val);
//            }
//            });
//
//        auto rbDeleteTime = measureTime([&]() {
//            for (const auto& val : shuffledData) {
//                rbSet.erase(val);
//            }
//            });
//
//        std::cout << std::setw(15) << pattern
//            << std::setw(15) << "Delete"
//            << std::setw(15) << avlDeleteTime
//            << std::setw(15) << rbDeleteTime
//            << std::setw(15) << std::fixed << std::setprecision(2)
//            << static_cast<double>(avlDeleteTime) / rbDeleteTime << "\n";
//        };
//
//    std::cout << "\nRunning benchmark with " << numOperations << " operations\n";
//    std::cout << "----------------------------------------\n";
//    runTest(data, "Sequential");
//    runTest(reverseData, "Reverse");
//    runTest(randomData, "Random");
//}

int main() {
    // Run benchmarks with different sizes
    //std::cout << "Testing Set..." << std::endl;
    //std::cout << "Running small benchmark...\n";
    //runDetailedBenchmark<AVLTree<int>>(100);

    //std::cout << "\nRunning medium benchmark...\n";
    //runDetailedBenchmark<AVLTree<int>>(10000);

    //std::cout << "\nRunning large benchmark...\n";
    //runDetailedBenchmark<AVLTree<int>>(100000);

    //std::cout << std::endl;

    //std::cout << "Testing Map..." << std::endl;
    //std::cout << "Running small benchmark...\n";
    //runMapBenchmark<AVLTree<MapPair<int, int>>>(100);

    //std::cout << "\nRunning medium benchmark...\n";
    //runMapBenchmark<AVLTree<MapPair<int, int>>>(10000);

    //std::cout << "\nRunning large benchmark...\n";
    //runMapBenchmark<AVLTree<MapPair<int, int>>>(100000);

    //compareTreeImplementations(1000);    // Small dataset
    //compareTreeImplementations(100000);  // Medium dataset
    //compareTreeImplementations(1000000); // Large dataset

    UnorderedSet<int> set;

    for (int i = 0; i < 100; i++)
        set.insert(i);

    int i = 0;
    for (auto it : set)
    {
        std::cout << it << std::endl;
        i++;
    }

    return 0;
}