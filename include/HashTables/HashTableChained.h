#pragma once
#include "Lists/Lists.h"
#include "Utilities/UnionStorage.h"

#include <memory>
#include <stdexcept>

namespace Containers {
	template<typename Key, typename Hasher, ListType ListImpl = ListOneSided<std::pair<Key, size_t>>>
	class HashTableChained
	{
	public:
		using ValueType = Key;
		using KeyHashPair = std::pair<Key, size_t>;
		using TableType = HashTableChained<Key, Hasher, ListImpl>;

		struct Bucket
		{
			size_t firstHash = 0;
			UnionStorage<Key> firstKey;
			ListImpl collisions;

			Bucket() : firstKey() {};

			Bucket(const Bucket&) = delete;
			Bucket& operator=(const Bucket&) = delete;
			Bucket(Bucket&&) = delete;
			Bucket& operator=(Bucket&&) = delete;

			~Bucket() noexcept(std::is_nothrow_destructible_v<Key>)
			{
				if (firstKey.isEngaged())
					firstKey.destroy();
			}
		};

		struct Node
		{
			ListImpl::Node* currentNode = nullptr; //if nullptr the value is first stored
			Bucket* bucket = nullptr;
			const TableType* table = nullptr;

			Node(ListImpl::Node* currentNode = nullptr, Bucket* bucket = nullptr, const TableType* table = nullptr) :
				currentNode(currentNode), bucket(bucket), table(table) {
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
				if (currentNode == nullptr)
					return bucket->firstKey.get();
				else return currentNode->m_data.first;
			}

