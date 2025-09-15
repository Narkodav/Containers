#pragma once
#include "HashTables/HashTables.h"
#include "MapPair.h"

#include <stdexcept>
#include <cassert>
#include <concepts>

namespace Containers {
	template <typename Key, typename Val, typename Hasher>
	class MapPairUnordered : public MapPair<Key, Val>
	{
	public:
		using MapPairImp = MapPair<Key, Val>;
		using MapPairImp::MapPair;
		using MapPairImp::operator=;
		using MapPairImp::getKey;
		using MapPairImp::getValue;

		struct PairHasher
		{
			size_t operator()(const MapPairImp& pair) const {
				return Hasher()(pair.getKey());
			}
		};

		bool operator==(const MapPairUnordered<Key, Val, Hasher>& other) const {
			return getKey() == other.getKey();
		}
	};

	template <typename Key, typename Val, typename Hasher = std::hash<Key>,
		HashTableType HashTableImpl = HashTableChained<MapPairUnordered<Key, Val, Hasher>,
		typename MapPairUnordered<Key, Val, Hasher>::PairHasher>>
		class UnorderedMap
	{
	private:
		template<typename S, typename T>
		class is_streamable {
			template<typename SS, typename TT>
			static auto test(int)
				-> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());

			template<typename, typename>
			static auto test(...) -> std::false_type;

		public:
			static const bool value = decltype(test<S, T>(0))::value;
		};

	public:
		using Pair = MapPairUnordered<Key, Val, Hasher>;

		class Iterator
		{
		public:
			// Iterator tags
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = MapPairUnordered<Key, Val, Hasher>;
			using size_type = size_t;
			using difference_type = std::ptrdiff_t;
			using pointer = value_type*;
			using reference = value_type&;
			using const_reference = const reference;
			using const_pointer = const pointer;

		private:
			typename HashTableImpl::Node m_node;

		public:
			Iterator(typename HashTableImpl::Node node = typename HashTableImpl::Node()) :
				m_node(node) {
			};
			~Iterator() = default;

			Iterator(const Iterator&) = default;
			Iterator& operator=(const Iterator&) = default;

			Iterator(Iterator&&) = default;
			Iterator& operator=(Iterator&&) = default;

			reference operator*() {
				return m_node.getKey();
			};

			const_reference operator*() const {
				return m_node.getKey();
			};

			pointer operator->() {
				return &(m_node.getKey());
			}

			const_pointer operator->() const {
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

			friend class UnorderedMap<Key, Val, Hasher, HashTableImpl>;
		};

		class ConstIterator
		{
		public:
			// Iterator tags
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = const MapPairUnordered<Key, Val, Hasher>;
			using size_type = size_t;
			using difference_type = std::ptrdiff_t;
			using pointer = value_type*;
			using reference = value_type&;
			using const_reference = reference;
			using const_pointer = pointer;

		private:
			const typename HashTableImpl::Node m_node;

		public:
			ConstIterator(typename HashTableImpl::Node node = typename HashTableImpl::Node()) :
				m_node(node) {
			};
			~ConstIterator() = default;

			ConstIterator(const ConstIterator&) = default;
			ConstIterator& operator=(const ConstIterator&) = default;

			ConstIterator(ConstIterator&&) = default;
			ConstIterator& operator=(ConstIterator&&) = default;

			const_reference operator*() const {
				return m_node.getKey().getValue();
			};

			const_pointer operator->() const {
				return &(m_node.getKey().getValue());
			}

			// Increment operator (in-order traversal)
			ConstIterator& operator++() {
				m_node = HashTableImpl::iterateNext(m_node);
				return *this;
			}

			// Post-increment
			ConstIterator operator++(int) {
				ConstIterator tmp = *this;
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

			friend class UnorderedMap<Key, Val, Hasher, HashTableImpl>;
		};

		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = MapPairUnordered<Key, Val, Hasher>;
		using size_type = size_t;

	private:
		HashTableImpl m_table;

	public:
		UnorderedMap() = default;

		UnorderedMap(const UnorderedMap&) requires std::is_copy_constructible_v<Pair> ||
			std::is_copy_assignable_v<Pair> = default;
		UnorderedMap& operator=(const UnorderedMap&) requires std::is_copy_constructible_v<Pair> ||
			std::is_copy_assignable_v<Pair> = default;

		UnorderedMap(UnorderedMap&&) = default;
		UnorderedMap& operator=(UnorderedMap&&) = default;

		template <typename Container>
			requires std::ranges::range<Container>&&
		std::convertible_to<std::ranges::range_value_t<Container>, T>
			UnorderedMap(Container&& container)
		{
			for (auto&& elem : std::forward<Container>(container))
				m_table.insert(std::forward<decltype(elem)>(elem));
		}

		template <typename Container>
			requires std::ranges::range<Container>&&
		std::convertible_to<std::ranges::range_value_t<Container>, T>
			UnorderedMap& operator=(Container&& container)
		{
			m_table.clear();
			for (auto&& elem : std::forward<Container>(container))
				m_table.insert(std::forward<decltype(elem)>(elem));
			return *this;
		}

		UnorderedMap(std::initializer_list<Pair> init)
		{
			for (const auto& value : init) {
				m_table.insert(value);
			}
		}

		UnorderedMap& operator=(std::initializer_list<Pair> init)
		{
			m_table.clear();
			for (const auto& value : init) {
				m_table.insert(value);
			}
			return *this;
		}

		template<typename K, typename V>
		Iterator insert(K&& key, V&& val)
			requires (std::is_same_v<std::decay_t<K>, Key>&& std::is_same_v<std::decay_t<V>, Val>)
		{
			return Iterator(m_table.insert(Pair(std::forward<K>(key), std::forward<V>(val))));
		}

		Iterator insert(const Key& key, const Val& val)
		{
			return Iterator(m_table.insert(Pair(key, val)));
		}

		Iterator erase(const Key& key)
		{
			auto node = m_table.find(Pair(key));
			if (node.index >= node.tableCapacity)
				return Iterator(m_table.end());
			auto toErase = Iterator(node);
			auto next = toErase;
			next++;
			m_table.erase(toErase.m_node);
			return next;
		}

		Iterator erase(const Iterator& it)
		{
			Iterator itNext = it;
			itNext++;
			m_table.erase(it.m_node);
			return itNext;
		}

		Iterator find(const Key& key)
		{
			return Iterator(m_table.find(Pair(key)));
		}

		const_iterator find(const Key& key) const
		{
			return const_iterator(m_table.find(Pair(key)));
		}

		size_t size() const
		{
			return m_table.size();
		}

		bool empty() const
		{
			return m_table.size() == 0;
		}

		Iterator begin() {
			return Iterator(m_table.begin());
		}

		Iterator end() {
			return Iterator(m_table.end());
		}

		ConstIterator begin() const {
			return ConstIterator(m_table.begin());
		}

		ConstIterator end() const {
			return ConstIterator(m_table.end());
		}

		ConstIterator cbegin() const {
			return ConstIterator(m_table.begin());
		}

		ConstIterator cend() const {
			return ConstIterator(m_table.end());
		}
	};
}