#pragma once
#include "Lists/Lists.h"

#include <memory>
#include <stdexcept>

template<typename Key, typename Hasher = std::hash<Key>, ListType ListImpl = ListOneSided<std::pair<Key, size_t>>>
class HashTableChained
{
public:
	using ValueType = Key;
	using KeyHashPair = std::pair<Key, size_t>;

	struct Bucket
	{
		size_t firstHash = 0;
		Key firstKey;
		ListImpl collisions;
		bool isOccupied = false;
	};

	struct Node
	{
		size_t tableCapacity = 0;
		size_t index = 0;
		ListImpl::Node* currentNode = nullptr; //if nullptr the value is first stored
		Bucket* bucket = nullptr;

		Node(size_t tableCapacity = 0, size_t index = 0,
			ListImpl::Node* currentNode = nullptr, Bucket* bucket = nullptr) :
			tableCapacity(tableCapacity), index(index), currentNode(currentNode), bucket(bucket) {
		};
		~Node() = default;

		// Only enable copy operations if T is copyable
		Node(const Node&)= default;
		Node& operator=(const Node&)= default;

		// Only enable move operations if T is moveable
		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;

		Key& getKey()
		{
			return const_cast<Key>(static_cast<const Node*>(this)->getKey());
		}

		const Key& getKey() const
		{
			if (index >= tableCapacity)
				throw std::runtime_error("Dereferencing end list iterator");
			if (currentNode == nullptr)
				return bucket->firstKey;
			else return currentNode->m_data.first;
		}
	};

	static inline const float maxLoadFactor = 0.75f;
	static inline const size_t initialSize = 16;
	static inline const size_t growthFactor = 2;

private:

	Bucket* m_table = nullptr;
	size_t m_size = 0;
	size_t m_capacity = initialSize;
	float m_loadFactor = 0.f;

public:
	HashTableChained() : m_table(new Bucket[initialSize]), m_size(0),
		m_capacity(initialSize), m_loadFactor(0.f)
	{};

	~HashTableChained() { delete[] m_table; };

	HashTableChained(const HashTableChained& other)
		requires std::is_copy_constructible_v<Key> || std::is_copy_assignable_v<Key> :
	m_table(new Bucket[other.m_capacity]), m_size(other.m_size),
		m_capacity(other.m_capacity), m_loadFactor(other.m_loadFactor)
	{
		for (size_t i = 0; i < m_capacity; i++)
			if (other.m_table[i].isOccupied)
			{
				m_table[i].isOccupied = true;
				m_table[i].firstHash = other.m_table[i].firstHash;
				m_table[i].firstKey = other.m_table[i].firstKey;
				m_table[i].collisions = other.m_table[i].collisions;
			}
	}

	HashTableChained& operator=(const HashTableChained& other)
		requires std::is_copy_constructible_v<Key> || std::is_copy_assignable_v<Key>
	{
		if (this == &other)
			return *this;
			
		delete[] m_table;
		m_table = new Bucket[other.m_capacity];
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		m_loadFactor = other.m_loadFactor;

		for (size_t i = 0; i < m_capacity; i++)
			if (other.m_table[i].isOccupied)
			{
				m_table[i].isOccupied = true;
				m_table[i].firstHash = other.m_table[i].firstHash;
				m_table[i].firstKey = other.m_table[i].firstKey;
				m_table[i].collisions = other.m_table[i].collisions;
			}

		return *this;
	}

	HashTableChained(HashTableChained&&) = default;
	HashTableChained& operator=(HashTableChained&&) = default;

	void clear(size_t reservation = initialSize)
	{
		delete[] m_table;
		m_table = new Bucket[reservation];
		m_size = 0;
		m_capacity = reservation;
		m_loadFactor = 0.f;
	}

	//reserving less data than m_loadFactor allows will cause rehashes during subsequent inserts
	void reserve(size_t reservation)
	{
		rehash(reservation);
	}

	template<typename K>
		requires std::is_same_v<Key, std::decay_t<K>>
	Node insert(K&& key)
	{
		size_t hash = Hasher()(key);
		Node node = insertAtHash<K, true>(hash, std::forward<K>(key));
		if (m_loadFactor > maxLoadFactor)
			rehash<true>(m_capacity * growthFactor, node);
		return node;
	}	

