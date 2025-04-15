#pragma once
#include "Lists/Lists.h"
#include "Utilities/UnionStorage.h"

#include <memory>
#include <stdexcept>

template<typename Key, typename Hasher>
class HashTableOpenAddress
{
public:
	using ValueType = Key;
	using KeyHashPair = std::pair<Key, size_t>;
	using TableType = HashTableOpenAddress<Key, Hasher>;

	struct Bucket
	{
		size_t hash = 0;
		UnionStorage<Key> key;
		size_t probingDistance = 0;

		Bucket() : key() {};

		Bucket(const Bucket&) = delete;
		Bucket& operator=(const Bucket&) = delete;
		Bucket(Bucket&&) = delete;
		Bucket& operator=(Bucket&&) = delete;

		~Bucket() noexcept(std::is_nothrow_destructible_v<Key>)
		{
			if (key.isEngaged())
				key.destroy();
		}
	};

	struct Node
	{
		Bucket* bucket = nullptr;
		const TableType* table = nullptr;

		Node(Bucket* bucket = nullptr, const TableType* table = nullptr) :
			bucket(bucket), table(table) {
		};
		~Node() = default;

		// Only enable copy operations if T is copyable
		Node(const Node&) = default;
		Node& operator=(const Node&) = default;

		// Only enable move operations if T is moveable
		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;

		Key& getKey()
		{
			return const_cast<Key&>(static_cast<const Node*>(this)->getKey());
		}

		const Key& getKey() const
		{
			if (bucket == nullptr)
				throw std::runtime_error("Dereferencing end list iterator");
			bucket->firstKey.get();
		}

		bool operator==(const Node& other) const
		{
			return bucket == other.bucket;
		}

		bool isValid() const
		{
			return bucket != nullptr;
		}
	};

	static inline const float maxLoadFactor = 0.75f;
	static inline const size_t initialSize = 16;
	static inline const float growthFactor = 2;

private:

	Bucket* m_table = nullptr;
	size_t m_size = 0;
	size_t m_capacity = initialSize;
	float m_loadFactor = 0.f;

public:
	HashTableOpenAddress() : m_table(new Bucket[initialSize]), m_size(0),
		m_capacity(initialSize), m_loadFactor(0.f)
	{
	};

	~HashTableOpenAddress() {
		if (m_table != nullptr)
			delete[] m_table;
	};

	HashTableOpenAddress(const HashTableOpenAddress& other)
		requires std::is_copy_constructible_v<Key> || std::is_copy_assignable_v<Key> :
	m_table(new Bucket[other.m_capacity]), m_size(other.m_size),
		m_capacity(other.m_capacity), m_loadFactor(other.m_loadFactor)
	{
		for (size_t i = 0; i < m_capacity; i++)
			if (other.m_table[i].firstKey.isEngaged())
			{
				m_table[i].hash = other.m_table[i].hash;
				m_table[i].probingDistance = other.m_table[i].probingDistance;
				m_table[i].key.copyConstructFrom(other.m_table[i].key);
			}
	}

	HashTableOpenAddress& operator=(const HashTableOpenAddress& other)
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
			if (other.m_table[i].firstKey.isEngaged())
			{
				m_table[i].hash = other.m_table[i].hash;
				m_table[i].probingDistance = other.m_table[i].probingDistance;
				m_table[i].key.copyConstructFrom(other.m_table[i].key);
			}

