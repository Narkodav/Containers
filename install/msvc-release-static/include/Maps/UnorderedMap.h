#pragma once
#include "../HashTables/HashTables.h"
#include "MapPair.h"

#include <stdexcept>
#include <cassert>
#include <concepts>

namespace Containers {
	template<typename Key, typename Val, typename KeyHasher>
	struct MapHasher {
		using ValueType = std::pair<Key, Val>;
		[[no_unique_address]] KeyHasher hasher;
		inline size_t operator()(const std::pair<Key, Val>& pair) const {
			return hasher(pair.first);
		}
		inline size_t operator()(const Key& key) const {
			return hasher(key);
		}
	};

	template<typename Key, typename Val, typename KeyComparator>
	struct MapComparator {
		using ValueType = std::pair<Key, Val>;
		[[no_unique_address]] KeyComparator comparator;
		inline size_t operator()(const ValueType& left, const ValueType& right) const {
			return comparator(left.first, right.first);
		}
		inline size_t operator()(const ValueType& left, const Key& right) const {
			return comparator(left.first, right);
		}
		inline size_t operator()(const Key& left, const ValueType& right) const {
			return comparator(left, right.first);
		}
		inline size_t operator()(const Key& left, const Key& right) const {
			return comparator(left, right);
		}
	};

	template <typename Key, typename Val, typename PairHasher = std::hash<Key>, 
	typename PairComparator = std::equal_to<Key>, RawAllocatorType Allocator = RawAllocator,
	LifetimeManagerType<std::pair<Key, Val>> Lifetime = LifetimeManager<std::pair<Key, Val>>,
	size_t s_initialCapacityDef = 16, float s_growthFactorDef = 1.618f, float s_maxLoadFactorDef = 0.75f,
	template<typename,
        typename,
        typename,
        typename,
        typename,
        size_t,
        float,
        float>
	typename HashTable = HashTableOpenAddress>
	class UnorderedMap
	{
	public:
		using HashTableImpl = HashTable<std::pair<Key, Val>, MapHasher<Key, Val, PairHasher>, 
		MapComparator<Key, Val, PairComparator>, Allocator, Lifetime, 
		s_initialCapacityDef, s_growthFactorDef, s_maxLoadFactorDef>;

		using Pair = std::pair<Key, Val>;
		using Iterator = typename HashTableImpl::Iterator;
		using ConstIterator = typename HashTableImpl::ConstIterator;
		using ValueType = std::pair<Key, Val>;
		using SizeType = size_t;
		using Bucket = typename HashTableImpl::Bucket;

	private:
		HashTableImpl m_table;

	public:
		UnorderedMap() = default;
		~UnorderedMap() = default;
		UnorderedMap(const UnorderedMap&) = default;
		UnorderedMap& operator=(const UnorderedMap&) = default;

		UnorderedMap(UnorderedMap&&) = default;
		UnorderedMap& operator=(UnorderedMap&&) = default;


		template<typename K, typename V>
		void insert(K&& key, V&& val)
		{
			m_table.insert(std::forward<K>(key), std::forward<V>(val));
		}

		template<typename K, typename V>
		void insertHint(Iterator it, K&& key, V&& val)
		{
			m_table.insertHint(it.bucket(), ValueType(std::forward<K>(key), std::forward<V>(val)));
		}

		void erase(const Key& key)
		{
			auto* bucket = m_table.find(key);
			if (bucket == m_table.end()) return;
			m_table.erase(bucket);
		}

		void erase(Iterator it) {
			m_table.erase(it.bucket());
		}

		Iterator eraseAndAdvance(Iterator it) {
			return m_table.eraseAndAdvance(it.bucket());
		}

		Iterator find(const Key& key) {
			return Iterator(m_table.find(key));
		}

		ConstIterator find(const Key& key) const {
			return Iterator(m_table.find(key));
		}

		SizeType size() const { return m_table.size(); }
		SizeType capacity() const { return m_table.capacity(); }
		bool empty() const { return m_table.empty(); }

		Iterator begin() { return Iterator(m_table.begin()); }
		Iterator end() { return Iterator(m_table.end()); }

		ConstIterator begin() const { return Iterator(m_table.begin()); }
		ConstIterator end() const { return Iterator(m_table.end()); }

		ConstIterator cbegin() const { return begin(); }
		ConstIterator cend() const { return end(); }

		inline Iterator asMutable(ConstIterator it) {
			return Iterator(const_cast<Bucket *>(it.bucket()));
		}
	};
}