#pragma once
#include "../Utilities/ByteArray.h"
#include "../Utilities/Concepts.h"
#include "../Utilities/Macros.h"

#include <memory>
#include <stdexcept>
#include <initializer_list>
#include <vector>

#include "InitializerList.h"
#include "Span.h"
#include "../Memory/UniquePtr.h"

namespace Containers {

	// Fixed-capacity dynamic array, does not allocate memory dynamically
	template <typename T, size_t s_capacity, LifetimeManagerConcept<T> Life = LifetimeManager<T>>
	class DynamicArray
	{
		static_assert(!std::is_reference_v<T>, "Vector does not support references");
		static_assert(std::is_destructible_v<T>, "Vector requires destructible types");

		static_assert(std::is_default_constructible_v<Life>, "LifetimeManager must be default constructible");
		static_assert(std::is_destructible_v<Life>, "LifetimeManager must be destructible");

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
		uint8_t[s_capacity * sizeof(T)] m_data;
		size_t m_size = 0;
		[[no_unique_address]] Life m_lifetimeManager;

	public:
		DynamicArray() = default;

		DynamicArray(size_t size) requires(std::is_default_constructible_v<T>) : m_size(size) {
			for (size_t i = 0; i < size; ++i)
				m_lifetimeManager.construct(getAt(i));
		};
		DynamicArray(size_t size, const T& value) 
			requires (std::is_copy_constructible_v<T>) : m_size(size) {
			for (size_t i = 0; i < size; ++i)
				m_lifetimeManager.construct(getAt(i), value);
		};

		~DynamicArray()
		{
			clear();
		}

		DynamicArray(InitializerList<T> list) 
			requires(std::is_copy_constructible_v<T>) : m_size(0) {
			for (auto& elem : list)
				m_lifetimeManager.construct(getAt(m_size++), elem);
		};

		DynamicArray& operator=(InitializerList<T> list) requires(std::is_copy_constructible_v<T>)
		{
			clear();
			for (auto& elem : list)
				m_lifetimeManager.construct(getAt(m_size++), elem);
			return *this;
		};

		DynamicArray(const DynamicArray& other) requires std::is_copy_constructible_v<T> 
			: m_size(other.m_size), m_lifetimeManager(other.m_lifetimeManager) {
			for (size_t i = 0; i < m_size; ++i)
				m_lifetimeManager.construct(getAt(i), other[i]);
		};

		DynamicArray(DynamicArray&& other) requires std::is_move_constructible_v<Life> 
			: m_size(other.m_size), m_lifetimeManager(std::move(other.m_lifetimeManager)) {
			other.m_size = 0;
			for(size_t i = 0; i < m_size; ++i)
			{
				m_lifetimeManager.construct(getAt(i), std::move(other[i]));
				m_lifetimeManager.destroy(other.getAt(i));
			}
			other.m_lifetimeManager = Life();
		};

		DynamicArray& operator=(const DynamicArray& other) requires std::is_copy_constructible_v<T> {
			if (this == &other)
				return *this;
			clear();
			m_size = m_size;
			for (size_t i = 0; i < m_size; ++i)
				m_lifetimeManager.construct(getAt(i), other.getAt(i));
			return *this;
		};

		DynamicArray& operator=(DynamicArray&& other) requires (std::is_move_assignable_v<Life>)
		{
			if (this == &other)
				return *this;
			clear();
									
			for (size_t i = 0; i < m_size; ++i)
			{
				m_lifetimeManager.destroy(getAt(i));
			}

			m_size = other.m_size;
			m_lifetimeManager = std::move(other.m_lifetimeManager);

			for (size_t i = 0; i < m_size; ++i)
			{
				m_lifetimeManager.construct(getAt(i), std::move(other[i]));
				m_lifetimeManager.destroy(other.getAt(i));
			}

			other.m_size = 0;
			other.m_lifetimeManager = Life();
			return *this;
		};

		inline T& operator[](size_t index) { return *getAt(index); };
		inline const T& operator[](size_t index) const { return *getAt(index); };
		inline T& at(size_t index) { CONTAINERS_VERIFY(index < m_size, "Index out of range"); return *getAt(index); }
		inline const T& at(size_t index) const { CONTAINERS_VERIFY(index < m_size, "Index out of range"); return *getAt(index); }
		inline const size_t size() const { return m_size; };
		inline const bool empty() const { return m_size == 0; };
		inline const size_t capacity() const { return s_capacity; };
		inline const T* data() const { return m_data; };
		inline T* data() { return m_data; };

		inline T& back() { return *getAt(m_size - 1); };
		inline const T& back() const { return *getAt(m_size - 1); };
		inline T& front() { return *getAt(0); };
		inline T& front() const { return *getAt(0); };