		return *this;
	}

	HashTableOpenAddress(HashTableOpenAddress&& other) noexcept
		: m_table(other.m_table),
		m_size(other.m_size),
		m_capacity(other.m_capacity),
		m_loadFactor(other.m_loadFactor) {
		other.m_table = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
		other.m_loadFactor = 0.f;
	}

	HashTableOpenAddress& operator=(HashTableOpenAddress&& other) noexcept {
		if (this == &other)
			return *this;
		m_table = std::exchange(other.m_table, nullptr);
		m_size = std::exchange(other.m_size, 0);
		m_capacity = std::exchange(other.m_capacity, 0);
		m_loadFactor = std::exchange(other.m_loadFactor, 0.f);
		return *this;
	}

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
		Node node = insertAtHash<true>(hash, std::forward<K>(key));
		if (m_loadFactor > maxLoadFactor)
			rehash<true>(growthFactor * m_capacity, node);
		return node;
	}

	template<typename K>
		requires std::is_same_v<Key, std::decay_t<K>>
	bool contains(K&& key) const
	{
		size_t index = Hasher()(key) % m_capacity;
		Bucket* bucket = m_table + index;

		while (bucket->firstKey.isEngaged())
		{
			if(bucket->firstKey.get() == key)
				return true;
			++index;
			index %= m_capacity;
			bucket = m_table + index;
		}
		return false;
	}

	Node find(const Key& key) const
	{
		size_t index = Hasher()(key) % m_capacity;

		while (m_table[index].key.isEngaged())
		{
			if (m_table[index].key.get() == key)
				return Node{ m_table + index, this };
			++index;
			index %= m_capacity;
		}
		return Node{ nullptr, this };
	}

	bool erase(const Key& key)
	{
		size_t index = Hasher()(key) % m_capacity;
		size_t probeDistance = 0;

		// Find the key
		while (m_table[index].key.isEngaged())
		{
			if (m_table[index].key.get() == key)
			{
				// Found the key, now perform backward shift
				size_t nextIndex = index + 1;
				nextIndex %= m_capacity;

				// Keep shifting elements backward as long as they have non-zero probe distance
				while (m_table[nextIndex].key.isEngaged() &&
					m_table[nextIndex].probingDistance > 0)
				{
					m_table[index].key.destroy();
					// Move next element back
					if constexpr (std::is_move_assignable_v<Key>) {
						m_table[index].key.moveConstruct(
							std::move(m_table[nextIndex].key));
					}
					else if constexpr (std::is_copy_assignable_v<Key>) {
						m_table[index].key.copyConstruct(
							m_table[nextIndex].key);
					}
					else static_assert(false, "Key type must be copy or move assignable");
					m_table[index].hash = m_table[nextIndex].hash;
					m_table[index].probingDistance =
						m_table[nextIndex].probingDistance - 1;

					// Move to next pair of elements
					index = nextIndex;
					++nextIndex;
					nextIndex %= m_capacity;
				}

				// Clear the last shifted position
				m_table[index].key.destroy();
				m_table[index].hash = 0;
				m_table[index].probingDistance = 0;
				m_size--;
				m_loadFactor = (float)m_size / m_capacity;
				return true;
			}

			// If we find an element with probe distance less than ours,
			// the key cannot be in the table
			if (m_table[index].probingDistance < probeDistance)
				return false;
			++index;
			index %= m_capacity;
			probeDistance++;
		}
		return false;
	}

	void erase(const Node& node)
	{
		if (node.bucket == nullptr)
			throw std::runtime_error("iterating with an invalid node");
		size_t index = node.bucket - m_table;
		size_t nextIndex = (index + 1) % m_capacity;
		if (!m_table[index].key.isEngaged() ||
			m_table[index].probingDistance == 0)
		{
			m_table[index].key.destroy();
			m_table[index].hash = 0;
			m_table[index].probingDistance = 0;
			m_size--;
			m_loadFactor = (float)m_size / m_capacity;
			return;
		}

		// Keep shifting elements backward as long as they have non-zero probe distance
		do
		{
			m_table[index].key.destroy();
			// Move next element back
			if constexpr (std::is_move_constructible_v<Key>) {
				m_table[index].key.moveConstructFrom(m_table[nextIndex].key);
			}
			else if constexpr (std::is_copy_constructible_v<Key>) {
				m_table[index].key.copyConstructFrom(m_table[nextIndex].key);
			}
			else static_assert(false, "Key type must be copy or move assignable");
			m_table[index].hash = m_table[nextIndex].hash;
			m_table[index].probingDistance =
				m_table[nextIndex].probingDistance - 1;

			// Move to next pair of elements
			index = nextIndex;
			++nextIndex;
			nextIndex %= m_capacity;
		} while (m_table[nextIndex].key.isEngaged() &&
			m_table[nextIndex].probingDistance > 0);

		// Clear the last shifted position
		m_table[index].key.destroy();
		m_table[index].hash = 0;
		m_table[index].probingDistance = 0;
		m_size--;
		m_loadFactor = (float)m_size / m_capacity;
	}

	static Node iterateNext(const Node& node)
	{
		if (node.bucket == nullptr)
			throw std::runtime_error("iterating with an invalid node");
		Node newNode = node;
		++newNode.bucket;
		if (node.bucket - node.table->m_table >= node.table->m_capacity)
			return Node(nullptr, node.table);
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
			if (m_table[i].key.isEngaged())
				return Node(m_table + i, this);
		}
		return Node(nullptr, this);
	}

	Node end() const
	{
		return Node(nullptr, this);
	}

