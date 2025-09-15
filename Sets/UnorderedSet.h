#pragma once
#include "HashTables/HashTables.h"

#include <stdexcept>
#include <cassert>

namespace Containers {
	//T must have operators < and == defined
	template <typename T, typename Hasher = std::hash<T>,
		HashTableType HashTableImpl = HashTableChained<T, Hasher>>
		class UnorderedSet
	{
	public:
		class Iterator
		{
		public:
			// Iterator tags
			using iterator_category = std::forward_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = const T*;
			using reference = const T&;

		private:
			typename HashTableImpl::Node m_node;

		public:
			Iterator(typename HashTableImpl::Node node = typename HashTableImpl::Node()) :
				m_node(node) {
			};
			~Iterator() = default;

			Iterator(const Iterator& other) = default;
			Iterator& operator=(const Iterator& other) = default;

			Iterator(Iterator&& other) = default;
			Iterator& operator=(Iterator&& other) = default;

			reference operator*() const {
				if (!m_node.isValid())
					throw std::runtime_error("dereferencing end list iterator");
				return m_node.getKey();
			};

			pointer operator->() const {
				if (!m_node.isValid())
					throw std::runtime_error("dereferencing end list iterator");
				return &(m_node.getKey());
			}

			// Increment operator (in-order traversal)
			Iterator& operator++() {
				m_node = HashTableImpl::iterateNext(m_node);
				return *this;
			}

			// Post-increment
			Iterator operator++(int) {
				Iterator tmp = *this;
				m_node = HashTableImpl::iterateNext(m_node);
				return tmp;
			}

			bool operator==(const Iterator& other) const
			{
				return  m_node == other.m_node;
			};

			bool operator!=(const Iterator& other) const
			{
				return  m_node != other.m_node;
			};

			friend class UnorderedSet<T, Hasher, HashTableImpl>;
		};

		using iterator = Iterator;          // Which is actually a const_iterator
		using const_iterator = Iterator;    // Same type as iterator
		using value_type = T;
		using size_type = size_t;

	private:
		HashTableImpl m_table;

	public:
		UnorderedSet() = default;
		~UnorderedSet() = default;
		UnorderedSet(const UnorderedSet&) requires std::is_copy_constructible_v<T> ||
			std::is_copy_assignable_v<T> = default;
		UnorderedSet& operator=(const UnorderedSet&) requires std::is_copy_constructible_v<T> ||
			std::is_copy_assignable_v<T> = default;

		UnorderedSet(UnorderedSet&&) = default;
		UnorderedSet& operator=(UnorderedSet&&) = default;

		template <typename Container>
			requires std::ranges::range<Container>&&
		std::convertible_to<std::ranges::range_value_t<Container>, T>
			UnorderedSet(Container&& container)
		{
			for (auto&& elem : std::forward<Container>(container))
				m_table.insert(std::forward<decltype(elem)>(elem));
		}

		template <typename Container>
			requires std::ranges::range<Container>&&
		std::convertible_to<std::ranges::range_value_t<Container>, T>
			UnorderedSet& operator=(Container&& container)
		{
			m_table.clear();
			for (auto&& elem : std::forward<Container>(container))
				m_table.insert(std::forward<decltype(elem)>(elem));
			return *this;
		}

		UnorderedSet(std::initializer_list<T> init)
		{
			for (const auto& value : init) {
				m_table.insert(value);
			}
		}

		UnorderedSet& operator=(std::initializer_list<T> init)
		{
			m_table.clear();
			for (const auto& value : init) {
				m_table.insert(value);
			}
			return *this;
		}

		template<typename U>
		iterator insert(U&& data)
		{
			return iterator(m_table.insert(std::forward<U>(data)));
		}

		iterator erase(const T& data)
		{
			auto node = m_table.find(data);
			if (!node.isValid())
				return iterator(m_table.end());
			auto toErase = iterator(node);
			auto next = toErase;
			next++;
			m_table.erase(toErase.m_node);
			return next;
		}

		iterator erase(const iterator& it)
		{
			iterator itNext = it;
			itNext++;
			m_table.erase(it.m_node);
			return itNext;
		}

		iterator find(const T& data) const
		{
			return iterator(m_table.find(data));
		}

		void reserve(size_t newCapacity)
		{
			m_table.reserve(newCapacity);
		}

		size_t size() const
		{
			return m_table.size();
		}

		size_t capacity() const
		{
			return m_table.capacity();
		}

		bool empty() const
		{
			return m_table.empty();
		}

		iterator begin() const {
			return iterator(m_table.begin());
		}

		iterator end() const {
			return iterator(m_table.end());
		}

		const_iterator cbegin() const { return begin(); }
		const_iterator cend() const { return end(); }
	};

	template <typename T, typename Hasher = std::hash<T>>
	using OpenAdressSet = UnorderedSet<T, Hasher, HashTableOpenAddress<T, Hasher>>;
}