		inline Iterator begin() { return getAt(0); }
		inline Iterator end() { return getAt(m_size); }
		inline ConstIterator begin() const { return getAt(0); }
		inline ConstIterator end() const { return getAt(m_size); }
		inline ConstIterator cbegin() const { return begin(); }
		inline ConstIterator cend() const { return end(); }

		void resize(size_t size, T arg = T()) requires
			((std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>)
				&& std::is_default_constructible_v<T>)
		{
			if (size == m_size)
				return;
			if (size > m_capacity)
				throw std::out_of_range("DynamicArray resize exceeds capacity");
			else if (size > m_size)
			{
				for (size_t i = m_size; i < size; i++)
					m_lifetimeManager.construct(getAt(i), arg);
			}
			else
			{
				for (size_t i = size; i < m_size; i++)
					m_lifetimeManager.destroy(getAt(i));
			}
			m_size = size;
		};

		void clear() {
			for (size_t i = 0; i < m_size; ++i)
				m_lifetimeManager.destroy(getAt(i));
			m_size = 0;
		};

		void pushBack(const T& data) requires (std::is_copy_constructible_v<T>)
		{
			CONTAINERS_VERIFY(m_size <= m_capacity, "DynamicArray pushBack exceeds capacity");
			m_lifetimeManager.construct(getAt(m_size++), data);
		}

		void pushBack(T&& data) requires (std::is_move_constructible_v<T>)
		{
			CONTAINERS_VERIFY(m_size <= m_capacity, "DynamicArray pushBack exceeds capacity");
			m_lifetimeManager.construct(getAt(m_size++), std::move(data));
		}

		void popBack()
		{
			CONTAINERS_VERIFY(m_size > 0, "Popping an empty Vector");
			m_lifetimeManager.destroy(getAt(--m_size));
		}

		template<typename... Args>
		bool emplaceBack(Args&&... data) requires std::constructible_from<T, Args...>
		{
			CONTAINERS_VERIFY(m_size <= m_capacity, "DynamicArray pushBack exceeds capacity");
			m_lifetimeManager.construct(getAt(m_size++), std::forward<Args>(data)...);
		}

		//template<typename ItType, typename... Args>
		//Iterator emplace(ItType pos, Args&&... data) requires
		//	((std::is_same_v<ItType, Iterator> ||
		//		std::is_same_v<ItType, ConstIterator>) &&
		//		(std::is_move_constructible_v<T> ||
		//			std::is_copy_constructible_v<T>) &&
		//		std::constructible_from<T, Args...>)
		//{
		//	if (m_size == 0)
		//	{
		//		m_lifetimeManager.construct(m_data, std::forward<Args>(data)...);
		//		m_size = 1;
		//		return pos;
		//	}

		//	if (m_size >= m_capacity)
		//	{
		//		size_t offset = pos - begin();
		//		auto newCapacity = s_growthFactor * (m_capacity + 1);
		//		auto newData = m_allocator.allocate(newCapacity);

		//		for (size_t i = 0; i < offset; ++i)
		//		{
		//			construct(newData + i, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		m_lifetimeManager.construct(newData + offset, std::forward<Args>(data)...);
		//		for (size_t i = offset; i < m_size; ++i)
		//		{
		//			construct(newData + i + 1, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		m_allocator.deallocate(m_data, m_capacity);
		//		m_data = newData;
		//		m_capacity = newCapacity;
		//		++m_size;
		//		return m_data + offset;
		//	}

		//	construct(m_data + m_size, m_data[m_size - 1]);
		//	Iterator it = begin() + m_size - 1;
		//	for (; it != pos; --it)
		//		assign(*it, *(it - 1));

		//	m_lifetimeManager.destroy(it);
		//	m_lifetimeManager.construct(it, std::forward<Args>(data)...);
		//	++m_size;
		//	return pos;
		//}

		//template<typename U, typename ItType>
		//Iterator insert(ItType pos, U&& value) requires
		//	((std::is_same_v<ItType, Iterator> ||
		//		std::is_same_v<ItType, ConstIterator>) &&
		//		(std::is_move_constructible_v<T> ||
		//			std::is_copy_constructible_v<T>) &&
		//		(std::is_move_assignable_v<T> ||
		//			std::is_copy_assignable_v<T>))
		//{
		//	if (m_size == 0)
		//	{
		//		m_lifetimeManager.construct(m_data, std::forward<U>(value));
		//		m_size = 1;
		//		return pos;
		//	}

