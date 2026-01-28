#pragma once
#include "../Utilities/ByteArray.h"
#include "../Utilities/Concepts.h"
#include "../Utilities/Macros.h"

#include <memory>
#include <stdexcept>
#include <initializer_list>
#include <vector>

#include "InitializerList.h"
#include "Span.h"
#include "../Memory/UniquePtr.h"

namespace Containers
{

	// Fixed-capacity dynamic array, does not allocate memory dynamically
	template <typename T, size_t s_capacity, LifetimeManagerType<T> Life = LifetimeManager<T>>
	class DynamicArray : public ArrayContainerBase<T, s_capacity, Life>,
						 public IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>,
						 public LifetimeManagedContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t, Life>,
						 public SubspanInterface<DynamicArray<T, s_capacity, Life>, T, size_t,
												 typename IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>::Iterator,
												 typename IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>::ConstIterator>,
						 public SizedContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t,
														typename IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>::Iterator,
														typename IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>::ConstIterator>
	{
		friend class IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>;
		friend class LifetimeManagedContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t, Life>;
		friend class SubspanInterface<DynamicArray<T, s_capacity, Life>, T, size_t,
									  typename IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>::Iterator,
									  typename IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>::ConstIterator>;
		friend class SizedContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t,
											 typename IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>::Iterator,
											 typename IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>::ConstIterator>;

		static_assert(!std::is_reference_v<T>, "DynamicArray does not support references");
		static_assert(std::is_destructible_v<T>, "DynamicArray requires destructible types");

		static_assert(std::is_default_constructible_v<Life>, "LifetimeManager must be default constructible");
		static_assert(std::is_destructible_v<Life>, "LifetimeManager must be destructible");

	public:
		using IteratorBase = IteratorContainerInterface<DynamicArray<T, s_capacity, Life>, T, size_t>;

		using ValueType = T;
		using SizeType = size_t;
		using LifetimeManagerType = Life;

		using Iterator = typename IteratorBase::Iterator;
		using ConstIterator = typename IteratorBase::ConstIterator;

	public:
		DynamicArray() = default;

		DynamicArray(SizeType size)
		{
			CONTAINERS_VERIFY(size <= this->capacity());
			this->m_size = size;
			this->rangeConstruct(this->size());
		}

		DynamicArray(SizeType size, const T &value)
		{
			CONTAINERS_VERIFY(size <= this->capacity());
			this->m_size = size;
			for (SizeType i = 0; i < this->size(); ++i)
				this->construct(i, value);
		}

		~DynamicArray() { this->clear(); }

		DynamicArray(InitializerList<T> list)
		{
			CONTAINERS_VERIFY(list.size() <= this->capacity());
			this->m_size = list.size();
			this->rangeConstruct(this->size(), list.begin());
		}

		DynamicArray &operator=(InitializerList<T> list)
		{
			CONTAINERS_VERIFY(list.size() <= this->capacity());
			this->clear();
			this->m_size = list.size();
			this->rangeConstruct(this->size(), list.begin());
			return *this;
		}

		template <SizeType otherCapacity, LifetimeManagerConcept<T> OtherLife>
		DynamicArray(const DynamicArray<ValueType, otherCapacity, OtherLife> &other)
		{
			CONTAINERS_VERIFY(other.size() <= this->capacity());
			this->m_size = other.size();
			this->rangeConstruct(this->size(), other.data());
		}

		template <SizeType otherCapacity, LifetimeManagerConcept<T> OtherLife>
		DynamicArray(DynamicArray<ValueType, otherCapacity, OtherLife> &&other)
		{
			CONTAINERS_VERIFY(other.size() <= this->capacity());
			this->m_size = std::exchange(other.m_size, 0);
			this->m_lifetimeManager = std::exchange(other.m_lifetimeManager, LifetimeManagerType());
			this->rangeMoveConstruct(this->size(), other.data());
			this->rangeDestroy(other.data(), this->size());
		}

		template <SizeType otherCapacity, LifetimeManagerConcept<T> OtherLife>
		DynamicArray &operator=(const DynamicArray<ValueType, otherCapacity, OtherLife> &other)
		{
			if (this == &other)
				return *this;
			CONTAINERS_VERIFY(other.size() <= this->capacity());
			this->m_size = other.size();
			this->rangeConstruct(this->size(), other.data());
			return *this;
		}

		template <SizeType otherCapacity, LifetimeManagerConcept<T> OtherLife>
		DynamicArray &operator=(DynamicArray<ValueType, otherCapacity, OtherLife> &&other)
		{
			if (this == &other)
				return *this;
			this->clear();

			CONTAINERS_VERIFY(other.size() <= this->capacity());
			this->m_size = std::exchange(other.m_size, 0);
			this->m_lifetimeManager = std::exchange(other.m_lifetimeManager, LifetimeManagerType());
			this->rangeMoveConstruct(this->size(), other.data());
			this->rangeDestroy(other.data(), this->size());
			return *this;
		}

		template <SizeType otherCapacity, LifetimeManagerConcept<T> OtherLife>
		bool operator==(const DynamicArray<ValueType, otherCapacity, OtherLife> &other) const
		{
			if (this->size() != other.m_size)
				return false;
			for (SizeType i = 0; i < this->size(); i++)
				if (this->at(i) != other[i])
					return false;
			return true;
		}
	};
}