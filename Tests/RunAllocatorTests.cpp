#include "TestFramework.h"

// Static member definitions
std::map<std::string, std::vector<std::function<void()>>> TestFramework::testsByFile;
std::string TestFramework::currentTest;
int TestFramework::passedTests = 0;
int TestFramework::failedTests = 0;

int main() {
    TestFramework::runAll();
    return TestFramework::failedTests > 0 ? 1 : 0;
}