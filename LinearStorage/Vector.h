#pragma once
#include "Utilities/ByteArray.h"
#include "Utilities/Allocator.h"

#include <memory>
#include <stdexcept>
#include <vector>
#include <initializer_list>

namespace Containers {
// WARNING: This Vector is NOT compatible with std::vector by design
// Use Vector as a complete replacement, not alongside std::vector
// Provides ownership transfer via release() which std::vector cannot do

	template <typename T, typename Alloc = Allocator<T>>
	class Vector
	{
	public:
		static constexpr float growthFactor = 1.618f;
		static constexpr size_t initialCapacity = 16;

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
		T* m_data;
		size_t m_size = 0;
		size_t m_capacity = 0;
		Alloc m_allocator;

	public:
		Vector() : m_data(m_allocator.allocate(initialCapacity)), m_capacity(initialCapacity) {};
		Vector(size_t size) : m_data(m_allocator.allocate(initialCapacity)),
			m_capacity(initialCapacity), m_size(0) {
			resize(size);
		};
		Vector(size_t size, const T& value) : m_data(m_allocator.allocate(initialCapacity)),
			m_capacity(initialCapacity), m_size(0) {
			if (size > m_capacity)
				reserve(growthFactor * (size + m_capacity - 1));
			m_size = size;
			for (size_t i = 0; i < size; ++i)
				m_allocator.construct(m_data + i, value);
		};

		~Vector()
		{
			clear();
			m_allocator.deallocate(m_data);
		}

		Vector(std::initializer_list<T> list) : m_capacity(list.size()), m_size(0),
			m_data(m_allocator.allocate(list.size())) {
			for (auto& elem : list)
				m_allocator.construct(m_data + m_size++, elem);
		};

		Vector& operator=(std::initializer_list<T> list) {
			clear();
			m_allocator.deallocate(m_data);
			m_capacity = list.size();
			m_size = 0;
			m_data = m_allocator.allocate(m_capacity);
			for (auto& elem : list)
				m_allocator.construct(m_data + m_size++, elem);
			return *this;
		};

		// EXPLICITLY DELETED: No std::vector interoperability
		Vector(const std::vector<T>&) = delete;
		Vector(std::vector<T>&&) = delete;
		Vector& operator=(const std::vector<T>&) = delete;
		Vector& operator=(std::vector<T>&&) = delete;

		Vector(const Vector& other) requires std::is_copy_constructible_v<T> :
		m_data(m_allocator.allocate(other.m_capacity)), m_capacity(other.m_capacity), m_size(other.m_size) {
			for (size_t i = 0; i < m_size; ++i)
				m_allocator.construct(m_data + i, other.m_data[i]);
		};

		Vector(Vector&& other) : m_data(std::move(other.m_data)),
			m_capacity(other.m_capacity), m_size(other.m_size), m_allocator(std::move(other.m_allocator)) {
			other.m_capacity = 0;
			other.m_size = 0;
			other.m_allocator = Alloc();
		};

		Vector& operator=(const Vector& other) requires std::is_copy_constructible_v<T> {
			if (this == &other)
				return *this;
			clear();
			m_allocator.deallocate(m_data);
			m_capacity = other.m_capacity;
			m_size = other.m_size;
			m_data = m_allocator.allocate(m_capacity);
			for (size_t i = 0; i < m_size; ++i)
				m_allocator.construct(m_data + i, other.m_data[i]);
			return *this;
		};

		Vector& operator=(Vector&& other) {
			if (this == &other)
				return *this;
			clear();
			m_allocator.deallocate(m_data);
			m_capacity = other.m_capacity;
			m_size = other.m_size;
			m_data = std::move(other.m_data);
			m_allocator = std::move(other.m_allocator);
			other.m_capacity = 0;
			other.m_size = 0;
			other.m_allocator = Alloc();
			return *this;
		};

		void clear() {
			if (m_size == 0)
				return;
			for (size_t i = 0; i < m_size; ++i)
				m_allocator.destroy(m_data + i);
			m_size = 0;
		};

		template<typename U>
		void pushBack(U&& data)
		{
			if (m_size >= m_capacity)
				reserve(growthFactor * (m_capacity + 1));

			m_allocator.construct(m_data + m_size, std::forward<U>(data));
			++m_size;
		}

		void popBack()
		{
			if (m_size == 0)
				throw std::runtime_error("Vector is empty");
			--m_size;
			m_allocator.destroy(m_data + m_size);
		}

		template<typename... Args>
		Iterator emplaceBack(Args&&... data)
		{
			if (m_size >= m_capacity)
				reserve(growthFactor * (m_capacity + 1));

			m_allocator.construct(m_data + m_size, std::forward<Args>(data)...);
			return Iterator(m_data + m_size++);
		}

