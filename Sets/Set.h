#pragma once
#include "Trees/Trees.h"

#include <stdexcept>
#include <cassert>

template <typename T, typename Comparator = std::less<T>,
	TreeType<T> TreeImpl = RedBlackTree<T, Comparator>>
	class Set
{
public:
	class Iterator
	{
	public:
		// Iterator tags
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = const T*;
		using reference = const T&;

	private:
		const typename TreeImpl::Node* m_node;

	public:
		Iterator(const typename TreeImpl::Node* node = nullptr) :
			m_node(node) {
		};
		~Iterator() = default;

		Iterator(const Iterator& other) = default;
		Iterator& operator=(const Iterator& other) = default;

		Iterator(Iterator&& other) = default;
		Iterator& operator=(Iterator&& other) = default;

		reference operator*() const {
			if (m_node == nullptr)
				throw std::runtime_error("dereferencing end list iterator");
			return m_node->value;
		};

		pointer operator->() const {
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

		friend class Set<T, Comparator, TreeImpl>;
	};

	using iterator = Iterator;          // Which is actually a const_iterator
	using const_iterator = Iterator;    // Same type as iterator
	using value_type = T;				// Must have value_type
	using size_type = size_t;			// Must have size_type

private:
	TreeImpl m_tree;

public:
	Set() = default;
	Set(const Set&) requires std::is_copy_constructible_v<T> ||
		std::is_copy_assignable_v<T> = default;
	Set& operator=(const Set&) requires std::is_copy_constructible_v<T> ||
		std::is_copy_assignable_v<T> = default;

	Set(Set&&) = default;
	Set& operator=(Set&&) = default;

	template <typename Container>
		requires std::ranges::range<Container>&&
	std::convertible_to<std::ranges::range_value_t<Container>, T>
		Set(Container&& container)
	{
		for (auto&& elem : std::forward<Container>(container))
			m_tree.insert(std::forward<decltype(elem)>(elem));
	}

	template <typename Container>
		requires std::ranges::range<Container>&&
	std::convertible_to<std::ranges::range_value_t<Container>, T>
		Set& operator=(Container&& container)
	{
		m_tree.clear();
		for (auto&& elem : std::forward<Container>(container))
			m_tree.insert(std::forward<decltype(elem)>(elem));
		return *this;
	}

	Set(std::initializer_list<T> init)
	{
		for (const auto& value : init) {
			m_tree.insert(value);
		}
	}

	Set& operator=(std::initializer_list<T> init)
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

	template<typename U>
	iterator insert(U&& data)
	{
		return iterator(m_tree.insert(std::forward<U>(data)));
	}

	iterator erase(const T& data)
	{
		auto* node = m_tree.find(data);
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

	iterator find(const T& data) const
	{
		return iterator(m_tree.find(data));
	}

	size_t size() const
	{
		return m_tree.size();
	}

	bool empty() const
	{
		return m_tree.size() == 0;
	}

	iterator begin() const {
		return iterator(m_tree.getLeftmost());
	}

	iterator end() const {
		return iterator(nullptr);
	}

	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }
};

// Choose RBSet when:
// - Insertions and deletions are frequent
// - Search time can be slightly slower
// - You need fewer rebalancing operations
template<typename T, typename Comparator = std::less<T>>
using RBSet = Set<T, Comparator, RedBlackTree<T, Comparator>>;

template<typename T, typename Comparator = std::less<T>>
using AVLSet = Set<T, Comparator, AVLTree<T, Comparator>>;

// Choose FastSearchSet when:
// - Lookups are more frequent than modifications
// - Maintaining minimal tree depth is important
// - You need consistently fast search times
template<typename T, typename Comparator = std::less<T>>
using FastSearchSet = Set<T, Comparator, AVLTree<T, Comparator>>;