#pragma once
#include "Trees/Trees.h"
#include "MapPair.h"

#include <stdexcept>
#include <cassert>
#include <concepts>

namespace Containers {
	template <typename Key, typename Val, typename Comparator>
	class MapPairOrdered : public MapPair<Key, Val>
	{
	public:
		using MapPairImp = MapPair<Key, Val>;
		using MapPairImp::MapPair;
		using MapPairImp::operator=;
		using MapPairImp::getKey;
		using MapPairImp::getValue;

		struct PairComparator
		{
			bool operator()(const MapPairImp& lhs, const MapPairImp& rhs) const {
				return Comparator()(lhs.key, rhs.key);
			}
		};
	};

	template <typename Key, typename Val, typename Comparator = std::less<Key>,
		TreeType<MapPairOrdered<Key, Val, Comparator>> TreeImpl =
		RedBlackTree<MapPairOrdered<Key, Val, Comparator>,
		typename MapPairOrdered<Key, Val, Comparator>::PairComparator>>
		class Map
	{
	public:
		class Iterator
		{
		public:
			// Iterator tags
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = MapPair<Key, Val>;
			using size_type = size_t;
			using difference_type = std::ptrdiff_t;
			using pointer = value_type*;
			using reference = value_type&;
			using const_reference = const reference;
			using const_pointer = const pointer;

		private:
			typename TreeImpl::Node* m_node;

		public:
			Iterator(typename TreeImpl::Node* node = nullptr) :
				m_node(node) {
			};
			~Iterator() = default;

			Iterator(const Iterator&) = default;
			Iterator& operator=(const Iterator&) = default;

			Iterator(Iterator&&) = default;
			Iterator& operator=(Iterator&&) = default;

			reference operator*() {
				if (m_node == nullptr)
					throw std::runtime_error("dereferencing end list iterator");
				return m_node->value;
			};

			const_reference operator*() const {
				if (m_node == nullptr)
					throw std::runtime_error("dereferencing end list iterator");
				return m_node->value;
			};

			pointer operator->() {
				if (m_node == nullptr)
					throw std::runtime_error("dereferencing end list iterator");
				return &(m_node->value);
			}

			const_pointer operator->() const {
				if (m_node == nullptr)
					throw std::runtime_error("dereferencing end list iterator");
				return &(m_node->value);
			}

			// Increment operator (in-order traversal)
			Iterator& operator++() {
				m_node = TreeImpl::traverseRight(m_node);
				return *this;
			}

			// Post-increment
			Iterator operator++(int) {
				Iterator tmp = *this;
				m_node = TreeImpl::traverseRight(m_node);
				return tmp;
			}

			Iterator& operator--() {
				m_node = TreeImpl::traverseLeft(m_node);
				return *this;
			}

			Iterator operator--(int) {
				Iterator tmp = *this;
				m_node = TreeImpl::traverseLeft(m_node);
				return tmp;
			}

			bool operator==(const Iterator& other) const
			{
				return m_node == other.m_node;
			};

			bool operator!=(const Iterator& other) const
			{
				return m_node != other.m_node;
			};

			friend class Map<Key, Val, Comparator, TreeImpl>;
		};

		class ConstIterator
		{
		public:
			// Iterator tags
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = const MapPair<Key, Val>;
			using size_type = size_t;
			using difference_type = std::ptrdiff_t;
			using pointer = value_type*;
			using reference = value_type&;
			using const_reference = reference;
			using const_pointer = pointer;

		private:
			const typename TreeImpl::Node* m_node;

		public:
			ConstIterator(const typename TreeImpl::Node* node = nullptr) :
				m_node(node) {
			};
			~ConstIterator() = default;

			ConstIterator(const ConstIterator&) = default;
			ConstIterator& operator=(const ConstIterator&) = default;

			ConstIterator(ConstIterator&&) = default;
			ConstIterator& operator=(ConstIterator&&) = default;

			const_reference operator*() const {
				if (m_node == nullptr)
					throw std::runtime_error("dereferencing end list iterator");
				return m_node->value;
			};

			const_pointer operator->() const {
				if (m_node == nullptr)
					throw std::runtime_error("dereferencing end list iterator");
				return &(m_node->value);
			}

			// Increment operator (in-order traversal)
			ConstIterator& operator++() {
				m_node = TreeImpl::traverseRight(m_node);
				return *this;
			}

			// Post-increment
			ConstIterator operator++(int) {
				ConstIterator tmp = *this;
				m_node = TreeImpl::traverseRight(m_node);
				return tmp;
			}

