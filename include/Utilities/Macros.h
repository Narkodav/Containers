#pragma once
#include <iostream>
#include <source_location>

#if !defined(_MSC_VER) && !defined(NDEBUG)
#define _DEBUG
#endif

namespace Containers
{

    inline void verify_runtime(bool test, const char *message, const char *function,
                               const char *file, int line, int column)
    {
#if defined(_DEBUG)
        if (!test)
        {
            std::cerr << "❌ ASSERT FAILED: " << message
                << "\n   at " << function << " (" << file << ":" << line << ":" << column << ")"
                << std::endl;
            throw std::runtime_error("Assertion failed");
        }
#else
        (void)test;
        (void)message;
        (void)function;
        (void)file;
        (void)line;
        (void)column;
#endif
    }

}

#ifdef _MSC_VER
extern "C++"
{
    namespace __intern
    {
        template <bool>
        struct __msvc_constant_p
        {
        };
        template <>
        struct __msvc_constant_p<true>
        {
            bool __is_constant_p__();
        };
    }
#define __builtin_constant_p(x) \
    (0 __if_exists(::__intern::__msvc_constant_p < !!(x) || !(x) > ::__is_constant_p__){+1})
}
#endif /* _MSC_VER */

#define STATIC_ASSERT(x) static_assert(x, #x)

#define CONTAINERS_VERIFY(test, message)                                            \
    do                                                                              \
    {                                                                               \
        Containers::verify_runtime(test, message,                                   \
                                   std::source_location::current().function_name(), \
                                   std::source_location::current().file_name(),     \
                                   std::source_location::current().line(),          \
                                   std::source_location::current().column());       \
    } while (0)