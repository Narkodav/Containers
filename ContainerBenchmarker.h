#pragma once
#pragma execution_character_set("utf-8")
#include <string>
#include <concepts>
#include <iterator>
#include <type_traits>
#include <vector>
#include <random>
#include <iostream>
#include <numeric>
#include <iomanip>
#include <chrono>
#include <functional>

template<typename T>
concept Container = requires(T container, typename T::value_type value) {
    typename T::value_type;     // Must have value_type
    typename T::size_type;      // Must have size_type
    typename T::iterator;       // Must have iterator
    typename T::const_iterator; // Must have const_iterator

    // Element access and modification
    { container.insert(value) };  // Can insert elements
    { container.erase(value) };   // Can erase elements
    { container.find(value) }     // Can find elements
    -> std::same_as<typename T::iterator>;

    // Size operations
    { container.size() } -> std::same_as<typename T::size_type>;
    { container.empty() } -> std::same_as<bool>;

    // Iterator support
    { container.begin() } -> std::same_as<typename T::iterator>;
    { container.end() } -> std::same_as<typename T::iterator>;
    { container.cbegin() } -> std::same_as<typename T::const_iterator>;
    { container.cend() } -> std::same_as<typename T::const_iterator>;

}&& std::is_default_constructible_v<T>  // Check default constructible
&& std::is_destructible_v<T>           // Check destructible
&& requires(const T container, typename T::value_type value) {
    // Const operations
    { container.find(value) }     // Can find elements in const context
    -> std::same_as<typename T::const_iterator>;
    { container.size() } -> std::same_as<typename T::size_type>;
    { container.empty() } -> std::same_as<bool>;
};

// Optional: Helper concept for checking if a type is a container of a specific value type
template<typename T, typename ValueType>
concept ContainerOf = Container<T> &&
std::same_as<typename T::value_type, ValueType>;

class ContainerBenchmarker
{
public:

    struct Metrics
    {
        double insertMean;
        double insertStdev;
        double findMean;
        double findStdev;
        double deleteMean;
        double deleteStdev;
    };

    template<typename Func>
    static double measureTime(Func&& func, size_t iterations = 5)
    {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }

    static Metrics fillMetrics(
        const std::vector<double>& insertTimes,
        const std::vector<double>& findTimes,
        const std::vector<double>& deleteTimes)
    {
        Metrics metrics;
        metrics.insertMean = std::accumulate(insertTimes.begin(), insertTimes.end(), 0.0) / insertTimes.size();
        metrics.insertStdev = std::sqrt(std::inner_product(
            insertTimes.begin(), insertTimes.end(),
            insertTimes.begin(), 0.0
        ) / insertTimes.size() - metrics.insertMean * metrics.insertMean);

        metrics.findMean = std::accumulate(findTimes.begin(), findTimes.end(), 0.0) / findTimes.size();
        metrics.findStdev = std::sqrt(std::inner_product(
            findTimes.begin(), findTimes.end(),
            findTimes.begin(), 0.0
        ) / findTimes.size() - metrics.findMean * metrics.findMean);

        metrics.deleteMean = std::accumulate(deleteTimes.begin(), deleteTimes.end(), 0.0) / deleteTimes.size();
        metrics.deleteStdev = std::sqrt(std::inner_product(
            deleteTimes.begin(), deleteTimes.end(),
            deleteTimes.begin(), 0.0
        ) / deleteTimes.size() - metrics.deleteMean * metrics.deleteMean);

        return metrics;
    }

