#pragma once
#include "Utilities/ByteArray.h"
#include <memory>
#include <stdexcept>

template <typename T, size_t initialCapacity = 16>
class Vector
{
public:
	static constexpr float growthFactor = 1.618f;

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

		Iterator(const Iterator&) = default;
		Iterator& operator=(const Iterator&) = default;

		Iterator(Iterator&&) = default;
		Iterator& operator=(Iterator&&) = default;

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

		ConstIterator(const T* ptr) : m_ptr(ptr) {}

		ConstIterator(const ConstIterator&) = default;
		ConstIterator& operator=(const ConstIterator&) = default;

		ConstIterator(ConstIterator&&) = default;
		ConstIterator& operator=(ConstIterator&&) = default;

		reference operator*() { return *m_ptr; }
		pointer operator->() { return m_ptr; }

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
	using SizeType = size_t;
	using iterator = Iterator;
	using const_iterator = ConstIterator;
	using value_type = ValueType;
	using size_type = SizeType;

	static inline const size_t typeSize = sizeof(T);
	static inline const size_t typeAlign = alignof(T);

private:
	ByteArray m_data;
	size_t m_size = 0;
	size_t m_capacity = 0;

public:
	Vector() : m_data(initialCapacity * typeSize), m_capacity(initialCapacity) {};
	Vector(size_t size) : m_data(initialCapacity * typeSize),
		m_capacity(initialCapacity), m_size(0) {
		resize(size);
	};
	Vector(size_t size, const T& value) : m_data(initialCapacity * typeSize),
		m_capacity(initialCapacity), m_size(0) {
		if (size > m_capacity)
			reserve(growthFactor * (size + m_capacity - 1));
		m_size = size;
		for (size_t i = 0; i < size; ++i)
			m_data.emplace(value, i * typeSize);
	};

	~Vector()
	{
		clear();
		m_data.destroy();
	}

	Vector(const Vector& other) requires std::is_copy_constructible_v<T> : m_data(other.m_capacity),
		m_capacity(other.m_capacity), m_size(other.m_size) {

		for (size_t i = 0; i < m_size; ++i)
			m_data.emplace(other.m_data.get<T>(i * typeSize), i * typeSize);
	};

	Vector(Vector&& other) : m_data(std::move(other.m_data)),
		m_capacity(other.m_capacity), m_size(other.m_size) {
		other.m_capacity = 0;
		other.m_size = 0;
	};

	Vector& operator=(const Vector& other) requires std::is_copy_constructible_v<T> {
		if (this == &other)
			return *this;
		clear();
		m_data.destroy();
		m_capacity = other.m_capacity;
		m_size = other.m_size;
		m_data.allocate(m_capacity);		
		for (size_t i = 0; i < m_size; ++i)
			m_data.emplace(other.m_data.get<T>(i * typeSize), i * typeSize);
		return *this;
	};

	Vector& operator=(Vector&& other) {
		if (this == &other)
			return *this;
		clear();
		m_data.destroy();
		m_capacity = other.m_capacity;
		m_size = other.m_size;
		m_data = std::move(other.m_data);
		other.m_capacity = 0;
		other.m_size = 0;
		return *this;
	};

	void clear() {
		if (m_size == 0)
			return;
		m_data.erase<T>(m_size, 0);
		m_size = 0;
	};

	template<typename U>
	void pushBack(U&& data)
	{
		if (m_size >= m_capacity)
			reserve(growthFactor * m_capacity);
		m_data.emplace(std::forward<U>(data), m_size * typeSize);
		m_size++;
	}

	void popBack()
	{
		if (m_size == 0)
			throw std::runtime_error("Vector is empty");
		m_data.erase<T>((m_size - 1) * typeSize);
		m_size--;
	}

	T& operator[](size_t index) { return *m_data.get<T>(index * typeSize); };
	const T& operator[](size_t index) const { return *m_data.get<T>(index * typeSize); };
	const size_t size() const { return m_size; };
	const bool empty() const { return m_size == 0; };
	const size_t capacity() const { return m_capacity; };
	const T* data() const { return m_data.get<T>(0); };
	T* data() { return m_data.get<T>(0); }

