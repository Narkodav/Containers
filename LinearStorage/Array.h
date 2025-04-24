#pragma once
#include "Utilities/UnionStorage.h"
#include <memory>
#include <stdexcept>

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

		Iterator(T* ptr) : m_ptr(ptr) {}

		// Dereference
		reference operator*() { return *m_ptr; }
		pointer operator->() { return m_ptr; }

		// Increment/Decrement
		Iterator& operator++() { ++m_ptr; return *this; }
		Iterator operator++(int) { Iterator tmp = *this; ++m_ptr; return tmp; }
		Iterator& operator--() { --m_ptr; return *this; }
		Iterator operator--(int) { Iterator tmp = *this; --m_ptr; return tmp; }

		// Arithmetic
		Iterator& operator+=(difference_type n) { m_ptr += n; return *this; }
		Iterator operator+(difference_type n) const { return iterator(m_ptr + n); }
		Iterator& operator-=(difference_type n) { m_ptr -= n; return *this; }
		Iterator operator-(difference_type n) const { return iterator(m_ptr - n); }
		difference_type operator-(const Iterator& other) const { return m_ptr - other.m_ptr; }

		// Comparison
		bool operator==(const Iterator& other) const { return m_ptr == other.m_ptr; }
		bool operator!=(const Iterator& other) const { return m_ptr != other.m_ptr; }
		bool operator<(const Iterator& other) const { return m_ptr < other.m_ptr; }
		bool operator<=(const Iterator& other) const { return m_ptr <= other.m_ptr; }
		bool operator>(const Iterator& other) const { return m_ptr > other.m_ptr; }
		bool operator>=(const Iterator& other) const { return m_ptr >= other.m_ptr; }
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

		ConstIterator(const UnionStorage<T>* ptr) : m_ptr(ptr) {}
		ConstIterator(const Iterator& it) : m_ptr(it.m_ptr) {}

		reference operator*() const { return *m_ptr; }
		pointer operator->() const { return m_ptr; }

		ConstIterator& operator++() { ++m_ptr; return *this; }
		ConstIterator operator++(int) { ConstIterator tmp = *this; ++m_ptr; return tmp; }
		ConstIterator& operator--() { --m_ptr; return *this; }
		ConstIterator operator--(int) { ConstIterator tmp = *this; --m_ptr; return tmp; }

		ConstIterator& operator+=(difference_type n) { m_ptr += n; return *this; }
		ConstIterator operator+(difference_type n) const { return ConstIterator(m_ptr + n); }
		ConstIterator& operator-=(difference_type n) { m_ptr -= n; return *this; }
		ConstIterator operator-(difference_type n) const { return ConstIterator(m_ptr - n); }
		difference_type operator-(const ConstIterator& other) const { return m_ptr - other.m_ptr; }

		bool operator==(const ConstIterator& other) const { return m_ptr == other.m_ptr; }
		bool operator!=(const ConstIterator& other) const { return m_ptr != other.m_ptr; }
		bool operator<(const ConstIterator& other) const { return m_ptr < other.m_ptr; }
		bool operator<=(const ConstIterator& other) const { return m_ptr <= other.m_ptr; }
		bool operator>(const ConstIterator& other) const { return m_ptr > other.m_ptr; }
		bool operator>=(const ConstIterator& other) const { return m_ptr >= other.m_ptr; }
	};

	using ValueType = typename std::remove_reference_t<T>;
	using iterator = Iterator;
	using const_iterator = ConstIterator;
	using value_type = ValueType;
	using size_type = size_t;
private:
	T m_data[m_size];

public:
	Array() = default;
	~Array() = default;

	Array(const Array& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
		requires std::is_copy_assignable_v<T>
	{
		for (size_t i = 0; i < m_size; ++i)
			m_data[i] = other.m_data[i];
	}

	Array(Array&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
		requires std::is_move_assignable_v<T>
	{
		for (size_t i = 0; i < m_size; ++i)
			m_data[i] = std::move(other.m_data[i]);
	}

	Array& operator=(const Array& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
		requires std::is_copy_assignable_v<T>
	{
		if(this != &other)
		{
			for (size_t i = 0; i < m_size; ++i)
				m_data[i] = other.m_data[i];
		}
		return *this;
	}

	Array& operator=(Array&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
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

	T& operator[](size_t index) { return m_data[index]; };
	const T& operator[](size_t index) const { return m_data[index]; };
	const size_t size() const { return m_size; };

	T* data() { return m_data; }
	T& at(size_t index) { if (index >= m_size) throw std::out_of_range("..."); return m_data[index]; }
	bool empty() const { return m_size == 0; }
	T& front() { return m_data[0]; }
	T& back() { return m_data[m_size - 1]; }

	Iterator begin() { return Iterator(m_data); }
	Iterator end() { return Iterator(m_data + m_size); }
	ConstIterator begin() const { return ConstIterator(m_data); }
	ConstIterator end() const { return ConstIterator(m_data + m_size); }
	ConstIterator cbegin() const { return ConstIterator(m_data); }
	ConstIterator cend() const { return ConstIterator(m_data + m_size); }
};