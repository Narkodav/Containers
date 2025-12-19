#pragma once
#include <iostream>
#include <source_location>

#if !defined(_MSC_VER) && !defined(NDEBUG)
#define _DEBUG
#endif

namespace Containers {
	void verify(bool test, const char* message, const char* function, const char* file, int line, int column);
}

#define CONTAINERS_VERIFY(test, message) { \
    auto LOCATION_VARIABLE = std::source_location::current(); \
    Containers::verify(test, message, \
        LOCATION_VARIABLE.function_name(), \
        LOCATION_VARIABLE.file_name(), \
        LOCATION_VARIABLE.line(), \
        LOCATION_VARIABLE.column()); \
}