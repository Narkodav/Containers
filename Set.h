#pragma once
#include "RedBlackTree.h"

#include <stdexcept>
#include <cassert>
#include <concepts>

//T must have operators < and == defined
template <typename T, TreeType<T> TreeImpl = RedBlackTree<T>>
class Set
{
public:
	class Iterator
	{
	private:
		const typename TreeImpl::Node* m_node;

	public:
		Iterator(const typename TreeImpl::Node* node) : m_node(node) {};
		~Iterator() {};
		Iterator(const Iterator& other) : m_node(other.m_node) {};
		Iterator& operator=(const Iterator& other)
		{
			m_node = other.m_node;
			return *this;
		};

		const T& operator*() const {
			if (m_node == nullptr)
				throw std::runtime_error("dereferencing nullptr iterator");
			return m_node->value;
		};

		const T* operator->() const {
			if (m_node == nullptr)
				throw std::runtime_error("dereferencing nullptr iterator");
			return &(m_node->value);
		}

		bool operator==(const Iterator& other) const
		{
			return m_node == other.m_node;
		};

		bool operator!=(const Iterator& other) const
		{
			return m_node != other.m_node;
		};

		bool operator==(std::nullptr_t) const
		{
			return m_node == nullptr;
		};

		bool operator!=(std::nullptr_t) const
		{
			return m_node != nullptr;
		};

		friend class Set<T, TreeImpl>;
	};

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

	void print() const
	{
		m_tree.printTree();
	}

	void insert(const T& data)
	{
		m_tree.insert(data);
	}

	void erase(const T& data)
	{
		m_tree.erase(data);
	}

	void erase(const Iterator& it)
	{
		m_tree.erase(it.m_node);
		it.m_node = nullptr;
	}

	Iterator find(const T& data) const
	{
		return Iterator(m_tree.find(data));
	}

	size_t size() const
	{
		return m_tree.size();
	}

	bool empty() const
	{
		return m_tree.size() == 0;
	}
};

