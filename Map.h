#pragma once
#include "Trees/RedBlackTree.h"
#include "Trees/Trees.h"

#include <stdexcept>
#include <cassert>
#include <concepts>

template <typename Key, typename Val>
class MapPair
{
private:
	Key key;
	Val val;

public:

	bool operator<(const MapPair& other) const
	{
		return key < other.key;
	}

	bool operator==(const MapPair& other) const
	{
		return key == other.key;
	}

	MapPair() = default;

	MapPair(const Key& k)
		requires std::is_copy_constructible_v<Key>
	: key(k) {
	}

	MapPair(Key&& k)
		requires std::is_move_constructible_v<Key>
	: key(std::move(k)) {
	}

	template<typename K, typename V>
	MapPair(K&& k, V&& v)
		requires (std::is_convertible_v<K, Key>&&
	std::is_convertible_v<V, Val>&&
		std::is_constructible_v<Key, K&&>&&
		std::is_constructible_v<Val, V&&>)
		: key(std::forward<K>(k))
		, val(std::forward<V>(v))
	{
	}

	MapPair(const MapPair&) requires std::is_copy_constructible_v<Key>&& std::is_copy_constructible_v<Val> = default;
	MapPair& operator=(const MapPair&) requires std::is_copy_assignable_v<Key>&& std::is_copy_assignable_v<Val> = default;

	MapPair(MapPair&&) requires std::is_move_constructible_v<Key>&& std::is_move_constructible_v<Val> = default;
	MapPair& operator=(MapPair&&) requires std::is_move_assignable_v<Key>&& std::is_move_assignable_v<Val> = default;

	const Key& getKey() const noexcept { return key; }
	const Val& getValue() const noexcept { return val; }
	Val& getValue() noexcept { return val; }
};

template<typename K, typename V>
MapPair(K, V) -> MapPair<std::decay_t<K>, std::decay_t<V>>;

//T must have operators < and == defined
template <typename Key, typename Val, 
	TreeType<MapPair<Key, Val>> TreeImpl = RedBlackTree<MapPair<Key, Val>>>
class Map
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
	class Iterator
	{
	public:
		// Iterator tags
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = MapPair<Key, Val>;
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

		Iterator(const Iterator& other) = default;
		Iterator& operator=(const Iterator& other) = default;

		Iterator(Iterator&& other) = default;
		Iterator& operator=(Iterator&& other) = default;

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

		friend class Map<Key, Val, TreeImpl>;
	};

	class ConstIterator
	{
	public:
		// Iterator tags
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = const MapPair<Key, Val>;
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

		ConstIterator(const ConstIterator& other) = default;
		ConstIterator& operator=(const ConstIterator& other) = default;

		ConstIterator(ConstIterator&& other) = default;
		ConstIterator& operator=(ConstIterator&& other) = default;

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

		friend class Map<Key, Val, TreeImpl>;
	};

	using iterator = Iterator;          
	using const_iterator = ConstIterator;
	using Pair = MapPair<Key, Val>;

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

	//template <typename Container>
	//	requires std::ranges::range<Container>&&
	//std::convertible_to<std::ranges::range_value_t<Container>, T>
	//	Map(Container&& container)
	//{
	//	for (auto&& elem : std::forward<Container>(container))
	//		m_tree.insert(std::forward<decltype(elem)>(elem));
	//}

	//template <typename Container>
	//	requires std::ranges::range<Container>&&
	//std::convertible_to<std::ranges::range_value_t<Container>, T>
	//	Map& operator=(Container&& container)
	//{
	//	m_tree.clear();
	//	for (auto&& elem : std::forward<Container>(container))
	//		m_tree.insert(std::forward<decltype(elem)>(elem));
	//	return *this;
	//}

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
		requires (std::is_same_v<std::decay_t<K>, Key> && std::is_same_v<std::decay_t<V>, Val>)
	{
		return iterator(m_tree.insert(MapPair<Key, Val>(std::forward<K>(key), std::forward<V>(val))));
	}

	iterator insert(const Key& key, const Val& val)
	{
		return iterator(m_tree.insert(MapPair<Key, Val>(key, val)));
	}

	void erase(const Key& key)
	{
		m_tree.erase(MapPair<Key, Val>(key));
	}

	void erase(const_iterator& it)
	{
		m_tree.erase(it->getKey());
	}

	iterator find(const Key& key)
	{
		return iterator(m_tree.find(MapPair<Key, Val>(key)));
	}

	const_iterator find(const Key& key) const
	{
		return const_iterator(m_tree.find(MapPair<Key, Val>(key)));
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

template<typename Key, typename Val>
using RBMap = Map<Key, Val, RedBlackTree<MapPair<Key, Val>>>;

template<typename Key, typename Val>
using AVLMap = Map<Key, Val, AVLTree<MapPair<Key, Val>>>;

template<typename Key, typename Val>
using FastSearchMap = Map<Key, Val, AVLTree<MapPair<Key, Val>>>;