		template<typename ItType, typename... Args>
		Iterator emplace(ItType pos, Args&&... data) requires
			std::is_same_v<ItType, Iterator> ||
			std::is_same_v<ItType, ConstIterator>
		{
			if (m_size >= m_capacity)
			{
				size_t offset = pos - begin();
				reserve(growthFactor * (m_capacity + 1));
				pos = begin() + offset;
			}

			if (m_size == 0)
			{
				m_allocator.construct(m_data, std::forward<Args>(data)...);
				m_size = 1;
				return pos;
			}

			construct(m_data + m_size, m_data[m_size - 1]);
			Iterator it = begin() + m_size - 1;
			for (; it != pos; --it)
				assign(*it, *(it - 1));

			m_allocator.destroy(&(*it));
			m_allocator.construct(&(*it), std::forward<Args>(data)...);
			++m_size;
			return pos;
		}

		T& operator[](size_t index) { return m_data[index]; };
		const T& operator[](size_t index) const { return m_data[index]; };
		const size_t size() const { return m_size; };
		const bool empty() const { return m_size == 0; };
		const size_t capacity() const { return m_capacity; };
		const T* data() const { return m_data; };

		T& back() { return m_data[m_size - 1]; };
		const T& back() const { return m_data[m_size - 1]; };
		T& front() { return m_data[0]; };
		const T& front() const { return m_data[0]; };

		// OWNERSHIP TRANSFER: What std::vector cannot do
		struct ReleaseData {
			T* ptr;
			size_t size;
			size_t capacity;
			Alloc allocator;
		};

		ReleaseData release() {
			ReleaseData data;
			data.ptr = m_data;
			data.size = m_size;
			data.capacity = m_capacity;
			data.allocator = std::move(m_allocator);

			m_allocator = Alloc();
			m_data = m_allocator.allocate(initialCapacity);
			m_size = 0;
			m_capacity = initialCapacity;
			return data;
		}

		void reserve(size_t capacity) {
			if (capacity == m_capacity)
				return;
			auto oldData = m_data;
			m_data = m_allocator.allocate(capacity);

			m_capacity = capacity;
			if (m_capacity < m_size)
				m_capacity = m_size;

			for (size_t i = 0; i < m_size; i++)
			{
				construct(m_data + i, oldData[i]);
				m_allocator.destroy(oldData + i);
			}
			m_allocator.deallocate(oldData);
		};

		void resize(size_t size) {
			if (size == m_size)
				return;
			if (size > m_capacity)
			{
				reserve(growthFactor * (size + m_capacity - 1));

				for (size_t i = m_size; i < size; i++)
					m_allocator.construct(m_data + i);
			}
			else if (size > m_size)
			{
				for (size_t i = m_size; i < size; i++)
					m_allocator.construct(m_data + i);
			}
			else
			{
				for (size_t i = size; i < m_size; i++)
					m_allocator.destroy(m_data + i);
			}
			m_size = size;
		};

		template<typename U, typename ItType>
		Iterator insert(ItType pos, U&& value) requires
			std::is_same_v<ItType, Iterator> ||
			std::is_same_v<ItType, ConstIterator>
		{
			if (m_size >= m_capacity)
			{
				size_t offset = pos - begin();
				reserve(growthFactor * (m_capacity + 1));
				pos = begin() + offset;
			}

			if (m_size == 0)
			{
				m_allocator.construct(m_data, std::forward<U>(value));
				m_size = 1;
				return pos;
			}

			construct(m_data + m_size, m_data[m_size - 1]);
			Iterator it = begin() + m_size - 1;
			for (; it != pos; --it)
				assign(*it, *(it - 1));

			*it = std::forward<U>(value);
			++m_size;
			return it;
		}

		template<typename U, typename ItType>
		Iterator insert(ItType pos, size_t count, U&& value) requires
			std::is_same_v<ItType, Iterator> ||
			std::is_same_v<ItType, ConstIterator>
		{
			if (count == 0)
				return Iterator(pos);

			if (m_size + count > m_capacity)
			{
				size_t offset = pos - begin();
				reserve(growthFactor * (m_capacity + count));
				pos = begin() + offset;
			}

			if (m_size == 0)
			{
				for (Iterator it = pos; it < pos + count; ++it)
					m_allocator.construct(m_data, std::forward<U>(value));
				m_size = count;
				return pos;
			}

			Iterator it = end();
			for (; it < end() + count; ++it)
				m_allocator.construct(&(*it));

			size_t rightSize = end() - pos;
			for (; it > pos + count - 1; --it)
				assign(*it, *(it - count));

			it = pos;
			for (; it < pos + count; ++it)
				*it = std::forward<U>(value);

			m_size += count;
			return Iterator(pos);
		}

