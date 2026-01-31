#pragma once
#include "../Lists/Lists.h"
#include "../Utilities/ReusableStorage.h"

#include <memory>
#include <stdexcept>

namespace Containers {

	template<typename T, typename S>
	struct OpenAddressSentinelBucket {
		static inline const S s_unoccupied = 0;
		static inline const S s_sentinel = 1;
		static inline const S s_minDistabce = 2;

		S probeInfo = s_unoccupied;
		
		bool occupied() const { return probeInfo != s_unoccupied; }
		bool sentinel() const { return probeInfo == s_sentinel; }
		void setUnoccupied() { probeInfo = s_unoccupied; }
		void setSentinel() { probeInfo = s_sentinel; }
		static inline S minDistance() { return s_minDistabce; };
		bool isMinDistance() const { return probeInfo == s_minDistabce; }
		S getDisplacedDistance() const { return probeInfo; }
		S getDistance() const { return probeInfo - s_minDistabce; }
		bool isIdealPlacement() const { return probeInfo == s_minDistabce; }
	};

	template<typename T, typename S>
	struct OpenAddressBucket : public OpenAddressSentinelBucket<T, S> {
		S hash;
		ReusableStorage<T> data;
	};

	template<typename T, typename S>
	class OpenAdressIteratorBase {
	private:
		using Bucket = OpenAddressBucket<T, S>;
		Bucket* m_bucket;
	public:
		using IteratorCategory = std::bidirectional_iterator_tag;
		using ValueType = T;
		using Pointer = ValueType *;
		using Reference = ValueType &;

		constexpr OpenAdressIteratorBase() noexcept = default;
		constexpr OpenAdressIteratorBase(Bucket *bucket) noexcept : m_bucket(bucket) {}

		template <typename U>
		constexpr OpenAdressIteratorBase(U *bucket) noexcept
			requires std::convertible_to<const U *, Bucket *>
			: m_bucket(bucket){};

		template <typename U>
		constexpr OpenAdressIteratorBase(OpenAdressIteratorBase<U, S> it) noexcept
			requires std::convertible_to<const U *, T *> : m_bucket(it.bucket()){

			};

		template <typename U>
		constexpr OpenAdressIteratorBase operator=(U *bucket) noexcept
			requires std::convertible_to<const U *, Bucket *>
		{
			m_bucket = bucket;
		};

		template <typename U>
		constexpr OpenAdressIteratorBase operator=(OpenAdressIteratorBase<U, S> it) noexcept
			requires std::convertible_to<const U *, T *>
		{
			m_bucket = it.bucket();
		};

		constexpr T &operator*() const noexcept { return m_bucket->data.value(); }
		constexpr T *operator->() const noexcept { return m_bucket->data.data(); }
		constexpr OpenAdressIteratorBase &operator++() noexcept
		{
			++m_bucket;
			while(!m_bucket->occupied()) ++m_bucket;
			return *this;
		}
		constexpr OpenAdressIteratorBase operator++(int) noexcept
		{
			OpenAdressIteratorBase temp = *this;
			++m_bucket;
			while(!m_bucket->occupied()) ++m_bucket;
			return temp;
		}
		constexpr OpenAdressIteratorBase &operator--() noexcept
		{
			--m_bucket;
			while(!m_bucket->occupied()) --m_bucket;
			return *this;
		}
		constexpr OpenAdressIteratorBase operator--(int) noexcept
		{
			OpenAdressIteratorBase temp = *this;
			--m_bucket;
			while(!m_bucket->occupied()) --m_bucket;
			return temp;
		}

		constexpr bool operator==(const OpenAdressIteratorBase &other) const noexcept
		{
			return m_bucket == other.m_bucket;
		}
		constexpr bool operator!=(const OpenAdressIteratorBase &other) const noexcept
		{
			return m_bucket != other.m_bucket;
		}
		constexpr bool operator<(const OpenAdressIteratorBase &other) const noexcept
		{
			return m_bucket < other.m_bucket;
		}
		constexpr bool operator<=(const OpenAdressIteratorBase &other) const noexcept
		{
			return m_bucket <= other.m_bucket;
		}
		constexpr bool operator>(const OpenAdressIteratorBase &other) const noexcept
		{
			return m_bucket > other.m_bucket;
		}
		constexpr bool operator>=(const OpenAdressIteratorBase &other) const noexcept
		{
			return m_bucket >= other.m_bucket;
		}
		constexpr ValueType* data() noexcept { return m_bucket->data.data(); }
		constexpr const ValueType* data() const noexcept { return m_bucket->data.data(); };
		constexpr Bucket* bucket() noexcept { return m_bucket; }
		constexpr const Bucket* bucket() const noexcept { return m_bucket; };
	};