			bool operator==(const Node& other) const
			{
				return bucket == other.bucket && currentNode == other.currentNode;
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
		HashTableChained() : m_table(new Bucket[initialSize]), m_size(0),
			m_capacity(initialSize), m_loadFactor(0.f)
		{
		};

		~HashTableChained() {
			if (m_table != nullptr)
				delete[] m_table;
		};

		HashTableChained(const HashTableChained& other)
			requires std::is_copy_constructible_v<Key> || std::is_copy_assignable_v<Key> :
		m_table(new Bucket[other.m_capacity]), m_size(other.m_size),
			m_capacity(other.m_capacity), m_loadFactor(other.m_loadFactor)
		{
			for (size_t i = 0; i < m_capacity; i++)
				if (other.m_table[i].firstKey.isEngaged())
				{
					m_table[i].firstHash = other.m_table[i].firstHash;
					m_table[i].firstKey.copyConstructFrom(other.m_table[i].firstKey);
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
				if (other.m_table[i].firstKey.isEngaged())
				{
					m_table[i].firstHash = other.m_table[i].firstHash;
					m_table[i].firstKey.copyConstructFrom(other.m_table[i].firstKey);
					m_table[i].collisions = other.m_table[i].collisions;
				}

			return *this;
		}

		HashTableChained(HashTableChained&& other) noexcept
			: m_table(other.m_table),
			m_size(other.m_size),
			m_capacity(other.m_capacity),
			m_loadFactor(other.m_loadFactor) {
			other.m_table = nullptr;
			other.m_size = 0;
			other.m_capacity = 0;
			other.m_loadFactor = 0.f;
		}

		HashTableChained& operator=(HashTableChained&& other) noexcept {
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
			Bucket& bucket = m_table[index];

			if (bucket.firstKey.isEngaged())
			{
				if (bucket.firstKey.get() == key)
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

			if (!bucket.firstKey.isEngaged())
				return Node{ nullptr, nullptr, this };

			// Check first element
			if (bucket.firstKey.get() == key)
				return Node{ nullptr, &bucket, this };

			// Check collision chain
			for (auto current = bucket.collisions.getFront(); current != nullptr;
				current = ListImpl::iterateNext(current))
			{
				if (current->m_data.first == key)
					return Node{ current, &bucket, this };
			}

			return Node{ nullptr, nullptr, this };
		}

		bool erase(const Key& key)
		{
			size_t index = Hasher(key) % m_capacity;
			Bucket& bucket = m_table[index];

			if (!bucket.firstKey.isEngaged())
				return false;

			if (bucket.firstKey.get() == key)
			{
				if (bucket.collisions.getFront() == nullptr)
				{
					bucket.firstKey.destroy();
					bucket.isOccupied = false;
					bucket.firstHash = 0;
				}
				else
				{
					bucket.firstKey.destroy();
					auto* head = bucket.collisions.getFront();
					if constexpr (std::is_move_assignable_v<Key>)
					{
						bucket.firstKey.moveConstruct(std::move(head->m_data.first));
					}
					else if constexpr (std::is_copy_assignable_v<Key>)
					{
						bucket.firstKey.copyConstruct(head->m_data.first);
					}
					else static_assert(false, "Key type must be copy or move assignable");
					bucket.firstHash = head->m_data.second;
					bucket.collisions.deleteFront();
				}
				m_size--;
				m_loadFactor = (float)m_size / m_capacity;
				return true;
			}

			for (auto current = bucket.collisions.getFront(); current != nullptr;
				current = ListImpl::iterateNext(current))
				if (current->m_data.first == key)
				{
					bucket.collisions.deleteNode(current);
					m_size--;
					m_loadFactor = (float)m_size / m_capacity;
					return true;
				}

			return false;
		}

		void erase(const Node& node)
		{
			if (node.bucket == nullptr)
				throw std::runtime_error("iterating with an invalid node");
			Bucket& bucket = *node.bucket;
			if (node.currentNode != nullptr)
				bucket.collisions.deleteNode(node.currentNode);
			else if (bucket.collisions.getFront() != nullptr)
			{
				bucket.firstKey.destroy();
				auto* head = bucket.collisions.getFront();
				if constexpr (std::is_move_assignable_v<Key>)
				{
					bucket.firstKey.moveConstruct(std::move(head->m_data.first));
				}
				else if constexpr (std::is_copy_assignable_v<Key>)
				{
					bucket.firstKey.copyConstruct(head->m_data.first);
				}
				else static_assert(false, "Key type must be copy or move assignable");
				bucket.firstHash = head->m_data.second;
				bucket.collisions.deleteFront();
			}
			else
			{
				bucket.firstKey.destroy();
				bucket.firstHash = 0;
			}
			m_size--;
			m_loadFactor = (float)m_size / m_capacity;
		}

		static Node iterateNext(const Node& node)
		{
			if (node.bucket == nullptr)
				throw std::runtime_error("iterating with an invalid node");

			Node newNode;
			newNode.bucket = node.bucket;
			newNode.currentNode = node.currentNode;
			newNode.table = node.table;

			if (newNode.currentNode == nullptr) {
				if (newNode.bucket->collisions.getFront() != nullptr)
				{
					// Move to first collision if it exists
					newNode.currentNode = newNode.bucket->collisions.getFront();
					return newNode;
				}
			}
			else {
				// Currently in collision list
				newNode.currentNode = ListImpl::iterateNext(newNode.currentNode);
				if (newNode.currentNode != nullptr) {
					return newNode;
				}
			}

			// Move to next bucket
			size_t capacity = newNode.table->m_capacity;
			size_t index = newNode.bucket - newNode.table->m_table;
			while (true) {
				index++;
				newNode.bucket++;
				if (index >= capacity)
				{
					newNode.bucket = nullptr;
					newNode.table = nullptr;
					break;
				}
				else if (newNode.bucket->firstKey.isEngaged())
					break;
			};
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
				if (m_table[i].firstKey.isEngaged())
					return Node(nullptr, m_table + i, this);
			}
			return Node(nullptr, nullptr, this);
		}

		Node end() const
		{
			return Node(nullptr, nullptr, this);
		}

	private:
		template<bool ShouldReturnNode, typename K>
			requires std::is_same_v<Key, std::decay_t<K>>
		std::conditional_t<ShouldReturnNode, Node, void> insertAtHash(size_t hash, K&& key)
		{
			size_t index = hash % m_capacity;
			Bucket& bucket = m_table[index];

			if (bucket.firstKey.isEngaged())
			{
				if (bucket.firstKey.get() == key)
				{
					bucket.firstKey.destroy();
					bucket.firstKey.perfectForwardConstruct(std::forward<K>(key));
					bucket.firstHash = hash;
				}
				else
				{
					for (auto* current = bucket.collisions.getFront();
						current != nullptr; current = ListImpl::iterateNext(current))
					{
						if (current->m_data.first == key)
						{
							current->m_data.first = std::forward<K>(key);
							current->m_data.second = hash;
							if constexpr (ShouldReturnNode)
								return Node(current, m_table + index, this);
							else return;
						}
					}
					bucket.collisions.insertFront(KeyHashPair(std::forward<K>(key), hash));
					m_size++;
					m_loadFactor = (float)m_size / m_capacity;
					if constexpr (ShouldReturnNode)
						return Node(bucket.collisions.getFront(), m_table + index, this);
					else return;
				}
			}
			else
			{
				//no need to destroy here, firstKey is disengaged
				bucket.firstHash = hash;
				bucket.firstKey.perfectForwardConstruct(std::forward<K>(key));
				m_size++;
				m_loadFactor = (float)m_size / m_capacity;
			}
			if constexpr (ShouldReturnNode)
				return Node(nullptr, m_table + index, this);
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
				Node buffer;
				if (oldNode.currentNode == nullptr)
				{
					if constexpr (std::is_move_assignable_v<Key>)
						buffer = insertAtHash<true>(bucket.firstHash, std::move(bucket.firstKey.get()));
					else if constexpr (std::is_copy_assignable_v<Key>)
						buffer = insertAtHash<true>(bucket.firstHash, bucket.firstKey.get());
					else static_assert(false, "Key type must be copy or move assignable");
					bucket.firstKey.destroy();

					if (bucket.collisions.getFront() != nullptr)
					{
						bucket.firstHash = bucket.collisions.getFront()->m_data.second;
						if constexpr (std::is_move_assignable_v<Key>)
							bucket.firstKey.moveConstruct(std::move(bucket.collisions.getFront()->m_data.first));
						else if constexpr (std::is_copy_assignable_v<Key>)
							bucket.firstKey.copyConstruct(bucket.collisions.getFront()->m_data.first);
						else static_assert(false, "Key type must be copy or move assignable");

						bucket.collisions.deleteFront();
					}
				}
				else
				{
					if constexpr (std::is_move_assignable_v<Key>)
						buffer = insertAtHash<true>
						(oldNode.currentNode->m_data.second, std::move(oldNode.currentNode->m_data.first));
					else if constexpr (std::is_copy_assignable_v<Key>)
						buffer = insertAtHash<true>
						(oldNode.currentNode->m_data.second, oldNode.currentNode->m_data.first);
					else static_assert(false, "Key type must be copy or move assignable");

					bucket.collisions.deleteNode(oldNode.currentNode);
				}
				oldNode = buffer;
			}

			for (size_t i = 0; i < oldCapacity; i++)
			{
				Bucket& bucket = oldTable[i];
				if (bucket.firstKey.isEngaged())
				{
					size_t newIndex = bucket.firstHash % m_capacity;
					if constexpr (std::is_move_assignable_v<Key>)
						insertAtHash<false>(bucket.firstHash, std::move(bucket.firstKey.get()));
					else if constexpr (std::is_copy_assignable_v<Key>)
						insertAtHash<false>(bucket.firstHash, bucket.firstKey.get());
					else static_assert(false, "Key type must be copy or move assignable");
					bucket.firstKey.destroy();
					auto* current = bucket.collisions.getFront();
					while (current != nullptr)
					{
						if constexpr (std::is_move_assignable_v<Key>)
							insertAtHash<false>(current->m_data.second, std::move(current->m_data.first));
						else if constexpr (std::is_copy_assignable_v<Key>)
							insertAtHash<false>(current->m_data.second, current->m_data.first);
						else static_assert(false, "Key type must be copy or move assignable");
						current = ListImpl::iterateNext(current);
					}
				}
			}

			delete[] oldTable;
			m_loadFactor = (float)m_size / m_capacity;
		}
	};

}