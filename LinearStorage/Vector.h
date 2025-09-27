#pragma once
#include "Utilities/ByteArray.h"
#include "Utilities/Concepts.h"
#include "Utilities/Macros.h"

#include <memory>
#include <stdexcept>
#include <vector>
#include <initializer_list>

namespace Containers {

	template <typename T, AllocatorConcept<T> Alloc>
	struct ReleaseData {
		T* ptr;
		size_t size;
		size_t capacity;
		Alloc allocator;

		void destroy() {
			for (size_t i = 0; i < size; ++i)
				allocator.destroy(ptr + i);
			allocator.deallocate(ptr, capacity);
		};
	};

	template <typename T, AllocatorConcept<T> Alloc = Allocator<T>,
		size_t s_initialCapacity = 16, float s_growthFactor = 1.618f>
	class Vector
	{
		static_assert(!std::is_reference_v<T>, "Vector does not support references");
		static_assert(std::is_destructible_v<T>, "Vector requires destructible types");
		static_assert(std::is_default_constructible_v<Alloc>, "Allocator must be default constructible");
		static_assert(std::is_destructible_v<Alloc>, "Allocator must be destructible");
		static_assert(s_initialCapacity > 0, "Initial capacity must be greater than 0");
		static_assert(s_growthFactor > 1.0f, "Growth factor must be greater than 1");

	public:

		using ValueType = T;
		using Iterator = ValueType*;
		using ConstIterator = const ValueType*;
		using SizeType = size_t;
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = SizeType;

	protected:
		T* m_data;
		size_t m_size = 0;
		size_t m_capacity = 0;
		Alloc m_allocator;

	public:
		Vector() : m_data(m_allocator.allocate(s_initialCapacity)), m_capacity(s_initialCapacity) {};
		Vector(size_t size) requires(std::is_default_constructible_v<T>)
			: m_data(m_allocator.allocate(size)), m_capacity(size), m_size(size) {			
			for (size_t i = 0; i < size; ++i)
				m_allocator.construct(m_data + i);
		};
		Vector(size_t size, const T& value) requires (std::is_copy_constructible_v<T>)
			: m_data(m_allocator.allocate(size)), m_capacity(size), m_size(size) {
			for (size_t i = 0; i < size; ++i)
				m_allocator.construct(m_data + i, value);
		};

		~Vector()
		{
			clear();
			m_allocator.deallocate(m_data, m_capacity);
		}

		Vector(std::initializer_list<T> list) requires(std::is_copy_constructible_v<T>)
			: m_capacity(list.size()), m_size(0),
			m_data(m_allocator.allocate(list.size())) {
			for (auto& elem : list)
				m_allocator.construct(m_data + m_size++, elem);
		};

		Vector& operator=(std::initializer_list<T> list) requires(std::is_copy_constructible_v<T>)
		{
			clear();
			m_allocator.deallocate(m_data, m_capacity);
			m_capacity = list.size();
			m_size = 0;
			m_data = m_allocator.allocate(m_capacity);
			for (auto& elem : list)
				m_allocator.construct(m_data + m_size++, elem);
			return *this;
		};

		Vector(const std::vector<T>& vector) requires(std::is_copy_constructible_v<T>)
			: m_capacity(vector.capacity()), m_size(0),
			m_data(m_allocator.allocate(vector.capacity()))
		{
			for (auto& elem : vector)
				m_allocator.construct(m_data + m_size++, elem);
		}

		Vector& operator=(const std::vector<T>& vector) requires(std::is_copy_constructible_v<T>)
		{
			clear();
			m_allocator.deallocate(m_data, m_capacity);
			m_capacity = vector.capacity();
			m_size = 0;
			m_data = m_allocator.allocate(m_capacity);
			for (auto& elem : vector)
				m_allocator.construct(m_data + m_size++, elem);
			return *this;
		}

		//cannot move vector since it can't release resources
		Vector(std::vector<T>&&) = delete;		
		Vector& operator=(std::vector<T>&&) = delete;

		Vector(const Vector& other) requires std::is_copy_constructible_v<T>
			: m_data(m_allocator.allocate(other.m_capacity)),
			m_capacity(other.m_capacity), m_size(other.m_size) {
			for (size_t i = 0; i < m_size; ++i)
				m_allocator.construct(m_data + i, other.m_data[i]);
		};

		Vector(Vector&& other) requires (std::is_move_constructible_v<Alloc>) : m_data(other.m_data),
			m_capacity(other.m_capacity), m_size(other.m_size), m_allocator(std::move(other.m_allocator)) {
			other.m_size = 0;
			other.m_allocator = Alloc();
			other.m_data = other.m_allocator.allocate(s_initialCapacity);
			other.m_capacity = s_initialCapacity;
		};