	template<typename T, typename Hasher, typename Comparator, typename Alloc, typename Life, 
	size_t s_initialCapacity, float s_growthFactor, float s_maxLoadFactor>
	class HashTableOpenAddress
	{
	public:
		using ValueType = T;
		using SizeType = size_t;
		using Bucket = OpenAddressBucket<T, size_t>;
		using Sentinel = OpenAddressSentinelBucket<T, size_t>;
		using Iterator = OpenAdressIteratorBase<T, size_t>;
		using ConstIterator = OpenAdressIteratorBase<const T, size_t>;

	private:
		Bucket* m_buckets = nullptr;
		size_t m_capacity= 0;
		size_t m_size = 0;
		[[no_unique_address]] Hasher m_hasher;
		[[no_unique_address]] Alloc m_allocator;
		[[no_unique_address]] Life m_lifetimeManager;
		[[no_unique_address]] Comparator m_equal;

	public:
		HashTableOpenAddress() : m_buckets(allocate(s_initialCapacity)), 
		m_capacity(s_initialCapacity), m_size(0) {}
		
		~HashTableOpenAddress() {
			clear();
			deallocate(m_buckets, m_capacity);
		}

		HashTableOpenAddress(const HashTableOpenAddress& other): m_capacity(other.capacity()), m_size(other.size()),
			m_buckets(allocate(other.capacity())) {
			Bucket* it = m_buckets;
			const Bucket* itOther = other.m_buckets;
			for(; it < end(); ++it, ++itOther)
			{
				if (itOther->occupied())
					m_lifetimeManager.construct(it->data(), itOther->data());
				it->distanceFromIdeal = itOther->distanceFromIdeal;
				it->hash = itOther->hash;
			}
		}

		HashTableOpenAddress& operator=(const HashTableOpenAddress& other) {
			if(this == &other) return *this;
			clear();
			deallocate(m_buckets, m_capacity);

			m_capacity = other.capacity();
			m_size = other.size();
			m_buckets = allocate(m_capacity);

			Bucket* it = m_buckets;
			const Bucket* itOther = other.m_buckets;
			for(; it < end(); ++it, ++itOther)
			{				
				if (itOther->occupied())
					m_lifetimeManager.construct(it->data(), itOther->data());
				it->distanceFromIdeal = itOther->distanceFromIdeal;
				it->hash = itOther->hash;
			}
			return *this;
		}

		HashTableOpenAddress(HashTableOpenAddress&& other) {
			m_allocator = std::exchange(other.m_allocator, Alloc());
			m_lifetimeManager = std::exchange(other.m_lifetimeManager, Life());
			m_hasher = std::exchange(other.m_hasher, Hasher());
			m_equal = std::exchange(other.m_equal, Comparator());
			m_buckets = std::exchange(other.m_buckets, other.allocate(s_initialCapacity));
			m_capacity = std::exchange(other.capacity, s_initialCapacity);
			m_size = std::exchange(other.m_size, 0);
		}

		HashTableOpenAddress& operator=(HashTableOpenAddress&& other) {
			if(this == &other) return *this;
			clear();
			deallocate(m_buckets, m_capacity);

			m_allocator = std::exchange(other.m_allocator, Alloc());
			m_lifetimeManager = std::exchange(other.m_lifetimeManager, Life());
			m_hasher = std::exchange(other.m_hasher, Hasher());
			m_equal = std::exchange(other.m_equal, Comparator());
			m_buckets = std::exchange(other.m_buckets, other.allocate(s_initialCapacity));
			m_capacity = std::exchange(other.capacity, s_initialCapacity);
			m_size = std::exchange(other.m_size, 0);

			return *this;
		}
		
		void clear() {
			for(Bucket* bucket = m_buckets; bucket < m_buckets + m_capacity; ++bucket)
			{
				if(bucket->occupied())
				{
					m_lifetimeManager.destroy(bucket->data.data());
					bucket->setUnoccupied();
				}
			}
			m_size = 0;
		}

		template<typename... Args>
		void insert(Args&&... args) {
			if(static_cast<float>(m_size) / static_cast<float>(m_capacity) >= s_maxLoadFactor)
				grow((m_capacity + 1) * s_growthFactor);
			T value(std::forward<Args>(args)...);
			SizeType hash = m_hasher(value);
			m_size += insertImpl(std::move(value), hash, m_buckets, m_capacity);
		}