		//	if (m_size >= m_capacity)
		//	{
		//		size_t offset = pos - begin();
		//		auto newCapacity = s_growthFactor * (m_capacity + 1);
		//		auto newData = m_allocator.allocate(newCapacity);

		//		for (size_t i = 0; i < offset; ++i)
		//		{
		//			construct(newData + i, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		m_lifetimeManager.construct(newData + offset, std::forward<U>(value));
		//		for (size_t i = offset; i < m_size; ++i)
		//		{
		//			construct(newData + i + 1, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		m_allocator.deallocate(m_data, m_capacity);
		//		m_data = newData;
		//		m_capacity = newCapacity;
		//		++m_size;
		//		return m_data + offset;
		//	}

		//	construct(m_data + m_size, m_data[m_size - 1]);
		//	Iterator it = begin() + m_size - 1;
		//	for (; it != pos; --it)
		//		assign(*it, *(it - 1));

		//	*it = std::forward<U>(value);
		//	++m_size;
		//	return it;
		//}

		//template<typename U, typename ItType>
		//Iterator insert(ItType pos, size_t count, U&& value) requires
		//	((std::is_same_v<ItType, Iterator> ||
		//		std::is_same_v<ItType, ConstIterator>) &&
		//		(std::is_move_constructible_v<T> ||
		//			std::is_copy_constructible_v<T>) &&
		//		(std::is_move_assignable_v<T> ||
		//			std::is_copy_assignable_v<T>))
		//{
		//	if (count == 0)
		//		return pos;

		//	if (m_size + count > m_capacity)
		//	{
		//		size_t offset = pos - begin();
		//		auto newCapacity = s_growthFactor * (m_capacity + count);
		//		auto newData = m_allocator.allocate(newCapacity);

		//		for (size_t i = 0; i < offset; ++i)
		//		{
		//			construct(newData + i, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		for (size_t i = 0; i < count; ++i)
		//			m_lifetimeManager.construct(newData + offset + i, std::forward<U>(value));
		//		for (size_t i = offset; i < m_size; ++i)
		//		{
		//			construct(newData + count + i, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		m_allocator.deallocate(m_data, m_capacity);
		//		m_data = newData;
		//		m_capacity = newCapacity;
		//		m_size += count;
		//		return m_data + offset;
		//	}

		//	Iterator it;
		//	if (m_size == 0)
		//	{
		//		for (it = pos; it < pos + count; ++it)
		//			m_lifetimeManager.construct(m_data, std::forward<U>(value));
		//		m_size = count;
		//		return pos;
		//	}

		//	for (it = end(); it < end() + count; ++it)
		//		m_lifetimeManager.construct(&(*it));

		//	size_t rightSize = end() - pos;
		//	for (; it > pos + count - 1; --it)
		//		assign(*it, *(it - count));

		//	it = pos;
		//	for (; it < pos + count; ++it)
		//		*it = std::forward<U>(value);

		//	m_size += count;
		//	return pos;
		//}

		//template<typename ItType, typename InputIt>
		//Iterator insert(ItType pos, InputIt first, InputIt last) requires
		//	((std::is_same_v<ItType, Iterator> ||
		//		std::is_same_v<ItType, ConstIterator>) &&
		//		(std::is_copy_constructible_v<T>) &&
		//		(std::is_copy_assignable_v<T>))
		//{
		//	CONTAINERS_VERIFY(first <= last, "Invalid range");
		//	size_t count = last - first;
		//	if (count == 0)
		//		return pos;

		//	if (m_size + count > m_capacity)
		//	{
		//		size_t offset = pos - begin();
		//		auto newCapacity = s_growthFactor * (m_capacity + count);
		//		auto newData = m_allocator.allocate(newCapacity);

		//		for (size_t i = 0; i < offset; ++i)
		//		{
		//			construct(newData + i, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		for (size_t i = 0; i < count; ++i, ++first)
		//			m_lifetimeManager.construct(newData + offset + i, *first);
		//		for (size_t i = offset; i < m_size; ++i)
		//		{
		//			construct(newData + count + i, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		m_allocator.deallocate(m_data, m_capacity);
		//		m_data = newData;
		//		m_capacity = newCapacity;
		//		m_size += count;
		//		return m_data + offset;
		//	}

		//	Iterator it;
		//	if (m_size == 0)
		//	{
		//		for (it = pos; it < pos + count; ++it, ++first)
		//			m_lifetimeManager.construct(m_data, *first);
		//		m_size = count;
		//		return pos;
		//	}

		//	for (it = end(); it < end() + count; ++it)
		//		m_lifetimeManager.construct(&(*it));

		//	size_t rightSize = end() - pos;
		//	for (; it > pos + count - 1; --it)
		//		assign(*it, *(it - count));

