#pragma once
#include "Utilities/UnionStorage.h"
#include <memory>
#include <stdexcept>

template <typename T, size_t m_size>
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

	using iterator = Iterator;
	using const_iterator = ConstIterator;
	using value_type = typename std::remove_reference_t<T>;
	using size_type = size_t;
private:
	T m_data[m_size];

public:
	Array() = default;
	~Array() = default;

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