    template <typename T, Container Container1, Container Container2>
    static std::pair<Metrics, Metrics> runTest(const std::vector<T>& testData, size_t measurementAmount)
    {
        std::random_device rd;
        std::mt19937 gen(rd());

        Container1 container1;
        Container2 container2;

        std::vector<double> insertTimes1, insertTimes2;
        std::vector<double> findTimes1, findTimes2;
        std::vector<double> deleteTimes1, deleteTimes2;

        auto shuffledData = testData;
        std::shuffle(shuffledData.begin(), shuffledData.end(), gen);

        // warmup
        for (const auto& val : testData) {
            container1.insert(val);
        }

        for (const auto& val : shuffledData) {
            auto it = container1.find(val);
            if (it == container1.end())
            {
                std::cerr << "Error\n";
                __debugbreak();
            }
        }

        for (const auto& val : shuffledData) {
            container1.erase(val);
        }

        for (const auto& val : testData) {
            container2.insert(val);
        }

        for (const auto& val : shuffledData) {
            auto it = container2.find(val);
            if (it == container2.end())
            {
                std::cerr << "Error\n";
                __debugbreak();
            }
        }

        for (const auto& val : shuffledData) {
            container2.erase(val);
        }

        for (size_t i = 0; i < measurementAmount; i++)
        {
            // Insert test
            insertTimes1.push_back(measureTime([&]() {
                for (const auto& val : testData) {
                    container1.insert(val);
                }
                }));

            insertTimes2.push_back(measureTime([&]() {
                for (const auto& val : testData) {
                    container2.insert(val);
                }
                }));

            // Find test
            findTimes1.push_back(measureTime([&]() {
                for (const auto& val : shuffledData) {
                    auto it = container1.find(val);
                    if (it == container1.end()) 
                    {
                        std::cerr << "Error\n";
                        __debugbreak();
                    }
                }
                }));

            findTimes2.push_back(measureTime([&]() {
                for (const auto& val : shuffledData) {
                    auto it = container2.find(val);
                    if (it == container2.end())
                    {
                        std::cerr << "Error\n";
                        __debugbreak();
                    }
                }
                }));

            // Delete test
            deleteTimes1.push_back(measureTime([&]() {
                size_t counter = 0;
                for (const auto& val : shuffledData) {
                    container1.erase(val);
                }
                }));

            deleteTimes2.push_back(measureTime([&]() {
                for (const auto& val : shuffledData) {
                    container2.erase(val);
                }
                }));
        }
        std::pair<Metrics, Metrics> metrics;

        // mean
        metrics.first = fillMetrics(insertTimes1, findTimes1, deleteTimes1);
        metrics.second = fillMetrics(insertTimes2, findTimes2, deleteTimes2);
        return metrics;
    };

    static void printMetrics(Metrics metrics1, Metrics metrics2, 
    const std::string& name1, const std::string& name2,
        const std::string& pattern)
    {
        int SpaceWidth = std::max(name1.size(), name2.size()) + 7;
        std::cout << std::setw(SpaceWidth) << pattern
            << std::setw(SpaceWidth) << "Insert"
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics1.insertMean
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics1.insertStdev
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics2.insertMean
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics2.insertStdev
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6)
            << metrics2.insertMean / metrics1.insertMean << "\n";

        std::cout << std::setw(SpaceWidth) << pattern
            << std::setw(SpaceWidth) << "Find"
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics1.findMean
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics1.findStdev
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics2.findMean
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics2.findStdev
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6)
            << metrics2.findMean / metrics1.findMean << "\n";

        std::cout << std::setw(SpaceWidth) << pattern
            << std::setw(SpaceWidth) << "Delete"
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics1.deleteMean
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics1.deleteStdev
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics2.deleteMean
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6) << metrics2.deleteStdev
            << std::setw(SpaceWidth) << std::fixed << std::setprecision(6)
            << metrics2.deleteMean / metrics1.deleteMean << "\n";
    }

    template <typename T, Container Container1, Container Container2>
    static void compareContainers(size_t numOperations,
        const std::string& name1, const std::string& name2,
        std::function<T(size_t)> valueGenerator = [](size_t i) { return static_cast<T>(i); })
    {
        int SpaceWidth = std::max(name1.size(), name2.size()) + 7;

        std::vector<T> data;
        data.reserve(numOperations);

        for (size_t i = 0; i < numOperations; ++i) {
            data.push_back(valueGenerator(i));
        }

        auto reverseData = data;
        std::reverse(reverseData.begin(), reverseData.end());
        auto randomData = data;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(randomData.begin(), randomData.end(), gen);

        std::cout << std::setw(SpaceWidth) << "Pattern"
            << std::setw(SpaceWidth) << "Operation"
            << std::setw(SpaceWidth) << name1 + " mean"
            << std::setw(SpaceWidth) << name1 + " stdev"
            << std::setw(SpaceWidth) << name2 + " mean"
            << std::setw(SpaceWidth) << name2 + " stdev"
            << std::setw(SpaceWidth) << "Ratio" << "\n";

        std::pair<Metrics, Metrics> metricsSequential = runTest<T, Container1, Container2>(data, 5);
        printMetrics(metricsSequential.first, metricsSequential.second,
            name1, name2,
            "Sequential");
        std::pair<Metrics, Metrics> metricsReverse = runTest<T, Container1, Container2>(reverseData, 5);
        printMetrics(metricsReverse.first, metricsReverse.second,
            name1, name2,
            "Reverse");
        std::pair<Metrics, Metrics> metricsRandom = runTest<T, Container1, Container2>(randomData, 5);
        printMetrics(metricsRandom.first, metricsRandom.second,
            name1, name2,
            "Random");
    }
};

