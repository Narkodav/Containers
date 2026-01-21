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

	template<typename T, TypedAllocatorConcept<T> Alloc, LifetimeManagerConcept<T> Life>
	struct ReleaseDeleter {
		size_t m_size = 0;
		size_t m_capacity = 0;
		[[no_unique_address]] Alloc m_allocator;
		[[no_unique_address]] Life m_lifetimeManager;

		void operator()(T* data) noexcept {
			for (size_t i = 0; i < m_size; ++i)
				m_lifetimeManager.destroy(data + i);
			m_size = 0;
			m_capacity = 0;
			m_allocator.deallocate(data, m_capacity);
		};
	};

	template <typename T, TypedAllocatorConcept<T> Alloc = TypedAllocator<T>,
		LifetimeManagerConcept<T> Life = LifetimeManager<T>,
		size_t s_initialCapacity = 16, float s_growthFactor = 1.618f>
	class Vector
	{
		static_assert(!std::is_reference_v<T>, "Vector does not support references");
		static_assert(std::is_destructible_v<T>, "Vector requires destructible types");

		static_assert(std::is_default_constructible_v<Alloc>, "Allocator must be default constructible");
		static_assert(std::is_destructible_v<Alloc>, "Allocator must be destructible");

		static_assert(std::is_default_constructible_v<Life>, "LifetimeManager must be default constructible");
		static_assert(std::is_destructible_v<Life>, "LifetimeManager must be destructible");

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
		[[no_unique_address]] Alloc m_allocator;
		[[no_unique_address]] Life m_lifetimeManager;

	public:
		Vector() : m_data(m_allocator.allocate(s_initialCapacity)), m_capacity(s_initialCapacity) {};

		Vector(size_t size) requires(std::is_default_constructible_v<T>)
			: m_data(m_allocator.allocate(size)), m_capacity(size), m_size(size) {			
			for (size_t i = 0; i < size; ++i)
				m_lifetimeManager.construct(m_data + i);
		};
		Vector(size_t size, const T& value) requires (std::is_copy_constructible_v<T>)
			: m_data(m_allocator.allocate(size)), m_capacity(size), m_size(size) {
			for (size_t i = 0; i < size; ++i)
				m_lifetimeManager.construct(m_data + i, value);
		};

		~Vector()
		{
			clear();
			m_allocator.deallocate(m_data, m_capacity);
		}

		Vector(InitializerList<T> list) requires(std::is_copy_constructible_v<T>)
			: m_capacity(list.size()), m_size(0),
			m_data(m_allocator.allocate(list.size())) {
			for (auto& elem : list)
				m_lifetimeManager.construct(m_data + m_size++, elem);
		};

		Vector& operator=(InitializerList<T> list) requires(std::is_copy_constructible_v<T>)
		{
			clear();
			m_allocator.deallocate(m_data, m_capacity);
			m_capacity = list.size();
			m_size = 0;
			m_data = m_allocator.allocate(m_capacity);
			for (auto& elem : list)
				m_lifetimeManager.construct(m_data + m_size++, elem);
			return *this;
		};

		Vector(const std::vector<T>& vector) requires(std::is_copy_constructible_v<T>)
			: m_capacity(vector.capacity()), m_size(0),
			m_data(m_allocator.allocate(vector.capacity()))
		{
			for (auto& elem : vector)
				m_lifetimeManager.construct(m_data + m_size++, elem);
		}

		Vector& operator=(const std::vector<T>& vector) requires(std::is_copy_constructible_v<T>)
		{
			clear();
			m_allocator.deallocate(m_data, m_capacity);
			m_capacity = vector.capacity();
			m_size = 0;
			m_data = m_allocator.allocate(m_capacity);
			for (auto& elem : vector)
				m_lifetimeManager.construct(m_data + m_size++, elem);
			return *this;
		}

		//cannot move std::vector since it can't release resources
		Vector(std::vector<T>&&) = delete;		
		Vector& operator=(std::vector<T>&&) = delete;

		Vector(const Vector& other) requires std::is_copy_constructible_v<T>
			: m_data(m_allocator.allocate(other.m_capacity)),
			m_capacity(other.m_capacity), m_size(other.m_size) {
			for (size_t i = 0; i < m_size; ++i)
				m_lifetimeManager.construct(m_data + i, other.m_data[i]);
		};

		Vector(Vector&& other) requires std::is_move_constructible_v<Alloc> &&
			std::is_move_constructible_v<Life> : m_data(other.m_data),
			m_capacity(other.m_capacity), m_size(other.m_size), m_allocator(std::move(other.m_allocator)),
			m_lifetimeManager(std::move(other.m_lifetimeManager)) {
			other.m_size = 0;
			other.m_allocator = Alloc();
			other.m_lifetimeManager = Life();
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
				m_lifetimeManager.construct(m_data + i, other.m_data[i]);
			m_size = other.m_size;
			return *this;
		};

		Vector& operator=(Vector&& other) requires (std::is_move_assignable_v<Alloc>) &&
			std::is_move_constructible_v<Life>
		{
			if (this == &other)
				return *this;
			clear();
			m_allocator.deallocate(m_data, m_capacity);

			m_capacity = other.m_capacity;
			m_size = other.m_size;
			m_data = std::move(other.m_data);
			m_allocator = std::move(other.m_allocator);
			m_lifetimeManager = std::move(other.m_lifetimeManager);

			other.m_size = 0;
			other.m_allocator = Alloc();
			other.m_lifetimeManager = Life();
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

		Memory::UniquePtr<T[], ReleaseDeleter<T, Alloc, Life>> release() {
			Memory::UniquePtr<T[], ReleaseDeleter<T, Alloc, Life>> data;
			ReleaseDeleter<T, Alloc, Life> deleter;
			deleter.m_allocator = std::exchange(m_allocator, Alloc());
			deleter.m_lifetimeManager = std::exchange(m_lifetimeManager, Life());
			deleter.m_size = std::exchange(m_size, 0);
			deleter.m_capacity = std::exchange(m_capacity, s_initialCapacity);
			data = Memory::UniquePtr<T[], ReleaseDeleter<T, Alloc, Life>>(
				std::exchange(m_data, m_allocator.allocate(s_initialCapacity)), std::move(deleter));
			return data;
		}

		void assign(T* data, size_t size) requires std::is_same_v<TypedAllocator<T>, Alloc>
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
				m_lifetimeManager.destroy(oldData + i);
			}
			m_allocator.deallocate(oldData, m_capacity);
			m_capacity = capacity;
		};

		void resize(size_t size, T arg = T()) requires
			((std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>)
				&& std::is_default_constructible_v<T>)
		{
			if (size == m_size)
				return;
			if (size > m_capacity)
			{
				migrate(s_growthFactor * (size + m_capacity - 1));
				for (size_t i = m_size; i < size; i++)
					m_lifetimeManager.construct(m_data + i, arg);
			}
			else if (size > m_size)
			{
				for (size_t i = m_size; i < size; i++)
					m_lifetimeManager.construct(m_data + i, arg);
			}
			else
			{
				for (size_t i = size; i < m_size; i++)
					m_lifetimeManager.destroy(m_data + i);
			}
			m_size = size;
		};

		void clear() {
			if (m_size == 0)
				return;
			for (size_t i = 0; i < m_size; ++i)
				m_lifetimeManager.destroy(m_data + i);
			m_size = 0;
		};

		void pushBack(const T& data) requires (std::is_copy_constructible_v<T>)
		{
			if (m_size >= m_capacity)
				grow();
			m_lifetimeManager.construct(m_data + m_size, data);
			++m_size;
		}

		void pushBack(T&& data) requires (std::is_move_constructible_v<T>)
		{
			if (m_size >= m_capacity)
				grow();
			m_lifetimeManager.construct(m_data + m_size, std::move(data));
			++m_size;
		}

		void popBack()
		{
			CONTAINERS_VERIFY(m_size > 0, "Popping an empty Vector");
			m_lifetimeManager.destroy(m_data + --m_size);
		}

		template<typename... Args>
		void emplaceBack(Args&&... data) requires std::constructible_from<T, Args...>
		{
			if (m_size >= m_capacity)
				grow();
			m_lifetimeManager.construct(m_data + m_size, std::forward<Args>(data)...);
			++m_size;
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
				m_lifetimeManager.construct(m_data, std::forward<Args>(data)...);
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
					m_lifetimeManager.destroy(m_data + i);
				}
				m_lifetimeManager.construct(newData + offset, std::forward<Args>(data)...);
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + i + 1, m_data[i]);
					m_lifetimeManager.destroy(m_data + i);
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

			m_lifetimeManager.destroy(it);
			m_lifetimeManager.construct(it, std::forward<Args>(data)...);
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
				m_lifetimeManager.construct(m_data, std::forward<U>(value));
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
					m_lifetimeManager.destroy(m_data + i);
				}
				m_lifetimeManager.construct(newData + offset, std::forward<U>(value));
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + i + 1, m_data[i]);
					m_lifetimeManager.destroy(m_data + i);
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
					m_lifetimeManager.destroy(m_data + i);
				}
				for (size_t i = 0; i < count; ++i)
					m_lifetimeManager.construct(newData + offset + i, std::forward<U>(value));
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + count + i, m_data[i]);
					m_lifetimeManager.destroy(m_data + i);
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
					m_lifetimeManager.construct(m_data, std::forward<U>(value));
				m_size = count;
				return pos;
			}

			for (it = end(); it < end() + count; ++it)
				m_lifetimeManager.construct(&(*it));

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
					m_lifetimeManager.destroy(m_data + i);
				}
				for (size_t i = 0; i < count; ++i, ++first)
					m_lifetimeManager.construct(newData + offset + i, *first);
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + count + i, m_data[i]);
					m_lifetimeManager.destroy(m_data + i);
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
					m_lifetimeManager.construct(m_data, *first);
				m_size = count;
				return pos;
			}

			for (it = end(); it < end() + count; ++it)
				m_lifetimeManager.construct(&(*it));

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
					m_lifetimeManager.destroy(m_data + i);
				}
				auto listIt = list.begin();
				for (size_t i = 0; i < count; ++i, ++listIt)
					m_lifetimeManager.construct(newData + offset + i, *listIt);
				for (size_t i = offset; i < m_size; ++i)
				{
					construct(newData + count + i, m_data[i]);
					m_lifetimeManager.destroy(m_data + i);
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
					m_lifetimeManager.construct(m_data, *listIt);
				m_size = count;
				return pos;
			}

			for (it = end(); it < end() + count; ++it)
				m_lifetimeManager.construct(&(*it));

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
			m_lifetimeManager.destroy(m_data + m_size);
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
				m_lifetimeManager.destroy(it + span);
			}

			for (; it < last; ++it)
				m_lifetimeManager.destroy(it);

			m_size -= span;
			return first;
		}

		void shrinkToFit() {
			if (m_size == m_capacity)
				return;
			migrate(m_size);
		}

		void swap(Vector& other) noexcept {
			std::swap(m_data, other.m_data);
			std::swap(m_size, other.m_size);
			std::swap(m_capacity, other.m_capacity);
			std::swap(m_allocator, other.m_allocator);
			std::swap(m_lifetimeManager, other.m_lifetimeManager);
		}

		bool operator==(const Vector& other) const {
			if (m_size != other.m_size)
				return false;
			for (size_t i = 0; i < m_size; i++)
				if (m_data[i] != other.m_data[i])
					return false;
			return true;
		}

		void grow() requires
			(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>) {
			auto newCapacity = s_growthFactor * (m_capacity + 1);
			migrate(newCapacity);
		};

		void shrink() {
			if (m_size == m_capacity)
				return;
			auto newCapacity = m_capacity / s_growthFactor;
			if (m_size > newCapacity)
				newCapacity = m_size;

			migrate(newCapacity);
		}

		Span<T> subSpan(size_t offset, size_t len) {
			return Span<T>(m_data + offset, len);
		}

		Span<const T> subSpan(size_t offset, size_t len) const {
			return Span<const T>(m_data + offset, len);
		}

	protected:

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

		inline void migrate(size_t newCapacity)
		{
			auto oldData = m_data;
			m_data = m_allocator.allocate(newCapacity);

			for (size_t i = 0; i < m_size; i++)
			{
				construct(m_data + i, oldData[i]);
				m_lifetimeManager.destroy(oldData + i);
			}
			m_allocator.deallocate(oldData, m_capacity);
			m_capacity = newCapacity;
		}
	};
}