private:
	template<bool ShouldReturnNode, typename K>
		requires std::is_same_v<Key, std::decay_t<K>>
	std::conditional_t<ShouldReturnNode, Node, void> insertAtHash(size_t hash, K&& key)
	{
		// Check if table is full
		if (m_size >= m_capacity)
			throw std::runtime_error("Hash table is full");

		size_t index = hash % m_capacity;
		size_t probeDistance = 0;

		if (!m_table[index].key.isEngaged())
		{
			// Found empty slot, insert here
			m_table[index].key.perfectForwardConstruct(std::forward<K>(key));
			m_table[index].hash = hash;
			m_table[index].probingDistance = probeDistance;
			m_size++;
			m_loadFactor = (float)m_size / m_capacity;

			if constexpr (ShouldReturnNode)
				return Node(m_table + index, this);
			else
				return;
		}

		Key storedKey = std::forward<K>(key);

		while (m_table[index].key.isEngaged())
		{
			// If current element has a lower probe distance, swap with it (Robin Hood)
			if (m_table[index].probingDistance < probeDistance)
			{
				// Swap elements
				Key tempKey = std::move(storedKey);
				size_t tempHash = hash;
				size_t tempProbeDistance = probeDistance;

				storedKey = std::move(m_table[index].key.get());
				hash = m_table[index].hash;
				probeDistance = m_table[index].probingDistance;

				m_table[index].key.destroy();
				if constexpr (std::is_move_assignable_v<Key>) {
					m_table[index].key.moveConstruct(std::move(tempKey));						
				}
				else if constexpr (std::is_copy_assignable_v<Key>) {
					m_table[index].key.copyConstruct(tempKey);
				}
				else static_assert(false, "Key type must be copy or move assignable");
				m_table[index].hash = tempHash;
				m_table[index].probingDistance = tempProbeDistance;
			}

			// Move to next slot
			++index;
			index %= m_capacity;
			++probeDistance;
		}

		//this node must be empty, insert here
		if constexpr (std::is_move_assignable_v<Key>) {
			m_table[index].key.moveConstruct(std::move(storedKey));
		}
		else if constexpr (std::is_copy_assignable_v<Key>) {
			m_table[index].key.copyConstruct(storedKey);
		}
		else static_assert(false, "Key type must be copy or move assignable");
		m_table[index].hash = hash;
		m_table[index].probingDistance = probeDistance;
		m_size++;
		m_loadFactor = (float)m_size / m_capacity;

		if constexpr (ShouldReturnNode)
			return Node(m_table + index, this);
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
			Node buffer;
			if constexpr (std::is_move_assignable_v<Key>)
				buffer = insertAtHash<true>(bucket.hash, std::move(bucket.key.get()));
			else if constexpr (std::is_copy_assignable_v<Key>)
				buffer = insertAtHash<true>(bucket.hash, bucket.key.get());
			else static_assert(false, "Key type must be copy or move assignable");
			bucket.key.destroy();
			oldNode = buffer;
		}

		for (size_t i = 0; i < oldCapacity; i++)
		{
			Bucket& bucket = oldTable[i];
			if (bucket.key.isEngaged())
			{
				size_t newIndex = bucket.hash % m_capacity;
				if constexpr (std::is_move_assignable_v<Key>)
					insertAtHash<false>(bucket.hash, std::move(bucket.key.get()));
				else if constexpr (std::is_copy_assignable_v<Key>)
					insertAtHash<false>(bucket.hash, bucket.key.get());
				else static_assert(false, "Key type must be copy or move assignable");
				bucket.key.destroy();
			}
		}

		delete[] oldTable;
		m_loadFactor = (float)m_size / m_capacity;
	}
};

