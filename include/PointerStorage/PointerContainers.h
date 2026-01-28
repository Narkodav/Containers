#pragma once
#include "../Utilities/Concepts.h"
#include "../Utilities/Templates.h"

namespace Containers {

    //Contiguous storage types

    //Any type that has a operator[]
    template<typename Container, typename ValueType, typename SizeType>
    concept IndexableContainer = requires(Container & container, SizeType index) {
        { container[index] } -> std::convertible_to<ValueType&>;
        { std::as_const(container)[index] } -> std::convertible_to<const ValueType&>;
    };

    //Any type that stores size
    template<typename Container, typename SizeType>
    concept SizedContainer = requires(Container & container) {
        { std::as_const(container).empty() } -> std::convertible_to<bool>;
        { std::as_const(container).size() } -> std::convertible_to<SizeType>;
    };

    template<typename Container, typename SizeType>
    concept StaticSizedContainer = requires(Container & container) {
        { Container::size() } -> std::convertible_to<SizeType>;
    };

    //Any type that has iterators
    template<typename Container>
    concept IteratedContainer = requires(Container& container) {
        // Iterator type requirements
        typename Container::Iterator;
        typename Container::ConstIterator;

        { container.begin() } -> std::same_as<typename Container::Iterator>;
        { container.end() } -> std::same_as<typename Container::Iterator>;
        { std::as_const(container).begin() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).end() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).cbegin() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).cend() } -> std::same_as<typename Container::ConstIterator>;
    };

    template<typename Container, typename ValueType, typename SizeType>
    concept GrowableContainer = requires(Container & c, ValueType & value, SizeType index) {
        { c.pushBack(value) } -> std::same_as<void>;
        { c.popBack() } -> std::same_as<void>;
        { c.resize(index) } -> std::same_as<void>;
        { c.clear() } -> std::same_as<void>;
    };

    template<typename Container, typename ValueType, typename SizeType>
    concept DynamicContainer = requires(Container& c, ValueType& value, SizeType index) {
        { c.reserve(index) } -> std::same_as<void>;
        { c.capacity() } -> std::same_as<SizeType>;
    };

    template<typename Container>
    concept PointerContainerType = requires {
        typename Container::ValueType;
        typename Container::SizeType;
    } && IteratedContainer<Container> &&
        IndexableContainer<Container, typename Container::ValueType, typename Container::SizeType>&&
        PointerIteratorType<typename Container::Iterator> && PointerIteratorType<typename Container::ConstIterator>;

    template<typename Container>
    concept ArrayContainerType = StaticSizedContainer<Container, typename Container::SizeType> && PointerContainerType<Container>;

    template<typename Container>
    concept VectorContainerType = PointerContainerType<Container> && 
        GrowableContainer<Container, typename Container::ValueType, typename Container::SizeType> &&
		DynamicContainer<Container, typename Container::ValueType, typename Container::SizeType>;

    template<typename Container>
    concept SizableContainerType = (StaticSizedContainer<Container, typename Container::SizeType> ||
        SizedContainer<Container, typename Container::SizeType>) &&
        IndexableContainer<Container, typename Container::ValueType, typename Container::SizeType>;

    template <typename T>
	class PointerIteratorBase
	{
	private:
		T *m_ptr = nullptr;

	public:
		using IteratorCategory = std::random_access_iterator_tag;
		using ValueType = T;
		using DifferenceType = std::ptrdiff_t;
		using Pointer = ValueType *;
		using Reference = ValueType &;

		constexpr PointerIteratorBase() noexcept = default;
		constexpr PointerIteratorBase(T *ptr) noexcept : m_ptr(ptr) {}

		template <typename U>
		constexpr PointerIteratorBase(U *ptr) noexcept
			requires std::convertible_to<const U *, T *>
			: m_ptr(ptr){};

		template <typename U>
		constexpr PointerIteratorBase(PointerIteratorBase<U> it) noexcept
			requires std::convertible_to<const U *, T *>
			: m_ptr(it.data()){};

		template <typename U>
		constexpr PointerIteratorBase operator=(U *ptr) noexcept
			requires std::convertible_to<const U *, T *>
		{
			m_ptr = ptr;
		};

		template <typename U>
		constexpr PointerIteratorBase operator=(PointerIteratorBase<U> it) noexcept
			requires std::convertible_to<const U *, T *>
		{
			m_ptr = it.data();
		};

		constexpr T &operator*() const noexcept { return *m_ptr; }
		constexpr T *operator->() const noexcept { return m_ptr; }
		constexpr PointerIteratorBase &operator++() noexcept
		{
			++m_ptr;
			return *this;
		}
		constexpr PointerIteratorBase operator++(int) noexcept
		{
			PointerIteratorBase temp = *this;
			++m_ptr;
			return temp;
		}
		constexpr PointerIteratorBase &operator--() noexcept
		{
			--m_ptr;
			return *this;
		}
		constexpr PointerIteratorBase operator--(int) noexcept
		{
			PointerIteratorBase temp = *this;
			--m_ptr;
			return temp;
		}
		constexpr PointerIteratorBase operator+(DifferenceType n) const noexcept
		{
			return PointerIteratorBase(m_ptr + n);
		}
		constexpr PointerIteratorBase operator-(DifferenceType n) const noexcept
		{
			return PointerIteratorBase(m_ptr - n);
		}
		constexpr PointerIteratorBase &operator+=(DifferenceType n) noexcept
		{
			m_ptr += n;
			return *this;
		}
		constexpr PointerIteratorBase &operator-=(DifferenceType n) noexcept
		{
			m_ptr -= n;
			return *this;
		}

		constexpr DifferenceType operator+(PointerIteratorBase n) const noexcept
		{
			return m_ptr + n.m_ptr;
		}
		constexpr DifferenceType operator-(PointerIteratorBase n) const noexcept
		{
			return m_ptr - n.m_ptr;
		}

		constexpr Reference operator[](DifferenceType n) const noexcept
		{
			return *(m_ptr + n);
		}
		constexpr bool operator==(const PointerIteratorBase &other) const noexcept
		{
			return m_ptr == other.m_ptr;
		}
		constexpr bool operator!=(const PointerIteratorBase &other) const noexcept
		{
			return m_ptr != other.m_ptr;
		}
		constexpr bool operator<(const PointerIteratorBase &other) const noexcept
		{
			return m_ptr < other.m_ptr;
		}
		constexpr bool operator<=(const PointerIteratorBase &other) const noexcept
		{
			return m_ptr <= other.m_ptr;
		}
		constexpr bool operator>(const PointerIteratorBase &other) const noexcept
		{
			return m_ptr > other.m_ptr;
		}
		constexpr bool operator>=(const PointerIteratorBase &other) const noexcept
		{
			return m_ptr >= other.m_ptr;
		}
		constexpr Pointer data() const noexcept { return m_ptr; }
		constexpr operator Pointer() const noexcept { return m_ptr; };
	};
}