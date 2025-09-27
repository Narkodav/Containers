#pragma once
#include <type_traits>
#include <utility>

namespace Containers {

	template <typename Alloc, typename T>
	concept AllocatorConcept = requires(Alloc alloc, T* ptr, size_t count) {
		{ alloc.allocate(count) } -> std::same_as<T*>;
		alloc.deallocate(ptr, count);
		alloc.construct(ptr);
		alloc.destroy(ptr);
	};

	template <typename T>
	class ClassAllocator {
	public:
	
		[[nondiscard]]
		inline T* allocate(size_t count) {
			return static_cast<T*>(::operator new(count * sizeof(T), std::nothrow));
		}

		inline void deallocate(T* ptr, size_t count) {
			::operator delete(ptr, count * sizeof(T));
		}

		template <typename... Args>
		inline void construct(T* ptr, Args&&... args) {
			static_assert(std::is_constructible_v<T, Args...>, "Type is not constructible with given arguments");
			::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
		}

		inline void destroy(T* ptr) {
			static_assert(std::is_destructible_v<T>, "Type is not destructible");
			ptr->~T();
		}
	};

	template <typename T>
	class TrivialAllocator {
	public:
		static_assert(std::is_trivially_constructible_v<T>, "Type is not trivially constructible");
		static_assert(std::is_trivially_destructible_v<T>, "Type is not trivially destructible");
		static_assert(std::is_trivially_copy_constructible_v<T>, "Type is not trivially copy constructible");
		static_assert(std::is_trivially_copy_assignable_v<T>, "Type is not trivially copy assignable");
		static_assert(std::is_trivially_move_constructible_v<T>, "Type is not trivially move constructible");
		static_assert(std::is_trivially_move_assignable_v<T>, "Type is not trivially move assignable");			

		[[nondiscard]]
		inline T* allocate(size_t count) {
			return static_cast<T*>(::operator new(count * sizeof(T), std::nothrow));
		}

		inline void deallocate(T* ptr, size_t count) {
			::operator delete(ptr, count);
		}

		inline void construct(T* ptr) {
			(void)ptr;
		}

		inline void construct(T* ptr, const T& value) {
			*ptr = value;
		}

		inline void destroy(T* ptr) {
			(void)ptr;
		}
	};
	
	template <typename T>
	using Allocator = std::conditional_t<std::is_trivial_v<T>,
		TrivialAllocator<T>, ClassAllocator<T>>;

	template <typename Equ, typename T>
	concept Equal = requires(Equ alloc, const T& left, const T & right) {
		{ alloc.operator()(left, right) } -> std::same_as<bool>;
	};

	template <typename T>
	class DefaultEqual {
	public:		
		bool operator()(const T& left, const T& right) const {
			return left == right;
		}
	};

}