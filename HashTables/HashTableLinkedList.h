#pragma once
#include "Lists/Lists.h"

template<typename Key, typename Val, typename Hasher = std::hash<Key>, ListType ListImpl = ListOneSided<Val>>
class HashTableChained
{
public:
	struct Bucket
	{
		Key key;
		Val firstVal;
		ListImpl collisions;
		bool isOccupied = false;
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

	template<typename K, typename V>
		requires std::is_same_v<Key, std::decay_t<K>> && std::is_same_v<Val, std::decay_t<V>>
	void insert(K&& key, V&& val)
	{
		if (m_loadFactor > maxLoadFactor)
			rehash();

		size_t index = Hasher(key) % m_capacity;
		Bucket& bucket = m_table[index];

		if (bucket.isOccupied)
			bucket.collisions.insertFront(std::forward<V>(val));
		else
		{
			bucket.key = std::forward<K>(key);
			bucket.firstVal = std::forward<V>(val);
			bucket.isOccupied = true;
			m_size++;
			m_loadFactor = (float)m_size / m_capacity;
		}
	}

	void rehash()
	{
		size_t newCapacity = m_capacity * growthFactor;
		Bucket* newTable = new Bucket[newCapacity];

		for (size_t i = 0; i < m_capacity; i++)
		{
			Bucket& bucket = m_table[i];
			if (bucket.key != Key())
			{
				size_t newIndex = Hasher(bucket.key) % m_capacity;;
				Bucket& newBucket = newTable[newIndex];
				newBucket.key = bucket.key;
				newBucket.firstVal = bucket.firstVal;
				newBucket.collisions = std::move(bucket.collisions);
			}
		}

		delete[] m_table;
		m_table = newTable;
		m_capacity = newCapacity;
		m_loadFactor = (float)m_size / m_capacity;
	}


};

