#include "Macros.h"
#include <iostream>
#include <stdexcept>

namespace Containers {
	void verify(bool test, const char* message, const char* function, const char* file, int line, int column)
	{
#ifdef _DEBUG
		if (!test)
		{
			std::cerr << "❌ ASSERT FAILED: " << message
				<< "\n   at " << function << " (" << file << ":" << line << ":" << column << ")"
				<< std::endl;
			throw std::runtime_error("Assertion failed");
		}
#else 
		(void)test, (void)message, (void)function, (void)file, (void)line, (void)column;
#endif // _DEBUG
	}
}