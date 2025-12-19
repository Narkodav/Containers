#include "TestFramework.h"

std::map<std::string, std::vector<std::function<void()>>> TestFramework::tests;
std::string TestFramework::currentTest;
int TestFramework::passedTests = 0;
int TestFramework::failedTests = 0;