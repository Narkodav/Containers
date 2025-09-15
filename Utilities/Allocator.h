#pragma once
#include <type_traits>
#include <utility>

namespace Containers {

	template <typename T>
	class Allocator {
	public:
		T* allocate(size_t size) {
			return static_cast<T*>(::operator new(size * sizeof(T)));
		}

		void deallocate(T* ptr) {
			::operator delete(ptr);
		}

		template <typename... Args>
		void construct(T* ptr, Args&&... args) {
			::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
		}

		void destroy(T* ptr) {
			ptr->~T();
		}

		// this is used in case we need memory pool tracking and memory ownership is passed somewhere else
		// there is no point in implementation for basic allocator because it doesn't track anything
		T* release(T* ptr) { return ptr; };

	};

}