#pragma once
#include <type_traits>
#include <utility>
#include <cstdint>
#include <cstring>
#include "Macros.h"

namespace Containers
{
	template <typename A>
	concept RawAllocatorType = requires(A &a, void *p, size_t bytes, size_t align) {
		{ a.allocate(bytes, align) } -> std::same_as<void *>;
		a.deallocate(p, bytes, align);
		{ std::as_const(a).maxSize(align) } -> std::convertible_to<size_t>;
	};

	template <typename A, typename T>
	concept TypedAllocatorType = requires(A &a, T *p, size_t n) {
		{ static_cast<T *>(a.allocate(n * sizeof(T), alignof(T))) };
		a.deallocate(p, n * sizeof(T), alignof(T));
		{ std::as_const(a).maxSize() } -> std::convertible_to<size_t>;
	};

	template <typename A, typename T>
	concept UniqueTypedAllocatorType = requires(A &a, T *p, size_t n) {
		{ static_cast<T *>(a.allocate()) };
		a.deallocate(p);
	};

	template <typename L, typename T>
	concept LifetimeManagerType = requires(L &l, T *p, const T *cp, size_t count) {
		l.construct(p);
		l.destroy(p);

		l.rangeConstruct(p, count);
		l.rangeConstruct(p, count, cp);
		l.rangeMoveConstruct(p, count, p);
		l.rangeDestroy(p, count);
	};

	class RawAllocator
	{
	public:
		[[nodiscard]]
		inline constexpr size_t maxSize(size_t align) const noexcept
		{
			// Worst-case: allocator may need (align - 1) extra bytes
			// We must ensure size + (align - 1) does not overflow size_t
			return std::numeric_limits<size_t>::max() - (align - 1);
		}

		[[nodiscard]]
		inline void *allocate(size_t count, size_t align) noexcept
		{
			if (count > maxSize(align))
				return nullptr;
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
	private:
		[[no_unique_address]] RawAllocator m_allocRaw;

	public:
		TypedAllocator() = default;
		~TypedAllocator() = default;

		TypedAllocator(const TypedAllocator &) = default;
		TypedAllocator(TypedAllocator &&) = default;

		TypedAllocator &operator=(const TypedAllocator &) = default;
		TypedAllocator &operator=(TypedAllocator &&) = default;

		[[nodiscard]]
		inline constexpr size_t maxSize(size_t align = alignof(T)) const noexcept
		{
			return m_allocRaw.maxSize(align) / sizeof(T);
		}

		[[nodiscard]]
		inline T *allocate(size_t count, size_t align = alignof(T)) noexcept
		{
			return static_cast<T *>(m_allocRaw.allocate(count * sizeof(T), align));
		}

		inline void deallocate(T *ptr, size_t count, size_t align = alignof(T)) noexcept
		{
			m_allocRaw.deallocate(ptr, count * sizeof(T), align);
		}
	};

	template <typename T>
	class UniqueTypedAllocator
	{
	public:
		[[nodiscard]]
		inline T *allocate(size_t align = alignof(T)) noexcept
		{
			return static_cast<T *>(::operator new(sizeof(T), std::align_val_t(align), std::nothrow));
		}

		inline void deallocate(T *ptr, size_t align = alignof(T)) noexcept
		{
			::operator delete(ptr, sizeof(T), std::align_val_t(align));
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
			::new (static_cast<void *>(std::launder(ptr))) T(std::forward<Args>(args)...);
		}

		inline void destroy(T *ptr)
		{
			static_assert(std::is_destructible_v<T>, "Type is not destructible");
			ptr->~T();
		}

		inline void rangeConstruct(T *dest, size_t count) noexcept
		{
			for (size_t i = 0; i < count; ++i)
				construct(dest + i);
		}

		inline void rangeConstruct(T *dest, size_t count, const T *src) noexcept
		{
			for (size_t i = 0; i < count; ++i)
				construct(dest + i, src[i]);
		}

		inline void rangeMoveConstruct(T *dest, size_t count, T *src) noexcept
		{
			for (size_t i = 0; i < count; ++i)
				construct(dest + i, std::move(src[i]));
		}

