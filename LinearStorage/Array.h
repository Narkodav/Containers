#pragma once
#include "Utilities/UnionStorage.h"
#include <memory>
#include <stdexcept>

namespace Containers {

	// this was done for education purposes, use std::array for anything serious
	template <typename T, size_t m_size>
		requires std::is_default_constructible_v<T>
	class Array
	{
	public:

		// Iterator class definitions
		class Iterator {
		private:
			T* m_ptr;

		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;

			constexpr Iterator(T* ptr) : m_ptr(ptr) {}

			// Dereference
			constexpr reference operator*() { return *m_ptr; }
			constexpr pointer operator->() { return m_ptr; }

			// Increment/Decrement
			constexpr Iterator& operator++() { ++m_ptr; return *this; }
			constexpr Iterator operator++(int) { Iterator tmp = *this; ++m_ptr; return tmp; }
			constexpr Iterator& operator--() { --m_ptr; return *this; }
			constexpr Iterator operator--(int) { Iterator tmp = *this; --m_ptr; return tmp; }

			// Arithmetic
			constexpr Iterator& operator+=(difference_type n) { m_ptr += n; return *this; }
			constexpr Iterator operator+(difference_type n) const { return Iterator(m_ptr + n); }
			constexpr Iterator& operator-=(difference_type n) { m_ptr -= n; return *this; }
			constexpr Iterator operator-(difference_type n) const { return Iterator(m_ptr - n); }
			constexpr difference_type operator-(const Iterator& other) const { return m_ptr - other.m_ptr; }

			// Comparison
			constexpr bool operator==(const Iterator& other) const { return m_ptr == other.m_ptr; }
			constexpr bool operator!=(const Iterator& other) const { return m_ptr != other.m_ptr; }
			constexpr bool operator<(const Iterator& other) const { return m_ptr < other.m_ptr; }
			constexpr bool operator<=(const Iterator& other) const { return m_ptr <= other.m_ptr; }
			constexpr bool operator>(const Iterator& other) const { return m_ptr > other.m_ptr; }
			constexpr bool operator>=(const Iterator& other) const { return m_ptr >= other.m_ptr; }
		};

		// Const iterator class
		class ConstIterator {
		private:
			const T* m_ptr;

		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = const T*;
			using reference = const T&;

			constexpr ConstIterator(const T* ptr) : m_ptr(ptr) {}
			constexpr ConstIterator(const Iterator& it) : m_ptr(it.m_ptr) {}

			constexpr reference operator*() const { return *m_ptr; }
			constexpr pointer operator->() const { return m_ptr; }

			constexpr ConstIterator& operator++() { ++m_ptr; return *this; }
			constexpr ConstIterator operator++(int) { ConstIterator tmp = *this; ++m_ptr; return tmp; }
			constexpr ConstIterator& operator--() { --m_ptr; return *this; }
			constexpr ConstIterator operator--(int) { ConstIterator tmp = *this; --m_ptr; return tmp; }

			constexpr ConstIterator& operator+=(difference_type n) { m_ptr += n; return *this; }
			constexpr ConstIterator operator+(difference_type n) const { return ConstIterator(m_ptr + n); }
			constexpr ConstIterator& operator-=(difference_type n) { m_ptr -= n; return *this; }
			constexpr ConstIterator operator-(difference_type n) const { return ConstIterator(m_ptr - n); }
			constexpr difference_type operator-(const ConstIterator& other) const { return m_ptr - other.m_ptr; }

			constexpr bool operator==(const ConstIterator& other) const { return m_ptr == other.m_ptr; }
			constexpr bool operator!=(const ConstIterator& other) const { return m_ptr != other.m_ptr; }
			constexpr bool operator<(const ConstIterator& other) const { return m_ptr < other.m_ptr; }
			constexpr bool operator<=(const ConstIterator& other) const { return m_ptr <= other.m_ptr; }
			constexpr bool operator>(const ConstIterator& other) const { return m_ptr > other.m_ptr; }
			constexpr bool operator>=(const ConstIterator& other) const { return m_ptr >= other.m_ptr; }

			friend class Iterator;
		};

		using ValueType = typename std::remove_reference_t<T>;
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = size_t;
	private:
		T m_data[m_size];

	public:
		constexpr Array() = default;
		constexpr ~Array() = default;

		constexpr Array(const Array& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
			requires std::is_copy_assignable_v<T>
		{
			for (size_t i = 0; i < m_size; ++i)
				m_data[i] = other.m_data[i];
		}

		constexpr Array(Array&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
			requires std::is_move_assignable_v<T>
		{
			for (size_t i = 0; i < m_size; ++i)
				m_data[i] = std::move(other.m_data[i]);
		}

		constexpr Array& operator=(const Array& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T>
		{
			if (this != &other)
			{
				for (size_t i = 0; i < m_size; ++i)
					m_data[i] = other.m_data[i];
			}
			return *this;
		}

		constexpr Array& operator=(Array&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
			requires std::is_move_assignable_v<T>
		{
			if (this != &other)
			{
				for (size_t i = 0; i < m_size; ++i)
					m_data[i] = std::move(other.m_data[i]);
			}
			return *this;
		}

		constexpr Array(const T(&init)[m_size]) noexcept(std::is_nothrow_copy_constructible_v<T>) {
			for (size_t i = 0; i < m_size; ++i) {
				m_data[i] = init[i];
			}
		}

		template<typename... Args>
		constexpr Array(Args&&... args)
			: m_data{ std::forward<Args>(args)... } // This will fail to compile if too many arguments
		{
			static_assert(sizeof...(Args) <= m_size, "Too many initializers for Array");
		}

		constexpr Array& operator=(const T(&init)[m_size]) noexcept(std::is_nothrow_copy_constructible_v<T>) {
			for (size_t i = 0; i < m_size; ++i) {
				m_data[i] = init[i];
			}
			return *this;
		}

		template<typename... Args>
		constexpr Array& operator=(Args&&... args)
		{
			static_assert(sizeof...(Args) <= m_size, "Too many initializers for Array");
			m_data = { std::forward<Args>(args)... };
			return *this;
		}

		constexpr T& operator[](size_t index) { return m_data[index]; };
		constexpr const T& operator[](size_t index) const { return m_data[index]; };
		constexpr const size_t size() const { return m_size; };

		constexpr T* data() { return m_data; }
		constexpr const T* data() const { return m_data; }
		constexpr bool empty() const { return m_size == 0; }
		T& at(size_t index) { if (index >= m_size) throw std::out_of_range("..."); return m_data[index]; }
		constexpr T& front() { return m_data[0]; }
		constexpr const T& front() const { return m_data[0]; }
		constexpr T& back() { return m_data[m_size - 1]; }
		constexpr const T& back() const { return m_data[m_size - 1]; }

		constexpr Iterator begin() { return Iterator(m_data); }
		constexpr Iterator end() { return Iterator(m_data + m_size); }
		constexpr ConstIterator begin() const { return ConstIterator(m_data); }
		constexpr ConstIterator end() const { return ConstIterator(m_data + m_size); }
		constexpr ConstIterator cbegin() const { return ConstIterator(m_data); }
		constexpr ConstIterator cend() const { return ConstIterator(m_data + m_size); }
	};
}