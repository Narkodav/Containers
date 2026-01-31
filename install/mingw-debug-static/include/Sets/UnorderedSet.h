#pragma once
#include "../HashTables/HashTables.h"

#include <stdexcept>
#include <cassert>

namespace Containers {
	//T must have operators < and == defined
	template <typename Type, typename HasherDef = std::hash<Type>, 
	typename ComparatorDef = std::equal_to<Type>, RawAllocatorType Allocator = RawAllocator,
	LifetimeManagerType<Type> Lifetime = LifetimeManager<Type>,
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
	class UnorderedSet
	{
	public:
		using HashTableImpl = HashTable<Type, HasherDef, ComparatorDef, Allocator, Lifetime, 
		s_initialCapacityDef, s_growthFactorDef, s_maxLoadFactorDef>;
		using Iterator = typename HashTableImpl::Iterator;
		using ConstIterator = typename HashTableImpl::ConstIterator;
		using ValueType = Type;
		using SizeType = size_t;

	private:
		HashTableImpl m_table;

	public:
		UnorderedSet() = default;
		~UnorderedSet() = default;
		UnorderedSet(const UnorderedSet&) = default;
		UnorderedSet& operator=(const UnorderedSet&) = default;

		UnorderedSet(UnorderedSet&&) = default;
		UnorderedSet& operator=(UnorderedSet&&) = default;

		template<typename... Args>
		void insert(Args&&... args)
		{
			m_table.insert(std::forward<Args>(args)...);
		}

		template<typename... Args>
		void insertHint(Iterator it, Args&&... args)
		{
			m_table.insertHint(it.bucket(), std::forward<Args>(args)...);
		}

		void erase(const ValueType& data)
		{
			auto* bucket = m_table.find(data);
			if (bucket == m_table.end()) return;
			m_table.erase(bucket);
		}

		void erase(Iterator it) {
			m_table.erase(it.bucket());
		}

		Iterator eraseAndAdvance(Iterator it) {
			return m_table.eraseAndAdvance(it.bucket());
		}

		Iterator find(const ValueType& data) {
			return Iterator(m_table.find(data));
		}

		ConstIterator find(const ValueType& data) const {
			return Iterator(m_table.find(data));
		}

		void reserve(SizeType newCapacity) {
			m_table.reserve(newCapacity);
		}

		SizeType size() const {
			return m_table.size();
		}

		SizeType capacity() const {
			return m_table.capacity();
		}

		bool empty() const {
			return m_table.empty();
		}

		Iterator begin() { return Iterator(m_table.begin()); }
		Iterator end() { return Iterator(m_table.end()); }

		ConstIterator begin() const { return Iterator(m_table.begin()); }
		ConstIterator end() const { return Iterator(m_table.end()); }

		ConstIterator cbegin() const { return begin(); }
		ConstIterator cend() const { return end(); }
	};
}