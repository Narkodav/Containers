#pragma once
#include "Trees/RedBlackTree.h"
#include "Trees/TreeConcept.h"

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
		requires (std::is_same_v<std::decay_t<K>, Key>&&
	std::is_same_v<std::decay_t<V>, Val> &&
		(std::is_constructible_v<Key, K&&>&&
			std::is_constructible_v<Val, V&&>))
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

//T must have operators < and == defined
template <typename Key, typename Val, TreeType<MapPair<Key, Val>> TreeImpl = RedBlackTree<MapPair<Key, Val>>>
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

		const Val& operator*() const {
			if (m_node == nullptr)
				throw std::runtime_error("dereferencing nullptr iterator");
			return m_node->value.getValue();
		};

		const Val* operator->() const {
			if (m_node == nullptr)
				throw std::runtime_error("dereferencing nullptr iterator");
			return &(m_node->value.getValue());
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

		const Key& getKey() const
		{
			if (m_node == nullptr)
				throw std::runtime_error("dereferencing nullptr iterator");
			return m_node->value.getKey();
		}

		friend class Map<Key, Val, TreeImpl>;
	};

private:
	TreeImpl m_tree;

public:
	Map() = default;
	Map(const Map&) requires std::is_copy_constructible_v<MapPair<Key, Val>> ||
		std::is_copy_assignable_v<MapPair<Key, Val>> = default;
	Map& operator=(const Map&) requires std::is_copy_constructible_v<MapPair<Key, Val>> ||
		std::is_copy_assignable_v<MapPair<Key, Val>> = default;

	Map(Map&&) = default;
	Map& operator=(Map&&) = default;

	void print() const
	{
		m_tree.printTree();
	}


	template<typename K, typename V>
	Iterator insert(K&& key, V&& val)
		requires (std::is_same_v<std::decay_t<K>, Key> && std::is_same_v<std::decay_t<V>, Val>)
	{
		return Iterator(m_tree.insert(MapPair<Key, Val>(std::forward<K>(key), std::forward<V>(val))));
	}

	Iterator insert(const Key& key, const Val& val)
	{
		return Iterator(m_tree.insert(MapPair<Key, Val>(key, val)));
	}

	void erase(const Key& key)
	{
		m_tree.erase(MapPair<Key, Val>(key));
	}

	void erase(const Iterator& it)
	{
		m_tree.erase(it);
	}

	Iterator find(const Key& key) const
	{
		return Iterator(m_tree.find(MapPair<Key, Val>(key)));
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