	template<typename K>
		requires std::is_same_v<Key, std::decay_t<K>>
	bool contains(K&& key) const
	{
		size_t index = Hasher()(key) % m_capacity;
		Bucket& bucket = m_table[index];

		if (bucket.isOccupied)
		{
			if (bucket.firstKey == key)
				return true;
			else
			{
				typename ListImpl::Node* current = bucket.collisions.getFront();
				while (current != nullptr)
				{
					if (current->m_data.first == key)
						return true;
					current = ListImpl::iterateNext(current);
				}
			}
		}
		return false;
	}

	Node find(const Key& key) const
	{
		size_t index = Hasher()(key) % m_capacity;
		Bucket& bucket = m_table[index];

		if (bucket.isOccupied)
		{
			if (bucket.firstKey == key)
				return Node{ m_capacity, index, nullptr, &bucket };
			else
			{
				typename ListImpl::Node* current = bucket.collisions.getFront();
				while (current != nullptr)
				{
					if (current->m_data.first == key)
						return Node{ m_capacity, index, current, &bucket };
					current = ListImpl::iterateNext(current);
				}
			}
		}
		return Node{ m_capacity, m_capacity, nullptr, nullptr };
	}

	bool erase(const Key& key)
	{
		size_t index = Hasher(key) % m_capacity;
		Bucket& bucket = m_table[index];

		if (bucket.isOccupied)
		{
			if (bucket.firstKey == key)
			{
				if (bucket.collisions.getFront() == nullptr)
				{
					std::destroy_at(&bucket.firstKey);  // More modern approach
					bucket.isOccupied = false;
					bucket.hash = 0;					
				}
				else
				{
					auto* head = bucket.collisions.getFront();
					if constexpr (std::is_move_assignable_v<Key>)
					{
						bucket.firstKey = std::move(head->m_data.first);
					}
					else if constexpr (std::is_copy_assignable_v<Key>)
					{
						bucket.firstKey = head->m_data.first;
					}
					else static_assert(false, "Key type must be copy or move assignable");
					bucket.firstHash = head->m_data.second;
					bucket.collisions.deleteFront();					
				}
				m_size--;
				return true;
			}
			else
			{
				typename ListImpl::Node* current = bucket.collisions.getFront();
				while (current != nullptr)
				{
					if (current->m_data.first == key)
					{
						bucket.collisions.deleteNode(current);
						m_size--;
						return true;
					}
					current = ListImpl::iterateNext(current);
				}
			}
		}
		return false;
	}

	void erase(const Node& node)
	{
		Bucket& bucket = *node.bucket;
		if (node.currentNode != nullptr)
			bucket.collisions.deleteNode(node.currentNode);
		else if(bucket.collisions.getFront() != nullptr)
		{
			auto* head = bucket.collisions.getFront();
			if constexpr (std::is_move_assignable_v<Key>)
			{
				bucket.firstKey = std::move(head->m_data.first);
			}
			else if constexpr (std::is_copy_assignable_v<Key>)
			{
				bucket.firstKey = head->m_data.first;
			}
			else static_assert(false, "Key type must be copy or move assignable");
			bucket.firstHash = head->m_data.second;
			bucket.collisions.deleteFront();
		}
		else
		{
			std::destroy_at(&bucket.firstKey);
			bucket.firstHash = 0;
			bucket.isOccupied = false;
		}
		m_size--;
	}

	static Node iterateNext(const Node& node)
	{
		if (node.index >= node.tableCapacity)
			throw std::runtime_error("iterating with an invalid node");

		Node newNode;
		newNode.bucket = node.bucket;
		newNode.currentNode = node.currentNode;
		newNode.index = node.index;
		newNode.tableCapacity = node.tableCapacity;

		if (newNode.currentNode == nullptr) {
			// Currently at bucket's first element
			if (newNode.bucket->collisions.getFront() != nullptr) {
				// Move to first collision if it exists
				newNode.currentNode = newNode.bucket->collisions.getFront();
			}
			else {
				// Move to next bucket
				do {
					newNode.index++;
					newNode.bucket++;
				} while (newNode.index < newNode.tableCapacity && !newNode.bucket->isOccupied);
			}
		}
		else {
			// Currently in collision list
			newNode.currentNode = ListImpl::iterateNext(newNode.currentNode);
			if (newNode.currentNode == nullptr) {
				// Move to next bucket
				do {
					newNode.index++;
					newNode.bucket++;
				} while (newNode.index < newNode.tableCapacity && !newNode.bucket->isOccupied);
			}
		}
		return newNode;
	}

