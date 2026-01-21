#pragma once
#include <type_traits>
#include <utility>

namespace Containers
{

	template <typename A>
	concept RawAllocatorConcept = requires(A a, void *p, size_t bytes, size_t align) {
		{ a.allocate(bytes, align) } -> std::same_as<void *>;
		a.deallocate(p, bytes, align);
	};

	template <typename A, typename T>
	concept TypedAllocatorConcept = requires(A a, T *p, size_t n) {
		{ static_cast<T *>(a.allocate(n * sizeof(T), alignof(T))) };
		a.deallocate(p, n * sizeof(T), alignof(T));
	};

	template <typename A, typename T>
	concept UniqueTypedAllocatorConcept = requires(A a, T *p, size_t n) {
		{ static_cast<T *>(a.allocate()) };
		a.deallocate(p);
	};

	template <typename L, typename T>
	concept LifetimeManagerConcept = requires(L l, T *p) {
		l.construct(p);
		l.destroy(p);
	};

	class RawAllocator
	{
	public:
		[[nondiscard]]
		inline void *allocate(size_t count, size_t align) noexcept
		{
			return (::operator new(count, std::align_val_t(align), std::nothrow));
		}

		inline void deallocate(void *ptr, size_t count, size_t align) noexcept
		{
			::operator delete(ptr, count, std::align_val_t(align));
		}
	};

	template <typename T>
	class TypedAllocator
	{
	public:
		[[nondiscard]]
		inline T *allocate(size_t count, size_t align = alignof(T)) noexcept
		{
			return static_cast<T *>(::operator new(count * sizeof(T),
												   std::align_val_t(align), std::nothrow));
		}

		inline void deallocate(T *ptr, size_t count, size_t align = alignof(T)) noexcept
		{
			::operator delete(ptr, count * sizeof(T), std::align_val_t(align));
		}
	};

	template <typename T>
	class UniqueTypedAllocator
	{
	public:
		[[nondiscard]]
		inline T *allocate() noexcept
		{
			return static_cast<T *>(::operator new(sizeof(T), std::align_val_t(alignof(T)), std::nothrow));
		}

		inline void deallocate(T *ptr) noexcept
		{
			::operator delete(ptr, sizeof(T), alignof(T));
		}
	};

	template <typename T>
	class ClassLifetimeManager
	{
	public:
		template <typename... Args>
		inline void construct(T *ptr, Args &&...args)
		{
			static_assert(std::is_constructible_v<T, Args...>, "Type is not constructible with given arguments");
			::new (static_cast<void *>(ptr)) T(std::forward<Args>(args)...);
		}

		inline void destroy(T *ptr)
		{
			static_assert(std::is_destructible_v<T>, "Type is not destructible");
			ptr->~T();
		}
	};

	template <typename T>
	class TrivialLifetimeManager
	{
	public:
		static_assert(std::is_trivially_constructible_v<T>, "Type is not trivially constructible");
		static_assert(std::is_trivially_destructible_v<T>, "Type is not trivially destructible");
		static_assert(std::is_trivially_copy_constructible_v<T>, "Type is not trivially copy constructible");
		static_assert(std::is_trivially_copy_assignable_v<T>, "Type is not trivially copy assignable");
		static_assert(std::is_trivially_move_constructible_v<T>, "Type is not trivially move constructible");
		static_assert(std::is_trivially_move_assignable_v<T>, "Type is not trivially move assignable");

		inline void construct(T *ptr)
		{
			(void)ptr;
		}

		inline void construct(T *ptr, const T &value)
		{
			*ptr = value;
		}

		inline void destroy(T *ptr)
		{
			(void)ptr;
		}
	};

	template <typename T>
	using LifetimeManager = std::conditional_t<std::is_trivial_v<T>,
											   TrivialLifetimeManager<T>, ClassLifetimeManager<T>>;

	template <typename Equ, typename T>
	concept Equal = requires(Equ alloc, const T &left, const T &right) {
		{ alloc.operator()(left, right) } -> std::same_as<bool>;
	};

	template <typename T>
	class DefaultEqual
	{
	public:
		bool operator()(const T &left, const T &right) const
		{
			return left == right;
		}
	};

	template <typename D, typename T>
	concept DeleterConcept =
		std::is_nothrow_move_constructible_v<D> &&
		std::is_nothrow_destructible_v<D> &&
		requires(D d, T *p) {
			{ d(p) } noexcept -> std::same_as<void>;
		};

	template <typename D, typename T>
	concept SizedDeleterConcept = DeleterConcept<D, T> &&
								  requires(const D d) {
									  { d.size() } noexcept -> std::convertible_to<size_t>;
									  { d.capacity() } noexcept -> std::convertible_to<size_t>;
								  };

	template <typename D, typename T>
	concept LifetimeAwareDeleter =
		DeleterConcept<D, T> &&
		requires(D d, T *p) {
			{ d.destroy(p) } noexcept -> std::same_as<void>;
		};

	template <typename D, typename T>
	concept ReleasableDeleter =
		SizedDeleterConcept<D, T> &&
		requires(D d, T *p) {
			{ d.releasePtr() } noexcept -> std::same_as<T *>;
			{ d.isReleased() } noexcept -> std::same_as<bool>;
		};

	template <typename T>
	class DefaultDeleter
	{
	public:
		void operator()(T *ptr) noexcept
		{
			delete ptr;
		}
	};

	template <typename T>
	class DefaultDeleter<T[]>
	{
	public:
		void operator()(T *ptr) noexcept
		{
			delete[] ptr;
		}
	};
}