		Vector& operator=(const Vector& other) requires std::is_copy_constructible_v<T> {
			if (this == &other)
				return *this;
			clear();
			if (m_capacity < other.m_size)
				reserve(other.m_size * s_growthFactor);
			for (size_t i = 0; i < other.m_size; ++i)
				m_allocator.construct(m_data + i, other.m_data[i]);
			m_size = other.m_size;
			return *this;
		};

		Vector& operator=(Vector&& other) requires (std::is_move_assignable_v<Alloc>) 
		{
			if (this == &other)
				return *this;
			clear();
			m_allocator.deallocate(m_data, m_capacity);
			m_capacity = other.m_capacity;
			m_size = other.m_size;
			m_data = std::move(other.m_data);
			m_allocator = std::move(other.m_allocator);
			other.m_size = 0;
			other.m_allocator = Alloc();
			other.m_data = other.m_allocator.allocate(s_initialCapacity);
			other.m_capacity = s_initialCapacity;
			return *this;
		};

		inline T& operator[](size_t index) { return m_data[index]; };
		inline const T& operator[](size_t index) const { return m_data[index]; };
		inline T& at(size_t index) { CONTAINERS_VERIFY(index < m_size, "Index out of range"); return m_data[index]; }
		inline const T& at(size_t index) const { CONTAINERS_VERIFY(index < m_size, "Index out of range"); return m_data[index]; }
		inline const size_t size() const { return m_size; };
		inline const bool empty() const { return m_size == 0; };
		inline const size_t capacity() const { return m_capacity; };
		inline const T* data() const { return m_data; };
		inline T* data() { return m_data; };

		inline T& back() { return m_data[m_size - 1]; };
		inline const T& back() const { return m_data[m_size - 1]; };
		inline T& front() { return m_data[0]; };
		inline T& front() const { return m_data[0]; };

		inline Iterator begin() { return m_data; }
		inline Iterator end() { return m_data + m_size; }
		inline ConstIterator begin() const { return m_data; }
		inline ConstIterator end() const { return m_data + m_size; }
		inline ConstIterator cbegin() const { return begin(); }
		inline ConstIterator cend() const { return end(); }

		ReleaseData<T, Alloc> release() {
			ReleaseData<T, Alloc> data;
			data.ptr = m_data;
			data.size = m_size;
			data.capacity = m_capacity;
			data.allocator = m_allocator;
			m_allocator = Alloc();
			m_data = m_allocator.allocate(s_initialCapacity);
			m_size = 0;
			m_capacity = s_initialCapacity;
			return data;
		}

		void assign(T* data, size_t size) requires std::is_same_v<Allocator<T>, Alloc>
		{
			clear();
			m_allocator.deallocate(m_data, m_capacity);
			m_data = data;
			m_size = size;
			m_capacity = size;
		}

		void reserve(size_t capacity) requires
			(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>) {
			if (capacity == m_capacity)
				return;
			if (capacity < m_size)
				capacity = m_size;
			auto oldData = m_data;
			m_data = m_allocator.allocate(capacity);

			for (size_t i = 0; i < m_size; i++)
			{
				construct(m_data + i, oldData[i]);
				m_allocator.destroy(oldData + i);
			}
			m_allocator.deallocate(oldData, m_capacity);
			m_capacity = capacity;
		};