	size_t size() const { return m_size; };
	float loadFactor() const { return m_loadFactor; };
	size_t capacity() const { return m_capacity; };
	bool empty() const { return m_size == 0; };

	Node begin() const
	{		
		for (int i = 0; i < m_capacity; i++)
		{
			if (m_table[i].isOccupied)
				return Node(m_capacity, i, nullptr, m_table + i);
		}
		return Node(m_capacity, m_capacity, nullptr, nullptr);
	}

	Node end() const
	{
		return Node(m_capacity, m_capacity, nullptr, nullptr);		
	}

private:
	template<typename K, bool ShouldReturnNode = false>
		requires std::is_same_v<Key, std::decay_t<K>>
	std::conditional_t<ShouldReturnNode, Node, void> insertAtHash(size_t hash, K&& key)
	{
		size_t index = hash % m_capacity;
		Bucket& bucket = m_table[index];

		if (bucket.isOccupied)
		{
			if (bucket.firstKey == key)
			{
				bucket.firstKey = std::forward<K>(key);
				bucket.firstHash = hash;
			}
			else
			{
				typename ListImpl::Node* current = bucket.collisions.getFront();
				while (current != nullptr)
				{
					if (current->m_data.first == key)
					{
						current->m_data.first = std::forward<K>(key);
						current->m_data.second = hash;
						if constexpr (ShouldReturnNode)
							return Node(m_capacity, index, current, m_table + index);
						else return;
					}
					current = ListImpl::iterateNext(current);
				}
				bucket.collisions.insertFront(KeyHashPair(std::forward<K>(key), hash));
				m_size++;
				m_loadFactor = (float)m_size / m_capacity;
				if constexpr (ShouldReturnNode)
					return Node(m_capacity, index, bucket.collisions.getFront(), m_table + index);
				else return;
			}
		}
		else
		{
			bucket.firstHash = hash;
			bucket.firstKey = std::forward<K>(key);
			bucket.isOccupied = true;
			m_size++;
			m_loadFactor = (float)m_size / m_capacity;
		}
		if constexpr (ShouldReturnNode)
			return Node(m_capacity, index, nullptr, m_table + index);
		else return;
	};

	template<bool ShouldUpdateNode = false>
	void rehash(size_t newCapacity, std::conditional_t<ShouldUpdateNode, Node&, void> oldNode)
	{
		Bucket* oldTable = m_table;
		size_t oldCapacity = m_capacity;
		m_table = new Bucket[newCapacity];
		m_capacity = newCapacity;
		m_size = 0;

		if constexpr (ShouldUpdateNode) {
			Bucket& bucket = *oldNode.bucket;
			if (oldNode.currentNode == nullptr)
			{
				oldNode = insertAtHash<Key, true>(bucket.firstHash, std::move(bucket.firstKey));
				if (bucket.collisions.getFront() != nullptr)
				{
					bucket.firstHash = bucket.collisions.getFront()->m_data.second;
					bucket.firstKey = bucket.collisions.getFront()->m_data.first;
					bucket.collisions.deleteFront();
				}
				else bucket.isOccupied = false;
			}
			else
			{
				oldNode = insertAtHash<Key, true>
					(oldNode.currentNode->m_data.second, std::move(oldNode.currentNode->m_data.first));
				bucket.collisions.deleteNode(oldNode.currentNode);
			}
		}

		for (size_t i = 0; i < oldCapacity; i++)
		{
			Bucket& bucket = oldTable[i];
			if (bucket.isOccupied)
			{
				size_t newIndex = bucket.firstHash % m_capacity;
				insertAtHash(bucket.firstHash, std::move(bucket.firstKey));
				auto* current = bucket.collisions.getFront();
				while (current != nullptr)
				{
					insertAtHash(current->m_data.second, std::move(current->m_data.first));
					current = ListImpl::iterateNext(current);
				}
			}
		}

		delete[] oldTable;
		m_loadFactor = (float)m_size / m_capacity;
	}
};