		inline void rangeDestroy(T *dest, size_t count) noexcept
		{
			for (size_t i = 0; i < count; ++i)
				destroy(dest + i);
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

		inline void rangeConstruct(T *dest, size_t count) noexcept
		{
			(void)dest;
			(void)count;
		}

		inline void rangeConstruct(T *dest, size_t count, const T *src) noexcept
		{
			std::memcpy(dest, src, sizeof(T) * count);
		}

		inline void rangeMoveConstruct(T *dest, size_t count, T *src) noexcept
		{
			std::memcpy(dest, src, sizeof(T) * count);
		}

		inline void rangeDestroy(T *dest, size_t count) noexcept
		{
			(void)dest;
			(void)count;
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
	concept SizedDeleterConcept = DeleterConcept<D, T> && requires(const D d) {
		{ d.size() } noexcept -> std::convertible_to<size_t>;
		{ d.capacity() } noexcept -> std::convertible_to<size_t>;
	};

	template <typename D, typename T>
	concept LifetimeAwareDeleter = DeleterConcept<D, T> && requires(D d, T *p) {
			{ d.destroy(p) } noexcept -> std::same_as<void>;
		};

	template <typename D, typename T>
	concept ReleasableDeleter = SizedDeleterConcept<D, T> && requires(D d, T *p) {
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

	template <typename It>
	concept EqualityComparableIteratorType = requires(It it) {
		{ it == it } -> std::same_as<bool>;
		{ it != it } -> std::same_as<bool>;
	};

	template <typename It>
	concept CopyableIteratorType = requires(It it) {
		It(it);
		it = it;
	};

	template <typename It>
	concept IteratorType = requires(It it) {
		typename It::ValueType;
		{ *it } -> std::convertible_to<typename It::ValueType &>;
		{ it.operator->() } -> std::convertible_to<typename It::ValueType*>;
	} && EqualityComparableIteratorType<It> && CopyableIteratorType<It>;

	template <typename It>
	concept ForwardIteratorType = IteratorType<It> && requires(It it) {

		// Increment
		{ ++it } -> std::convertible_to<It &>;
		{ it++ } -> std::convertible_to<It>;
	};

	template <typename It>
	concept BackwardIteratorType = IteratorType<It> && requires(It it) {

		// Increment
		{ ++it } -> std::convertible_to<It &>;
		{ it++ } -> std::convertible_to<It>;
	};

	template <typename It>
	concept BidirectionalIteratorType = ForwardIteratorType<It> && BackwardIteratorType<It>;

	template <typename It>
	concept ComparableIteratorType = requires(It it) {
		{ it < it } -> std::convertible_to<bool>;
		{ it <= it } -> std::convertible_to<bool>;
		{ it > it } -> std::convertible_to<bool>;
		{ it >= it } -> std::convertible_to<bool>;
	};

	template <typename It>
	concept BidirectionalComparableIteratorType = BidirectionalIteratorType<It> && ComparableIteratorType<It>;

	template <typename It>
	concept RandomAccessIteratorType = BidirectionalComparableIteratorType<It> &&
		requires(It it, It::DifferenceType n) {
		typename It::DifferenceType;

		{ it + n } -> std::convertible_to<It>;
		{ it - n } -> std::convertible_to<It>;
		{ it += n } -> std::convertible_to<It &>;
		{ it -= n } -> std::convertible_to<It &>;

		{ it[n] } -> std::convertible_to<typename It::ValueType &>;
	};

	// Contiguous iterator: a random access iterator that can be dereferenced to a value type, not necessarily tight, can be strided or mapped
	template <typename It>
	concept ContiguousIteratorType = RandomAccessIteratorType<It> && requires(It a, It b) {
        { b - a } -> std::convertible_to<typename It::DifferenceType>; // allows strided arithmetic
    };

	// Pointer iterator: a contiguous iterator that is tightly packed, can be converted to a raw pointer, implies array-like layout
	template <typename It>
	concept PointerIteratorType = ContiguousIteratorType<It> && requires(It it, It::DifferenceType n) {
		{ static_cast<typename It::ValueType *>(it) } -> std::convertible_to<typename It::ValueType *>;
		{ it.data() } -> std::convertible_to<typename It::ValueType *>;

		requires std::convertible_to<typename It::DifferenceType, size_t>;
	};
}