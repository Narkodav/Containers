#pragma once
#include <initializer_list>

namespace Containers {
	// initializer_list is compiler intrinsic, no way to reimplement it
	template <typename T>
	using InitializerList = std::initializer_list<T>;
}