		//	it = pos;
		//	for (; it < pos + count; ++it, ++first)
		//		*it = *first;

		//	m_size += count;
		//	return pos;
		//}

		//template<typename ItType>
		//Iterator insert(ItType pos, std::initializer_list<T> list) requires
		//	((std::is_same_v<ItType, Iterator> ||
		//		std::is_same_v<ItType, ConstIterator>) &&
		//		(std::is_move_constructible_v<T> ||
		//			std::is_copy_constructible_v<T>) &&
		//		(std::is_move_assignable_v<T> ||
		//			std::is_copy_assignable_v<T>))
		//{
		//	size_t count = list.size();
		//	if (count == 0)
		//		return pos;
		//	if (m_size + count > m_capacity)
		//	{
		//		size_t offset = pos - begin();
		//		auto newCapacity = s_growthFactor * (m_capacity + count);
		//		auto newData = m_allocator.allocate(newCapacity);

		//		for (size_t i = 0; i < offset; ++i)
		//		{
		//			construct(newData + i, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		auto listIt = list.begin();
		//		for (size_t i = 0; i < count; ++i, ++listIt)
		//			m_lifetimeManager.construct(newData + offset + i, *listIt);
		//		for (size_t i = offset; i < m_size; ++i)
		//		{
		//			construct(newData + count + i, m_data[i]);
		//			m_lifetimeManager.destroy(m_data + i);
		//		}
		//		m_allocator.deallocate(m_data, m_capacity);
		//		m_data = newData;
		//		m_capacity = newCapacity;
		//		m_size += count;
		//		return m_data + offset;
		//	}

		//	Iterator it;
		//	if (m_size == 0)
		//	{
		//		auto listIt = list.begin();
		//		for (it = pos; it < pos + count; ++it, ++listIt)
		//			m_lifetimeManager.construct(m_data, *listIt);
		//		m_size = count;
		//		return pos;
		//	}

		//	for (it = end(); it < end() + count; ++it)
		//		m_lifetimeManager.construct(&(*it));

		//	size_t rightSize = end() - pos;
		//	for (; it > pos + count - 1; --it)
		//		assign(*it, *(it - count));

		//	it = pos;
		//	auto listIt = list.begin();
		//	for (; it < pos + count; ++it, ++listIt)
		//		*it = *listIt;

		//	m_size += count;
		//	return pos;
		//}

		//template <typename ItType>
		//Iterator erase(ItType pos) requires
		//	((std::is_same_v<ItType, Iterator> ||
		//		std::is_same_v<ItType, ConstIterator>) &&
		//		(std::is_move_assignable_v<T> ||
		//			std::is_copy_assignable_v<T>))
		//{
		//	for (Iterator it = pos; it < end() - 1; ++it)
		//		assign(*it, *(it + 1));
		//	m_size--;
		//	m_lifetimeManager.destroy(m_data + m_size);
		//	return pos;
		//}

		//template <typename ItType>
		//Iterator erase(ItType first, ItType last) requires
		//	((std::is_same_v<ItType, Iterator> ||
		//		std::is_same_v<ItType, ConstIterator>) &&
		//		(std::is_move_assignable_v<T> ||
		//			std::is_copy_assignable_v<T>))
		//{
		//	CONTAINERS_VERIFY(first <= last, "Invalid range");
		//	Iterator it = first;
		//	size_t span = last - first;
		//	for (; it < end() - span; ++it)
		//	{
		//		assign(*it, *(it + span));
		//		m_lifetimeManager.destroy(it + span);
		//	}

		//	for (; it < last; ++it)
		//		m_lifetimeManager.destroy(it);

		//	m_size -= span;
		//	return first;
		//}

		bool operator==(const DynamicArray& other) const {
			if (m_size != other.m_size)
				return false;
			for (size_t i = 0; i < m_size; i++)
				if (*getAt(i) != *other.getAt(i))
					return false;
			return true;
		}

		Span<T> subSpan(size_t offset, size_t len) {
			return Span<T>(getAt(offset), len);
		}

		Span<const T> subSpan(size_t offset, size_t len) const {
			return Span<const T>(getAt(offset), len);
		}

	protected:

		T* getAt(size_t index) {
			return reinterpret_cast<T*>(m_data) + index;
		}

		void construct(T* ptr, T& value) requires (
			std::is_move_constructible_v<T> ||
			std::is_copy_constructible_v<T>)
		{
			if constexpr (std::is_move_constructible_v<T>) {
				m_lifetimeManager.construct(ptr, std::move(value));
			}
			else if constexpr (std::is_copy_constructible_v<T>) {
				m_lifetimeManager.construct(ptr, value);
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