			ConstIterator& operator--() {
				m_node = TreeImpl::traverseLeft(m_node);
				return *this;
			}

			ConstIterator operator--(int) {
				ConstIterator tmp = *this;
				m_node = TreeImpl::traverseLeft(m_node);
				return tmp;
			}

			bool operator==(const ConstIterator& other) const
			{
				return m_node == other.m_node;
			};

			bool operator!=(const ConstIterator& other) const
			{
				return m_node != other.m_node;
			};

			friend class Map<Key, Val, Comparator, TreeImpl>;
		};

		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = const MapPairOrdered<Key, Val, Comparator>;
		using size_type = size_t;
		using Pair = MapPairOrdered<Key, Val, Comparator>;

	private:
		TreeImpl m_tree;

	public:
		Map() = default;
		Map(const Map&) requires std::is_copy_constructible_v<Pair> ||
			std::is_copy_assignable_v<Pair> = default;
		Map& operator=(const Map&) requires std::is_copy_constructible_v<Pair> ||
			std::is_copy_assignable_v<Pair> = default;

		Map(Map&&) = default;
		Map& operator=(Map&&) = default;

		template <typename Container>
			requires std::ranges::range<Container>&&
		std::convertible_to<std::ranges::range_value_t<Container>, T>
			Map(Container&& container)
		{
			for (auto&& elem : std::forward<Container>(container))
				m_tree.insert(std::forward<decltype(elem)>(elem));
		}

		template <typename Container>
			requires std::ranges::range<Container>&&
		std::convertible_to<std::ranges::range_value_t<Container>, T>
			Map& operator=(Container&& container)
		{
			m_tree.clear();
			for (auto&& elem : std::forward<Container>(container))
				m_tree.insert(std::forward<decltype(elem)>(elem));
			return *this;
		}

		Map(std::initializer_list<Pair> init)
		{
			for (const auto& value : init) {
				m_tree.insert(value);
			}
		}

		Map& operator=(std::initializer_list<Pair> init)
		{
			m_tree.clear();
			for (const auto& value : init) {
				m_tree.insert(value);
			}
			return *this;
		}

		void print() const
		{
			m_tree.printTree();
		}

		template<typename K, typename V>
		iterator insert(K&& key, V&& val)
			requires (std::is_same_v<std::decay_t<K>, Key>&& std::is_same_v<std::decay_t<V>, Val>)
		{
			return iterator(m_tree.insert(Pair(std::forward<K>(key), std::forward<V>(val))));
		}

		iterator insert(const Key& key, const Val& val)
		{
			return iterator(m_tree.insert(Pair(key, val)));
		}

		iterator erase(const Key& key)
		{
			auto* node = m_tree.find(Pair(key));
			if (node == nullptr)
				return iterator(nullptr);
			iterator next = iterator(TreeImpl::traverseRight(node));
			m_tree.erase(node);
			return next;
		}

		iterator erase(const iterator& it)
		{
			iterator next = it;
			next++;
			m_tree.erase(it.m_node);
			return next;
		}

		iterator find(const Key& key)
		{
			return iterator(m_tree.find(Pair(key)));
		}

		const_iterator find(const Key& key) const
		{
			return const_iterator(m_tree.find(Pair(key)));
		}

		size_t size() const
		{
			return m_tree.size();
		}

		bool empty() const
		{
			return m_tree.size() == 0;
		}

		iterator begin() {
			return iterator(m_tree.getLeftmost());
		}

		iterator end() {
			return iterator(nullptr);
		}

		const_iterator begin() const {
			return const_iterator(m_tree.getLeftmost());
		}

		const_iterator end() const {
			return const_iterator(nullptr);
		}
	};

	template<typename Key, typename Val, typename Comparator = std::less<Key>>
	using RBMap = Map<Key, Val, Comparator, RedBlackTree<MapPairOrdered<Key, Val, Comparator>,
		typename MapPairOrdered<Key, Val, Comparator>::PairComparator>>;

	template<typename Key, typename Val, typename Comparator = std::less<Key>>
	using AVLMap = Map<Key, Val, Comparator, AVLTree<MapPairOrdered<Key, Val, Comparator>,
		typename MapPairOrdered<Key, Val, Comparator>::PairComparator>>;

	template<typename Key, typename Val, typename Comparator = std::less<Key>>
	using FastSearchMap = Map<Key, Val, Comparator, AVLTree<MapPairOrdered<Key, Val, Comparator>,
		typename MapPairOrdered<Key, Val, Comparator>::PairComparator>>;
}