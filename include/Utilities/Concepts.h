#pragma once
#include <type_traits>
#include <utility>
#include <cstdint>

#include "Macros.h"

namespace Containers
{

	template <typename A>
	concept RawAllocatorConcept = requires(A& a, void *p, size_t bytes, size_t align) {
		{ a.allocate(bytes, align) } -> std::same_as<void *>;
		a.deallocate(p, bytes, align);
		{ std::as_const(a).maxSize() } -> std::convertible_to<size_t>;
	};

	template <typename A, typename T>
	concept TypedAllocatorConcept = requires(A& a, T *p, size_t n) {
		{ static_cast<T *>(a.allocate(n * sizeof(T), alignof(T))) };
		a.deallocate(p, n * sizeof(T), alignof(T));
		{ std::as_const(a).maxSize() } -> std::convertible_to<size_t>;
	};

	template <typename A, typename T>
	concept UniqueTypedAllocatorConcept = requires(A& a, T *p, size_t n) {
		{ static_cast<T *>(a.allocate()) };
		a.deallocate(p);
	};

	template <typename L, typename T>
	concept LifetimeManagerConcept = requires(L& l, T *p, const T * cp, size_t count) {
		l.construct(p);
		l.destroy(p);

		l.range—onstruct(p, count);
		l.range—onstruct(p, count, cp);
		l.rangeMove—onstruct(p, count, p);
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

		[[nondiscard]]
		inline void* allocate(size_t count, size_t align) noexcept {
			if (count > maxSize(align))
				return nullptr;
			return (::operator new(count, std::align_val_t(align), std::nothrow));
		}

		inline void deallocate(void* ptr, size_t count, size_t align) noexcept
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

		TypedAllocator(const TypedAllocator&) = default;
		TypedAllocator(TypedAllocator&&) = default;

		TypedAllocator& operator=(const TypedAllocator&) = default;
		TypedAllocator& operator=(TypedAllocator&&) = default;

		[[nodiscard]]
		inline constexpr size_t maxSize(size_t align = alignof(T)) const noexcept
		{
			return m_allocRaw.maxSize(align) / sizeof(T);
		}

		[[nondiscard]]
		inline T *allocate(size_t count, size_t align = alignof(T)) noexcept
		{
			return static_cast<T*>(m_allocRaw.allocate(count * sizeof(T), align));
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
		[[nondiscard]]
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
			::new (static_cast<void *>(ptr)) T(std::forward<Args>(args)...);
		}

		inline void destroy(T *ptr)
		{
			static_assert(std::is_destructible_v<T>, "Type is not destructible");
			ptr->~T();
		}

		inline void range—onstruct(T* dest, size_t count) noexcept {
			for (size_t i = 0; i < count; ++i)
				construct(dest + i);
		}

		inline void range—onstruct(T* dest, size_t count, const T* src) noexcept {
			for (size_t i = 0; i < count; ++i)
				construct(dest + i, src[i]);
		}

		inline void rangeMove—onstruct(T* dest, size_t count, T* src) noexcept {
			for (size_t i = 0; i < count; ++i)
				construct(dest + i, std::move(src[i]));
		}

		inline void rangeDestroy(T* dest, size_t count) noexcept {
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

		inline void range—onstruct(T* dest, size_t count) noexcept {
			(void)dest; (void)count;
		}

		inline void range—onstruct(T* dest, size_t count, const T* src) noexcept {
			std::memcpy(dest, src, sizeof(T) * count);
		}

		inline void rangeMove—onstruct(T* dest, size_t count, T* src) noexcept {
			range—onstruct(dest, count, src);
		}

		inline void rangeDestroy(T* dest, size_t count) noexcept {
			(void)dest; (void)count;
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

	template<typename It>
	concept IteratorType = requires(It it) {
		// Copyability
		It(it);
		it = it;

		// Equality
		{ it == it } -> std::same_as<bool>;
		{ it != it } -> std::same_as<bool>;

		// Increment
		{ ++it } -> std::same_as<It&>;
		{ it++ } -> std::same_as<It>;
	};

	template<typename It>
	concept BidirectionalIteratorType = IteratorType<It> && requires(It it) {
		{ --it } -> std::same_as<It&>;
		{ it-- } -> std::same_as<It>;
	};

	template<typename It>
	concept RandomAccessIteratorType = BidirectionalIteratorType<It> && requires(It it, It::DifferenceType n) {
		{ it + n } -> std::same_as<It>;
		{ it - n } -> std::same_as<It>;
		{ it += n } -> std::same_as<It&>;
		{ it -= n } -> std::same_as<It&>;

		{ it[n] };

		{ it < it } -> std::same_as<bool>;
		{ it <= it } -> std::same_as<bool>;
		{ it > it } -> std::same_as<bool>;
		{ it >= it } -> std::same_as<bool>;
	};

	// Contiguous iterator: a random access iterator that can be dereferenced to a value type, not necessarily tight, can be strided or mapped
	template<typename It, typename ValueType>
	concept ContiguousIteratorType = RandomAccessIteratorType<It> && requires(It it) {
		{ *it } -> std::convertible_to<ValueType&>;

		{ it + it } -> std::same_as<typename It::DifferenceType>;
		{ it - it } -> std::same_as<typename It::DifferenceType>;
	};

	// Pointer iterator: a contiguous iterator that is tightly packed, can be converted to a raw pointer, implies array-like layout
	template<typename It, typename ValueType>
	concept PointerIteratorType = ContiguousIteratorType<It, ValueType> && requires(It it) {
		{ static_cast<ValueType*>(it) } -> std::same_as<ValueType*>;
		{ it.data() } -> std::same_as<ValueType*>;
	};

	template<typename T>
	class PointerIteratorBase {
	private:
		T* m_ptr = nullptr;
	public:
		using IteratorCategory = std::random_access_iterator_tag;
		using ValueType = T;
		using DifferenceType = std::ptrdiff_t;
		using Pointer = ValueType*;
		using Reference = ValueType&;

		constexpr PointerIteratorBase() noexcept = default;
		constexpr PointerIteratorBase(T* ptr) noexcept : m_ptr(ptr) {}

		template<typename U>
		constexpr PointerIteratorBase(U* ptr) noexcept requires std::convertible_to<const U*, T*> : m_ptr(ptr) {};

		template<typename U>
		constexpr PointerIteratorBase(PointerIteratorBase<U> it) noexcept requires std::convertible_to<const U*, T*> : m_ptr(it.data()) {};

		template<typename U>
		constexpr PointerIteratorBase operator=(U* ptr) noexcept requires std::convertible_to<const U*, T*> {
			m_ptr = ptr;
		};

		template<typename U>
		constexpr PointerIteratorBase operator=(PointerIteratorBase<U> it) noexcept requires std::convertible_to<const U*, T*> {
			m_ptr = it.data();
		};


		constexpr T& operator*() const noexcept { return *m_ptr; }
		constexpr T* operator->() const noexcept { return m_ptr; }
		constexpr PointerIteratorBase& operator++() noexcept { ++m_ptr; return *this; }
		constexpr PointerIteratorBase operator++(int) noexcept {
			PointerIteratorBase temp = *this; ++m_ptr; return temp;
		}
		constexpr PointerIteratorBase& operator--() noexcept { --m_ptr; return *this; }
		constexpr PointerIteratorBase operator--(int) noexcept {
			PointerIteratorBase temp = *this; --m_ptr; return temp;
		}
		constexpr PointerIteratorBase operator+(DifferenceType n) const noexcept {
			return PointerIteratorBase(m_ptr + n);
		}
		constexpr PointerIteratorBase operator-(DifferenceType n) const noexcept {
			return PointerIteratorBase(m_ptr - n);
		}
		constexpr PointerIteratorBase& operator+=(DifferenceType n) noexcept {
			m_ptr += n; return *this;
		}
		constexpr PointerIteratorBase& operator-=(DifferenceType n) noexcept {
			m_ptr -= n; return *this;
		}

		constexpr DifferenceType operator+(PointerIteratorBase n) const noexcept {
			return m_ptr + n.m_ptr;
		}
		constexpr DifferenceType operator-(PointerIteratorBase n) const noexcept {
			return m_ptr - n.m_ptr;
		}

		constexpr Reference operator[](DifferenceType n) const noexcept {
			return *(m_ptr + n);
		}
		constexpr bool operator==(const PointerIteratorBase& other) const noexcept {
			return m_ptr == other.m_ptr;
		}
		constexpr bool operator!=(const PointerIteratorBase& other) const noexcept {
			return m_ptr != other.m_ptr;
		}
		constexpr bool operator<(const PointerIteratorBase& other) const noexcept {
			return m_ptr < other.m_ptr;
		}
		constexpr bool operator<=(const PointerIteratorBase& other) const noexcept {
			return m_ptr <= other.m_ptr;
		}
		constexpr bool operator>(const PointerIteratorBase& other) const noexcept {
			return m_ptr > other.m_ptr;
		}
		constexpr bool operator>=(const PointerIteratorBase& other) const noexcept {
			return m_ptr >= other.m_ptr;
		}
		constexpr Pointer data() const noexcept { return m_ptr; }
		constexpr operator Pointer() const noexcept { return m_ptr; };
	};


}