		void resize(size_t size) requires
			((std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>)
				&& std::is_default_constructible_v<T>)
		{
			if (size == m_size)
				return;
			if (size > m_capacity)
			{
				auto oldCapacity = m_capacity;
				auto oldData = m_data;

				m_capacity = s_growthFactor * (size + m_capacity - 1);				
				m_data = m_allocator.allocate(m_capacity);

				for (size_t i = 0; i < m_size; i++)
				{
					construct(m_data + i, oldData[i]);
					m_allocator.destroy(oldData + i);
				}
				m_allocator.deallocate(oldData, m_capacity);

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

		void clear() {
			if (m_size == 0)
				return;
			for (size_t i = 0; i < m_size; ++i)
				m_allocator.destroy(m_data + i);
			m_size = 0;
		};

		void pushBack(const T& data) requires (std::is_copy_constructible_v<T>)
		{
			if (m_size >= m_capacity)
				reserve(s_growthFactor * (m_capacity + 1));
			m_allocator.construct(m_data + m_size, data);
			++m_size;
		}

		void pushBack(T&& data) requires (std::is_move_constructible_v<T>)
		{
			if (m_size >= m_capacity)
				reserve(s_growthFactor * (m_capacity + 1));
			m_allocator.construct(m_data + m_size, std::move(data));
			++m_size;
		}

		void popBack()
		{
			CONTAINERS_VERIFY(m_size > 0, "Popping an empty Vector");
			m_allocator.destroy(m_data + --m_size);
		}

		template<typename... Args>
		void emplaceBack(Args&&... data) requires std::constructible_from<T, Args...>
		{
			if (m_size >= m_capacity)
				reserve(s_growthFactor * (m_capacity + 1));
			m_allocator.construct(m_data + m_size, std::forward<Args>(data)...);
		}

		template<typename ItType, typename... Args>
		Iterator emplace(ItType pos, Args&&... data) requires
			((std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>) &&
				(std::is_move_constructible_v<T> ||
					std::is_copy_constructible_v<T>) &&
				std::constructible_from<T, Args...>)
		{			
			if (m_size == 0)
			{
				m_allocator.construct(m_data, std::forward<Args>(data)...);
				m_size = 1;
				return pos;
			}

			if (m_size >= m_capacity)
			{
				size_t offset = pos - begin();
				auto newCapacity = s_growthFactor * (m_capacity + 1);
				auto newData = m_allocator.allocate(newCapacity);

				for (size_t i = 0; i < offset; ++i)
				{
					construct(newData + i, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				m_allocator.construct(newData + offset, std::forward<Args>(data)...);
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + i + 1, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				m_allocator.deallocate(m_data, m_capacity);
				m_data = newData;
				m_capacity = newCapacity;
				++m_size;
				return m_data + offset;
			}

			construct(m_data + m_size, m_data[m_size - 1]);
			Iterator it = begin() + m_size - 1;
			for (; it != pos; --it)
				assign(*it, *(it - 1));

			m_allocator.destroy(it);
			m_allocator.construct(it, std::forward<Args>(data)...);
			++m_size;
			return pos;
		}

		template<typename U, typename ItType>
		Iterator insert(ItType pos, U&& value) requires
			((std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>) &&
				(std::is_move_constructible_v<T> ||
					std::is_copy_constructible_v<T>) &&
				(std::is_move_assignable_v<T> ||
					std::is_copy_assignable_v<T>))
		{
			if (m_size == 0)
			{
				m_allocator.construct(m_data, std::forward<U>(value));
				m_size = 1;
				return pos;
			}

			if (m_size >= m_capacity)
			{
				size_t offset = pos - begin();
				auto newCapacity = s_growthFactor * (m_capacity + 1);
				auto newData = m_allocator.allocate(newCapacity);

				for (size_t i = 0; i < offset; ++i)
				{
					construct(newData + i, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				m_allocator.construct(newData + offset, std::forward<U>(value));
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + i + 1, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				m_allocator.deallocate(m_data, m_capacity);
				m_data = newData;
				m_capacity = newCapacity;
				++m_size;
				return m_data + offset;
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
			((std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>) &&
				(std::is_move_constructible_v<T> ||
					std::is_copy_constructible_v<T>) &&
				(std::is_move_assignable_v<T> ||
					std::is_copy_assignable_v<T>))
		{
			if (count == 0)
				return pos;

			if (m_size + count > m_capacity)
			{
				size_t offset = pos - begin();
				auto newCapacity = s_growthFactor * (m_capacity + count);
				auto newData = m_allocator.allocate(newCapacity);

				for (size_t i = 0; i < offset; ++i)
				{
					construct(newData + i, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				for (size_t i = 0; i < count; ++i)
					m_allocator.construct(newData + offset + i, std::forward<U>(value));
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + count + i, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				m_allocator.deallocate(m_data, m_capacity);
				m_data = newData;
				m_capacity = newCapacity;
				m_size += count;
				return m_data + offset;
			}

			Iterator it;
			if (m_size == 0)
			{
				for (it = pos; it < pos + count; ++it)
					m_allocator.construct(m_data, std::forward<U>(value));
				m_size = count;
				return pos;
			}

			for (it = end(); it < end() + count; ++it)
				m_allocator.construct(&(*it));

			size_t rightSize = end() - pos;
			for (; it > pos + count - 1; --it)
				assign(*it, *(it - count));

			it = pos;
			for (; it < pos + count; ++it)
				*it = std::forward<U>(value);

			m_size += count;
			return pos;
		}

		template<typename ItType, typename InputIt>
		Iterator insert(ItType pos, InputIt first, InputIt last) requires
			((std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>) &&
				(std::is_copy_constructible_v<T>) &&
				(std::is_copy_assignable_v<T>))
		{
			CONTAINERS_VERIFY(first <= last, "Invalid range");
			size_t count = last - first;
			if (count == 0)
				return pos;

			if (m_size + count > m_capacity)
			{
				size_t offset = pos - begin();
				auto newCapacity = s_growthFactor * (m_capacity + count);
				auto newData = m_allocator.allocate(newCapacity);

				for (size_t i = 0; i < offset; ++i)
				{
					construct(newData + i, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				for (size_t i = 0; i < count; ++i, ++first)
					m_allocator.construct(newData + offset + i, *first);
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + count + i, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				m_allocator.deallocate(m_data, m_capacity);
				m_data = newData;
				m_capacity = newCapacity;
				m_size += count;
				return m_data + offset;
			}

			Iterator it;
			if (m_size == 0)
			{
				for (it = pos; it < pos + count; ++it, ++first)
					m_allocator.construct(m_data, *first);
				m_size = count;
				return pos;
			}

			for (it = end(); it < end() + count; ++it)
				m_allocator.construct(&(*it));

			size_t rightSize = end() - pos;
			for (; it > pos + count - 1; --it)
				assign(*it, *(it - count));

			it = pos;
			for (; it < pos + count; ++it, ++first)
				*it = *first;

			m_size += count;
			return pos;
		}

		template<typename ItType>
		Iterator insert(ItType pos, std::initializer_list<T> list) requires
			((std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>) &&
				(std::is_move_constructible_v<T> ||
					std::is_copy_constructible_v<T>) &&
				(std::is_move_assignable_v<T> ||
					std::is_copy_assignable_v<T>))
		{
			size_t count = list.size();
			if (count == 0)
				return pos;
			if (m_size + count > m_capacity)
			{
				size_t offset = pos - begin();
				auto newCapacity = s_growthFactor * (m_capacity + count);
				auto newData = m_allocator.allocate(newCapacity);

				for (size_t i = 0; i < offset; ++i)
				{
					construct(newData + i, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				auto listIt = list.begin();
				for (size_t i = 0; i < count; ++i, ++listIt)
					m_allocator.construct(newData + offset + i, *listIt);
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + count + i, m_data[i]);
					m_allocator.destroy(m_data + i);
				}
				m_allocator.deallocate(m_data, m_capacity);
				m_data = newData;
				m_capacity = newCapacity;
				m_size += count;
				return m_data + offset;
			}

			Iterator it;
			if (m_size == 0)
			{
				auto listIt = list.begin();
				for (it = pos; it < pos + count; ++it, ++listIt)
					m_allocator.construct(m_data, *listIt);
				m_size = count;
				return pos;
			}

			for (it = end(); it < end() + count; ++it)
				m_allocator.construct(&(*it));

			size_t rightSize = end() - pos;
			for (; it > pos + count - 1; --it)
				assign(*it, *(it - count));

			it = pos;
			auto listIt = list.begin();
			for (; it < pos + count; ++it, ++listIt)
				*it = *listIt;

			m_size += count;
			return pos;
		}

		template <typename ItType>
		Iterator erase(ItType pos) requires
			((std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>) &&
				(std::is_move_assignable_v<T> ||
					std::is_copy_assignable_v<T>))
		{
			for (Iterator it = pos; it < end() - 1; ++it)
				assign(*it, *(it + 1));
			m_size--;
			m_allocator.destroy(m_data + m_size);
			return pos;
		}

		template <typename ItType>
		Iterator erase(ItType first, ItType last) requires
			((std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>) &&
				(std::is_move_assignable_v<T> ||
					std::is_copy_assignable_v<T>))
		{
			CONTAINERS_VERIFY(first <= last, "Invalid range");
			Iterator it = first;
			size_t span = last - first;
			for (; it < end() - span; ++it)
			{
				assign(*it, *(it + span));
				m_allocator.destroy(it + span);
			}

			for (; it < last; ++it)
				m_allocator.destroy(it);

			m_size -= span;
			return first;
		}

		void shrinkToFit() {
			if (m_size == m_capacity)
				return;
			auto oldData = m_data;
			m_data = m_allocator.allocate(m_size);
			for (size_t i = 0; i < m_size; i++)
			{
				construct(m_data + i, oldData[i]);
				m_allocator.destroy(oldData + i);
			}
			m_allocator.deallocate(oldData, m_capacity);
			m_capacity = m_size;
		}

	protected:

		void construct(T* ptr, T& value) requires (
			std::is_move_constructible_v<T> ||
			std::is_copy_constructible_v<T>)
		{
			if constexpr (std::is_move_constructible_v<T>) {
				m_allocator.construct(ptr, std::move(value));
			}
			else if constexpr (std::is_copy_constructible_v<T>) {
				m_allocator.construct(ptr, value);
			}
			else static_assert(false, "Value type must be copy or move constructible");
		}

		void assign(T& left, T& right)requires (
			std::is_move_assignable_v<T> ||
			std::is_copy_assignable_v<T>)
			{
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