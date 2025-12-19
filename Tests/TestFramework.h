#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <filesystem>

class TestFramework {
private:
    static std::map<std::string, std::vector<std::function<void()>>> tests;
    static std::string currentTest;
    static int passedTests;
    static int failedTests;

public:
    static void addTest(const std::string& filename, const std::string& name, std::function<void()> test) {
        tests[filename].push_back([name, test]() {
            currentTest = name;
            std::cout << "  " << name << "... ";
            try {
                test();
                std::cout << "PASSED\n";
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "FAILED - " << e.what() << "\n";
                failedTests++;
            }
        });
    }

    static void runAll() {
        passedTests = 0;
        failedTests = 0;
        int totalTests = 0;
        for (const auto& [file, tests] : tests) {
            totalTests += tests.size();
        }
        std::cout << "Running " << totalTests << " tests from " << tests.size() << " files...\n\n";
        
        for (const auto& [filename, tests] : tests) {
            std::cout << "[" << filename << "]\n";
            for (auto& test : tests) {
                test();
            }
            std::cout << "\n";
        }
        
        std::cout << "Results: " << passedTests << " passed, " << failedTests << " failed\n";
    }

    static void verify(bool condition, const std::string& message = "") {
        if (!condition) {
            throw std::runtime_error(message.empty() ? "Assertion failed" : message);
        }
    }

    template<typename T, typename U>
    static void assertEqual(const T& expected, const U& actual, const std::string& message = "") {
        if (expected != actual) {
            throw std::runtime_error(message.empty() ? 
                "Expected != Actual" : message);
        }
    }
};



#define __TEST__(name) \
    void test_##name(); \
    static bool registered_##name = (TestFramework::addTest(std::filesystem::path(__FILE__).filename().string(), #name, test_##name), true); \
    void test_##name()

#define __ASSERT__(condition) TestFramework::verify(condition, #condition)
#define __ASSERT_EQ__(expected, actual) TestFramework::assertEqual(expected, actual)