	void reserve(size_t capacity) {
		if (capacity == m_capacity)
			return;
		ByteArray oldData(m_data.data());
		m_data.allocate(capacity * typeSize);

		m_capacity = capacity;
		if (m_capacity < m_size)
			m_capacity = m_size;

		for (size_t i = 0; i < m_size; i++)
		{
			if constexpr (std::is_move_assignable_v<T>) {
				m_data.emplace(std::move(*(oldData.get<T>(i * typeSize))), i * typeSize);
			}
			else if constexpr (std::is_copy_assignable_v<T>) {
				m_data.emplace(*(oldData.get<T>(i * typeSize)), i * typeSize);
			}
			else static_assert(false, "Key type must be copy or move assignable");
			oldData.erase<T>(i * typeSize);
		}
		oldData.destroy();
	};

	void resize(size_t size) {
		if (size == m_size)
			return;
		if (size > m_capacity)
		{
			reserve(growthFactor * (size + m_capacity - 1));

			for (size_t i = m_size; i < size; i++)
			{
				if constexpr (std::is_move_assignable_v<T>) {
					m_data.emplace(std::move(T()), i * typeSize);
				}
				else if constexpr (std::is_copy_assignable_v<T>) {
					m_data.emplace(T(), i * typeSize);
				}
				else static_assert(false, "Key type must be copy or move assignable");
			}			
		}
		else if(size > m_size)
		{
			for (size_t i = m_size; i < size; i++)
			{
				if constexpr (std::is_move_assignable_v<T>) {
					m_data.emplace(std::move(T()), i * typeSize);
				}
				else if constexpr (std::is_copy_assignable_v<T>) {
					m_data.emplace(T(), i * typeSize);
				}
				else static_assert(false, "Key type must be copy or move assignable");
			}
		}
		else
		{
			for (size_t i = size; i < m_size; i++)
				m_data.erase<T>(i * typeSize);
		}
		m_size = size;
	};

	template<typename U>
	Iterator insert(U&& value)
	{
		if (m_size >= m_capacity)
			reserve(growthFactor * m_capacity);
		m_data.emplace(std::forward<U>(value), m_size * typeSize);
		return Iterator(m_data.data<T>() + m_size++);
	}

	Iterator erase(size_t index)
	{
		m_data.erase<T>(index * typeSize);
		for (size_t j = index + 1; j < m_size; j++)
		{
			m_data.emplace(std::move(*(m_data.get<T>(j * typeSize))), (j - 1) * typeSize);
			m_data.erase<T>(j * typeSize);
		}
		m_size--;
		return Iterator(m_data.data<T>() + index);
	}

	Iterator erase(const T& value)
	{
		for (size_t i = 0; i < m_size; i++)
		{
			if (*(m_data.get<T>(i * typeSize)) == value)
			{
				m_data.erase<T>(i * typeSize);
				for (size_t j = i + 1; j < m_size; j++)
				{
					m_data.emplace(std::move(*(m_data.get<T>(j * typeSize))), (j - 1) * typeSize);
					m_data.erase<T>(j * typeSize);
				}
				m_size--;
				return Iterator(m_data.data<T>() + i);
			}
		}

		return end();
	}

	Iterator find(const T& value)
	{
		for (size_t i = 0; i < m_size; i++)
		{
			if (*(m_data.get<T>(i * typeSize)) == value)
				return Iterator(m_data.data<T>() + i);
		}
		return end();
	}

	ConstIterator find(const T& value) const
	{
		for(size_t i = 0; i < m_size; i++)
		{
			if (*(m_data.get<T>(i * typeSize)) == value)
				return ConstIterator(m_data.data<T>() + i);
		}
		return cend();
	}

	Iterator begin() { return Iterator(m_data.data<T>()); }
	Iterator end() { return Iterator(m_data.data<T>() + m_size); }
	ConstIterator begin() const { return ConstIterator(m_data.data<T>()); }
	ConstIterator end() const { return ConstIterator(m_data.data<T>() + m_size); }
	ConstIterator cbegin() const { return ConstIterator(m_data.data<T>()); }
	ConstIterator cend() const { return ConstIterator(m_data.data<T>() + m_size); }
};