		template<typename ItType, typename InputIt>
		Iterator insert(ItType pos, InputIt first, InputIt last) requires
			std::is_same_v<ItType, Iterator> ||
			std::is_same_v<ItType, ConstIterator>
		{
			if (first > last)
				throw std::runtime_error("Invalid range");
			size_t count = last - first;
			if (count == 0)
				return Iterator(pos);
			if (m_size + count > m_capacity)
			{
				size_t offset = pos - begin();
				reserve(growthFactor * (m_capacity + count));
				pos = begin() + offset;
			}

			if (m_size == 0)
			{
				for (Iterator it = pos; it < pos + count; ++it, ++first)
					m_allocator.construct(m_data, *first);
				m_size = count;
				return pos;
			}

			Iterator it = end();
			for (; it < end() + count; ++it)
				m_allocator.construct(&(*it));

			size_t rightSize = end() - pos;
			for (; it > pos + count - 1; --it)
				assign(*it, *(it - count));

			it = pos;
			for (; it < pos + count; ++it, ++first)
				*it = *first;

			m_size += count;
			return Iterator(pos);
		}

		template<typename ItType>
		Iterator insert(ItType pos, std::initializer_list<T> list) requires
			std::is_same_v<ItType, Iterator> ||
			std::is_same_v<ItType, ConstIterator>
		{
			size_t count = list.size();
			if (count == 0)
				return Iterator(pos);
			if (m_size + count > m_capacity)
			{
				size_t offset = pos - begin();
				reserve(growthFactor * (m_capacity + count));
				pos = begin() + offset;
			}

			if (m_size == 0)
			{
				auto listIt = list.begin();
				for (Iterator it = pos; it < pos + count; ++it, ++listIt)
					m_allocator.construct(m_data, *listIt);
				m_size = count;
				return pos;
			}

			Iterator it = end();
			for (; it < end() + count; ++it)
				m_allocator.construct(&(*it));

			size_t rightSize = end() - pos;
			for (; it > pos + count - 1; --it)
				assign(*it, *(it - count));

			it = pos;
			auto listIt = list.begin();
			for (; it < pos + count; ++it, ++listIt)
				*it = *listIt;

			m_size += count;
			return Iterator(pos);
		}

		template <typename ItType>
		Iterator erase(ItType pos) requires
			std::is_same_v<ItType, Iterator> ||
			std::is_same_v<ItType, ConstIterator>
		{
			Iterator it = pos;
			for (; it < end() - 1; ++it)
				assign(*it, *(it + 1));
			m_size--;
			m_allocator.destroy(m_data + m_size);
			return Iterator(pos);
		}

		template <typename ItType>
		Iterator erase(ItType first, ItType last) requires
			std::is_same_v<ItType, Iterator> ||
			std::is_same_v<ItType, ConstIterator>
		{
			if (first > last)
				throw std::runtime_error("Invalid range");
			Iterator it = first;
			size_t span = last - first;
			for (; it < end() - span; ++it)
			{
				assign(*it, *(it + span));
				m_allocator.destroy(&(*(it + span)));
			}

			for (; it < last; ++it)
				m_allocator.destroy(&(*(it)));

			m_size -= span;
			return Iterator(first);
		}

		Iterator find(const T& value)
		{
			for (size_t i = 0; i < m_size; i++)
			{
				if (m_data[i] == value)
					return Iterator(m_data + i);
			}
			return end();
		}

		ConstIterator find(const T& value) const
		{
			for (size_t i = 0; i < m_size; i++)
			{
				if (m_data[i] == value)
					return ConstIterator(m_data + i);
			}
			return cend();
		}

		Iterator begin() { return Iterator(m_data); }
		Iterator end() { return Iterator(m_data + m_size); }
		ConstIterator begin() const { return ConstIterator(m_data); }
		ConstIterator end() const { return ConstIterator(m_data + m_size); }
		ConstIterator cbegin() const { return ConstIterator(m_data); }
		ConstIterator cend() const { return ConstIterator(m_data + m_size); }

	private:

		void construct(T* ptr, T& value) {
			if constexpr (std::is_move_constructible_v<T>) {
				m_allocator.construct(ptr, std::move(value));
			}
			else if constexpr (std::is_copy_constructible_v<T>) {
				m_allocator.construct(ptr, value);
			}
			else static_assert(false, "Value type must be copy or move constructible");
		}

		void assign(T& left, T& right) {
			if constexpr (std::is_move_assignable_v<T>) {
				left = std::move(right);
			}
			else if constexpr (std::is_copy_assignable_v<T>) {
				left = right;
			}
			else static_assert(false, "Value type must be copy or move assignable");
		}
	};
}