		template<typename... Args>
		void insertHint(SizeType hash, Args&&... args) {			
			if(static_cast<float>(m_size) / static_cast<float>(m_capacity) >= s_maxLoadFactor)
				grow((m_capacity + 1) * s_growthFactor);
			T value(std::forward<Args>(args)...);
			CONTAINERS_VERIFY(hash == m_hasher(value), "Hinted hash doesnt match actual hash");
			m_size += insertImpl(std::move(value), hash, m_buckets, m_capacity);
		}

		template<typename... Args>
		void insertHint(Bucket* it, Args&&... args) {			
			if(static_cast<float>(m_size) / static_cast<float>(m_capacity) >= s_maxLoadFactor)
				grow((m_capacity + 1) * s_growthFactor);
			T value(std::forward<Args>(args)...);
			CONTAINERS_VERIFY(m_equal(it->data.value(), value), "Hinted bucket doesnt match passed value");
			it->data.value() = std::move(value);
		}

		// If the requested new capacity is smaller than current size / max_load_factor,
		// the table will rehash automatically on the next insert
		void rehash(size_t newCapacity) {
			if(newCapacity <= m_capacity)
				return;
			grow(newCapacity);
		}

		template<typename U>
		Bucket* find(const U& value) {
			return const_cast<Bucket*>(std::as_const(*this).find(value));
		}

		template<typename U>
		const Bucket* find(const U& value) const {
			SizeType hash = m_hasher(value);
			SizeType index = hash % m_capacity;
			SizeType distance = Bucket::minDistance();
			const Bucket* bucket = m_buckets + index;
			return searchRange(bucket, m_buckets + m_capacity, index, distance, value);
		}

		void erase(Bucket* it) {
			eraseImpl(it);
		}

		[[nodiscard]]
		Bucket* eraseAndAdvance(Bucket* it) {
			eraseImpl(it);
			while(!it->occupied()) ++it;
			return it;
		}

        bool empty() const { return m_size == 0; }
        SizeType size() const { return m_size; }
		SizeType capacity() const { return m_capacity; }

        Bucket* begin() {
			Bucket* bucket = m_buckets;
			while(!bucket->occupied()) ++bucket;
			return bucket;
		}
        Bucket* end() { return static_cast<Bucket*>(sentinel()); }
        const Bucket* begin() const { 
			const Bucket* bucket = m_buckets;
			while(!bucket->occupied()) ++bucket;
			return bucket;
		}
        const Bucket* end() const { return static_cast<const Bucket*>(sentinel()); }
        const Bucket* cbegin() const { return begin(); }
        const Bucket* cend() const { return end(); }

	private:

		void grow(size_t newCapacity) {
			Bucket* newBuckets = allocate(newCapacity);

			for(Bucket* bucket = m_buckets; bucket < m_buckets + m_capacity; ++bucket)
			{
				if(bucket->occupied())
				{
					insertUniqueImpl(std::move(bucket->data.value()), bucket->hash, newBuckets, newCapacity);
					m_lifetimeManager.destroy(bucket->data.data());
				}
			}
			
			deallocate(m_buckets, m_capacity);
			m_buckets = newBuckets;
			m_capacity = newCapacity;
		}
		
		SizeType insertImpl(T&& value, SizeType hash, Bucket* buckets, SizeType capacity) {
			SizeType distance = Bucket::minDistance();
			SizeType index = hash % capacity;
			Bucket* bucket = buckets + index;
			return probeFullRange<true>(std::move(value), distance, hash, bucket, buckets, capacity, index);			
		}

		SizeType insertUniqueImpl(T&& value, SizeType hash, Bucket* buckets, SizeType capacity) {			
			SizeType distance = Bucket::minDistance();
			SizeType index = hash % capacity;
			Bucket* bucket = buckets + index;
			return probeFullRange<false>(std::move(value), distance, hash, bucket, buckets, capacity, index);			
		}

		template<bool firstPass>
		inline SizeType probeFullRange(T&& value, SizeType& distance, SizeType& hash, Bucket* bucket, 
			Bucket* buckets, SizeType capacity, SizeType index) {
			for(; bucket < buckets + capacity; ++bucket, ++distance)
			{
				if(!bucket->occupied())
				{
					bucket->hash = hash;
					bucket->probeInfo = distance;
					m_lifetimeManager.construct(bucket->data.data(), std::move(value));
					return 1;
				}
				if constexpr(firstPass) {
					if (bucket->probeInfo == distance && m_equal(bucket->data.value(), value))
					{
						bucket->data.value() = std::move(value);
						return 0;
					}
				}
				if(bucket->probeInfo < distance)
				{
					distance = std::exchange(bucket->probeInfo, distance);
					hash = std::exchange(bucket->hash, hash);
					value = std::move(std::exchange(bucket->data.value(), std::move(value)));
					if constexpr(firstPass) {
						++bucket;
						++distance;
						return probeFullRange<false>(std::move(value), distance, hash, bucket, buckets, capacity, index);
					}
				}
			}
			bucket = buckets;
			return probeLastRange<firstPass>(std::move(value), distance, hash, bucket, buckets, capacity, index);
		}

