#pragma once
#include "../Utilities/Concepts.h"
#include "PointerContainers.h"
#include "ContainerInterfaces.h"
#include "InitializerList.h"

#include <stdexcept>
#include <limits>

namespace Containers
{

	namespace
	{
		static inline const size_t dynamicExtent = std::numeric_limits<size_t>::max();
	}

	template <typename T, size_t s_size = dynamicExtent>
	class Span : public IteratorContainerInterface<Span<T, s_size>, T, size_t>
	{
	public:
		using IteratorBase = IteratorContainerInterface<Span<T, s_size>, T, size_t>;
		using ValueType = T;
		using Iterator = typename IteratorBase::Iterator;
		using ConstIterator = typename IteratorBase::ConstIterator;
		using SizeType = size_t;

	private:
		template <typename Convertible>
		static inline const bool s_inheritanceConstructorCondition =
			std::is_base_of_v<T, Convertible> || std::is_base_of_v<Convertible, T>;

		template <typename Convertible>
		static inline const bool s_trivialConstructorCondition =
			sizeof(Convertible) == sizeof(T) &&
			std::is_trivially_copyable_v<Convertible> &&
			std::is_trivially_copyable_v<T>;

		static inline const bool s_isDynamic = s_size == dynamicExtent;

		T *m_data = nullptr;
		std::conditional_t<s_isDynamic, size_t, std::nullptr_t> m_size = 0;

	public:
		constexpr Span() noexcept = default;
		constexpr ~Span() noexcept = default;

		constexpr Span(const Span &) noexcept = default;
		constexpr Span(Span &&) noexcept = default;
		constexpr Span &operator=(const Span &) noexcept = default;
		constexpr Span &operator=(Span &&) noexcept = default;

		template <SizableContainerType Container>
		constexpr Span(Container &container) requires std::same_as<typename Container::ValueType, T>
			: m_data(container.data())
		{
			if constexpr (s_isDynamic)
				m_size = container.size();
			else
				static_assert(s_size == container.size());
		};

		template <SizableContainerType Container>
		constexpr Span(const Container &container) requires std::same_as<const typename Container::ValueType, T>
			: m_data(container.data())
		{
			if constexpr (s_isDynamic)
				m_size = container.size();
			else
				static_assert(s_size == container.size());
		};

		template <SizableContainerType Container>
		constexpr Span(Container &container, size_t size) requires std::same_as<typename Container::ValueType, T>
			: m_data(container.data())
		{
			if constexpr (s_isDynamic)
				m_size = size;
			else
				static_assert(s_size == size);
		};

		template <SizableContainerType Container>
		constexpr Span(const Container &container, size_t size) requires std::same_as<const typename Container::ValueType, T>
			: m_data(container.data())
		{
			if constexpr (s_isDynamic)
				m_size = size;
			else
				static_assert(s_size == size);
		};

		template <PointerIteratorType ItType>
		constexpr Span(ItType begin, ItType end)
			: m_data(begin)
		{
			if constexpr (s_isDynamic)
				m_size = end - begin;
			else
				static_assert(end - begin == s_size);
		};

		template <size_t size>
		constexpr Span(T (&arr)[size]) : m_data(arr)
		{
			if constexpr (s_isDynamic)
				m_size = size;
			else
				static_assert(size == s_size);
		};

		constexpr Span(T *data, size_t size) requires s_isDynamic
		: m_data(data), m_size(size) {
		};

		constexpr size_t size() const
		{
			if constexpr (s_isDynamic)
				return m_size;
			else
				return s_size;
		};

		constexpr const bool empty() const { return m_data == nullptr; };
		constexpr const ValueType *data() const { return m_data; };
		constexpr ValueType*data() { return m_data; };

		constexpr void clear()
		{
			m_data = nullptr;
			if constexpr (s_size == dynamicExtent)
				m_size = 0;
		};

		template <SizableContainerType Container>
		constexpr bool operator==(const Container &other)
		{
			if (this->size() != other.size())
				return false;
			for (size_t i = 0; i < this->size(); ++i)
			{
				if (!((*this)[i] == other[i]))
					return false;
			}
			return true;
		}

		constexpr void swap(Span &other) noexcept
		{
			std::swap(m_data, other.m_data);
			if constexpr (s_isDynamic)
			{
				std::swap(m_size, other.m_size);
			}
		}

		constexpr void assign(T *data, size_t size) noexcept
		{
			m_data = data;
			if constexpr (s_isDynamic)
			{
				m_size = size;
			}
		}

		constexpr void reset() noexcept
		{
			m_data = nullptr;
			if constexpr (s_isDynamic)
			{
				m_size = 0;
			}
		}

		Span<ValueType> subSpan(Iterator first, Iterator last) { return Span<ValueType>(first, last); };
		Span<const ValueType> subSpan(ConstIterator first, ConstIterator last) const { return Span<const ValueType>(first, last); };

		Span<ValueType> subSpan(SizeType offset, SizeType size) { return Span<ValueType>(this->derived().data() + offset, size); };
		Span<const ValueType> subSpan(SizeType offset, SizeType size) const { return Span<const ValueType>(this->derived().data() + offset, size); };

		template<SizeType size>
		Span<ValueType, size> subSpan(SizeType offset) { return Span<ValueType, size>(this->derived().data() + offset); };

		template<SizeType size>
		Span<const ValueType, size> subSpan(SizeType offset) const { return Span<const ValueType, size>(this->derived().data() + offset); };
	};


	template<typename Derived, typename V, typename S, typename I, typename CI>
	class SubspanInterface : private CRTPBase<SubspanInterface, Derived, V, S, I, CI> {
		friend class CRTPBase<SubspanInterface, Derived, V, S, I, CI>;
	public:
		using ValueType = V;
		using SizeType = S;
		using Iterator = I;
		using ConstIterator = CI;

		Span<ValueType> asSpan() { return Span<ValueType>(this->derived().data(), this->derived().size()); };
		Span<const ValueType> asSpan() const { return Span<const ValueType>(this->derived().data(), this->derived().size()); };

		Span<ValueType> subSpan(Iterator first, Iterator last) { return Span<ValueType>(first, last); };
		Span<const ValueType> subSpan(ConstIterator first, ConstIterator last) const { return Span<const ValueType>(first, last); };

		Span<ValueType> subSpan(SizeType offset, SizeType size) { return Span<ValueType>(this->derived().data() + offset, size); };
		Span<const ValueType> subSpan(SizeType offset, SizeType size) const { return Span<const ValueType>(this->derived().data() + offset, size); };

		template<SizeType size>
		Span<ValueType, size> subSpan(SizeType offset) { return Span<ValueType, size>(this->derived().data() + offset); };

		template<SizeType size>
		Span<const ValueType, size> subSpan(SizeType offset) const { return Span<const ValueType, size>(this->derived().data() + offset); };
	};
}
