#pragma once
#include "../Utilities/Concepts.h"
#include "linearContainer.h"
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
	class Span
	{
	public:
		using ValueType = T;
		using Iterator = ValueType *;
		using ConstIterator = const ValueType *;
		using SizeType = size_t;
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = SizeType;

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

		template <LinearContainerType Container>
		constexpr Span(Container &container)
			requires std::same_as<typename Container::ValueType, T>
			: m_data(container.data())
		{
			if constexpr (s_isDynamic)
				m_size = container.size();
			else
				static_assert(s_size == container.size());
		};

		template <LinearContainerType Container>
		constexpr Span(const Container &container)
			requires std::same_as<const typename Container::ValueType, T>
			: m_data(container.data())
		{
			if constexpr (s_isDynamic)
				m_size = container.size();
			else
				static_assert(s_size == container.size());
		};

		template <LinearContainerType Container>
		constexpr Span(Container &container, size_t size)
			requires std::same_as<typename Container::ValueType, T>
			: m_data(container.data())
		{
			if constexpr (s_isDynamic)
				m_size = size;
			else
				static_assert(s_size == size);
		};

		template <LinearContainerType Container>
		constexpr Span(const Container &container, size_t size)
			requires std::same_as<const typename Container::ValueType, T>
			: m_data(container.data())
		{
			if constexpr (s_isDynamic)
				m_size = size;
			else
				static_assert(s_size == size);
		};

		template <typename ItType>
		constexpr Span(ItType begin, ItType end)
			requires std::contiguous_iterator<ItType>
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

		constexpr Span(T *data, size_t size)
			requires s_isDynamic
			: m_data(data), m_size(size) {
			  };

		constexpr T &operator[](size_t index) { return m_data[index]; };
		constexpr const T &operator[](size_t index) const { return m_data[index]; };
		T &at(size_t index)
		{
			if (index >= size())
				throw std::out_of_range("...");
			return m_data[index];
		}
		const T &at(size_t index) const
		{
			if (index >= size())
				throw std::out_of_range("...");
			return m_data[index];
		}
		constexpr size_t size() const
		{
			if constexpr (s_isDynamic)
				return m_size;
			else
				return s_size;
		};
		constexpr const bool empty() const { return m_data == nullptr; };
		constexpr const T *data() const { return m_data; };
		constexpr T *data() { return m_data; };

		constexpr T &back() { return m_data[size() - 1]; };
		constexpr const T &back() const { return m_data[size() - 1]; };
		constexpr T &front() { return m_data[0]; };
		constexpr const T &front() const { return m_data[0]; };

		constexpr inline Iterator begin() { return m_data; }
		constexpr inline Iterator end() { return m_data + size(); }
		constexpr inline ConstIterator begin() const { return m_data; }
		constexpr inline ConstIterator end() const { return m_data + size(); }
		constexpr ConstIterator cbegin() const { return begin(); }
		constexpr inline ConstIterator cend() const { return end(); }

		constexpr void clear()
		{
			m_data = nullptr;
			if constexpr (s_size == dynamicExtent)
				m_size = 0;
		};

		template <LinearContainerType Container>
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

		constexpr Span subSpan(size_t offset, size_t length) noexcept
		{
			return Span(m_data + offset, length);
		};
	};
}