		template<bool firstPass>
		inline SizeType probeLastRange(T&& value, SizeType& distance, SizeType& hash, Bucket* bucket, 
			Bucket* buckets, SizeType capacity, SizeType index) {
			for(; bucket < buckets + index; ++bucket, ++distance)
			{
				if(!bucket->occupied())
				{
					bucket->probeInfo = distance;
					bucket->hash = hash;
					m_lifetimeManager.construct(bucket->data.data(), std::move(value));
					return 1;
				}
				if constexpr(firstPass) {
					if (bucket->probeInfo == distance && m_equal(bucket->data.value(), value))
					{
						bucket->data.value() = std::move(value);
						return 0;
					}
				}
				if(bucket->probeInfo < distance)
				{
					distance = std::exchange(bucket->probeInfo, distance);
					hash = std::exchange(bucket->hash, hash);
					value = std::move(std::exchange(bucket->data.value(), std::move(value)));
					if constexpr(firstPass) {
						++bucket;
						++distance;
						return probeLastRange<false>(std::move(value), distance, hash, bucket, buckets, capacity, index);
					}
				}
			}
			if constexpr(!firstPass) {
				throw std::runtime_error("Hash table out of space");
			}
			return 0;
		}

		template<typename U, bool firstPass = true>
		const Bucket* searchRange(const Bucket* bucket, const Bucket* endBucket, 
			SizeType index, SizeType distance, const U& value) const
		{
			for(; bucket < endBucket; ++bucket, ++distance)
			{
				if(bucket->probeInfo < distance) return end();
				else if(bucket->probeInfo == distance && m_equal(bucket->data.value(), value))
					return bucket;
			}
			if constexpr(firstPass) {
				bucket = m_buckets;
				return searchRange<U, false>(bucket, m_buckets + index, index, distance, value);
			}
			return end();
		}

		Bucket* allocate(SizeType capacity) {
			SizeType byteCount = capacity * sizeof(Bucket) + sizeof(Sentinel);
			void* newData = m_allocator.allocate(byteCount, alignof(Bucket));
			std::memset(newData, 0, byteCount);
			Bucket* newBuckets = static_cast<Bucket*>(newData);
			reinterpret_cast<Sentinel*>(newBuckets + capacity)->setSentinel();
			return newBuckets;
		}

		void deallocate(Bucket* buckets, SizeType capacity) {
			m_allocator.deallocate(buckets, capacity * sizeof(Bucket) + sizeof(Sentinel), alignof(Bucket));
		}

		Sentinel* sentinel() { return reinterpret_cast<Sentinel*>(m_buckets + m_capacity); }
		const Sentinel* sentinel() const { return reinterpret_cast<const Sentinel*>(m_buckets + m_capacity); }

		void eraseImpl(Bucket* it) {
			Bucket* bucket = it;
			Bucket* next = bucket + 1;
			for(; next < m_buckets + m_capacity; ++bucket, ++next)
			{
				if(next->isIdealPlacement() || !next->occupied())
				{
					m_lifetimeManager.destroy(bucket->data.data());
					bucket->setUnoccupied();
					--m_size;
					return;
				}
				bucket->data.value() = std::move(next->data.value());
				bucket->probeInfo = next->probeInfo - 1;
				bucket->hash = next->hash;
			}
			next = m_buckets;
			if(next->isIdealPlacement() || !next->occupied())
			{
				m_lifetimeManager.destroy(bucket->data.data());
				bucket->setUnoccupied();
				--m_size;
				return;
			}
			bucket->data.value() = std::move(next->data.value());
			bucket->probeInfo = next->probeInfo - 1;
			bucket->hash = next->hash;
			++next;
			bucket = m_buckets;
			for(; next < it; ++bucket, ++next)
			{
				if(next->isIdealPlacement() || !next->occupied())
				{
					m_lifetimeManager.destroy(bucket->data.data());
					bucket->setUnoccupied();
					--m_size;
					return;
				}
				bucket->data.value() = std::move(next->data.value());
				bucket->probeInfo = next->probeInfo - 1;
				bucket->hash = next->hash;
			}
		}
	};
}