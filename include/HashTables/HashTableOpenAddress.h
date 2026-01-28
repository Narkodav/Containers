#pragma once
#include "../Lists/Lists.h"
#include "../Utilities/ReusableStorage.h"

#include <memory>
#include <stdexcept>

namespace Containers {
	template<typename T, typename Hasher, TypedAllocatorType<T> Alloc = TypedAllocator<T>,
		LifetimeManagerType<T> Life = LifetimeManager<T>, size_t s_initialCapacity = 16, 
		float s_growthFactor = 1.618f, float s_maxLoadFactor = 0.75f>
	class HashTableOpenAddress
	{
	public:
		using ValueType = T;
		using SizeType = size_t;

		struct Bucket {
			static inline const SizeType s_unoccupird = std::numeric_limits<SizeType>::max();
			ReusableStorage<ValueType> data;
			SizeType hash = 0;
			SizeType distanceFromIdeal = s_unoccupird;
		}
	};

}