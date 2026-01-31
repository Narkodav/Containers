#pragma once
#include "../Utilities/ByteArray.h"
#include "../Utilities/Concepts.h"
#include "../Utilities/Macros.h"

#include <memory>
#include <stdexcept>
#include <initializer_list>
#include <vector>

#include "InitializerList.h"
#include "ContainerInterfaces.h"
#include "Span.h"
#include "Vector.h"
#include "../Memory/UniquePtr.h"

namespace Containers
{

	template <typename T, TypedAllocatorType<T> Alloc = TypedAllocator<T>, LifetimeManagerType<T> Life = LifetimeManager<T>>
	class ManualVector : public VectorContainerBase<T, size_t, Alloc>,
						 public IteratorContainerInterface<ManualVector<T, Alloc, Life>, T, size_t>,
						 public LifetimeManagedContainerInterface<ManualVector<T, Alloc, Life>, T, size_t, Life>,
						 public SizedContainerInterface<ManualVector<T, Alloc, Life>, T, size_t,
														typename IteratorContainerInterface<ManualVector<T, Alloc, Life>, T, size_t>::Iterator,
														typename IteratorContainerInterface<ManualVector<T, Alloc, Life>, T, size_t>::ConstIterator>,
						 public SubspanInterface<ManualVector<T, Alloc, Life>, T, size_t,
												 typename IteratorContainerInterface<ManualVector<T, Alloc, Life>, T, size_t>::Iterator,
												 typename IteratorContainerInterface<ManualVector<T, Alloc, Life>, T, size_t>::ConstIterator>
	{
		friend class SizedContainerInterface<ManualVector<T, Alloc, Life>, T, size_t,
											 typename IteratorContainerInterface<ManualVector<T, Alloc, Life>, T, size_t>::Iterator,
											 typename IteratorContainerInterface<ManualVector<T, Alloc, Life>, T, size_t>::ConstIterator>;
		friend class LifetimeManagedContainerInterface<ManualVector<T, Alloc, Life>, T, size_t, Life>;
		friend class VectorContainerBase<T, size_t, Alloc>;

		static_assert(!std::is_reference_v<T>, "Vector does not support references");
		static_assert(std::is_destructible_v<T>, "Vector requires destructible types");

		static_assert(std::is_default_constructible_v<Alloc>, "Allocator must be default constructible");
		static_assert(std::is_destructible_v<Alloc>, "Allocator must be destructible");

		static_assert(std::is_default_constructible_v<Life>, "LifetimeManager must be default constructible");
		static_assert(std::is_destructible_v<Life>, "LifetimeManager must be destructible");

	public:
		using IteratorBase = IteratorContainerInterface<ManualVector<T, Alloc, Life>, T, size_t>;
		using ValueType = T;
		using SizeType = size_t;
		using LifetimeManagerType = Life;
		using AllocatorType = Alloc;
		using Iterator = typename IteratorBase::Iterator;
		using ConstIterator = typename IteratorBase::ConstIterator;

		using ReleasePtr = Memory::UniquePtr<T[], ReleaseDeleter<T, Alloc, Life>>;

		ManualVector() { this->allocate(0); };

		ManualVector(SizeType capacity) { this->allocate(capacity); };

		ManualVector(SizeType capacity, SizeType size)
		{
			CONTAINERS_VERIFY(capacity >= size, "Capacity must be greater then size");
			this->allocate(capacity);
			this->m_size = size;
			this->rangeConstruct(this->size());
		};

		ManualVector(SizeType capacity, SizeType size, const T &value)
		{
			CONTAINERS_VERIFY(capacity >= size, "Capacity must be greater then size");
			this->allocate(capacity);
			this->m_size = size;
			for (SizeType i = 0; i < this->size(); ++i)
				this->construct(i, value);
		};

		~ManualVector()
		{
			this->clear();
			this->deallocate();
		}

		ManualVector(SizeType capacity, InitializerList<T> list)
		{
			CONTAINERS_VERIFY(capacity >= list.size(), "Capacity must be greater then list size");
			this->allocate(capacity);
			this->m_size = list.size();
			this->rangeConstruct(this->size(), list.begin());
		}

		ManualVector &operator=(InitializerList<T> list)
		{
			CONTAINERS_VERIFY(this->capacity() >= list.size(), "Capacity must be greater then list size");
			this->clear();
			this->m_size = list.size();
			this->rangeConstruct(this->size(), list.begin());
			return *this;
		}

		ManualVector(const ManualVector &other)
		{
			this->allocate(other.capacity());
			this->m_size = other.m_size;
			this->rangeConstruct(this->size(), other.data());
		}

		ManualVector(ManualVector &&other)
		{
			this->m_allocator = std::exchange(other.m_allocator, AllocatorType());
			this->m_lifetimeManager = std::exchange(other.m_lifetimeManager, LifetimeManagerType());
			this->m_capacity = other.m_capacity;
			this->m_size = std::exchange(other.m_size, 0);
			this->m_data = other.m_data;
			other.allocate(0);
		}

		ManualVector &operator=(const ManualVector &other)
		{
			if (this == &other)
				return *this;
			CONTAINERS_VERIFY(this->capacity() >= other.size(), "Capacity must be greater then list size");
			this->clear();
			this->m_size = other.m_size;
			this->rangeConstruct(this->size(), other.data());
			return *this;
		}

		ManualVector &operator=(ManualVector &&other)
		{
			if (this == &other)
				return *this;
			this->clear();
			this->deallocate();

			this->m_allocator = std::exchange(other.m_allocator, AllocatorType());
			this->m_lifetimeManager = std::exchange(other.m_lifetimeManager, LifetimeManagerType());
			this->m_capacity = other.m_capacity;
			this->m_size = std::exchange(other.m_size, 0);
			this->m_data = other.m_data;
			other.allocate(0);
			return *this;
		}

		ReleasePtr release()
		{
			ReleasePtr data;
			typename ReleasePtr::Deleter deleter;
			deleter.m_allocator = std::exchange(this->m_allocator, Alloc());
			deleter.m_lifetimeManager = std::exchange(this->m_lifetimeManager, Life());
			deleter.m_size = std::exchange(this->m_size, 0);
			deleter.m_capacity = std::exchange(this->m_capacity, 0);
			data = ReleasePtr(this->m_data, std::move(deleter));
			this->allocate(0);
			return data;
		}

		void reserve(SizeType capacity)
			requires(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>)
		{
			if (capacity == this->capacity())
				return;
			if (capacity < this->size())
				capacity = this->size();
			migrate(capacity);
		};

		void shrinkToFit()
		{
			if (this->size() == this->capacity())
				return;
			migrate(this->size());
		}

		void swap(ManualVector &other) noexcept
		{
			std::swap(this->m_data, other.m_data);
			std::swap(this->m_size, other.m_size);
			std::swap(this->m_capacity, other.m_capacity);
			std::swap(this->m_allocator, other.m_allocator);
			std::swap(this->m_lifetimeManager, other.m_lifetimeManager);
		}

		bool operator==(const ManualVector &other) const
		{
			if (this->size() != other.m_size)
				return false;
			for (SizeType i = 0; i < this->size(); i++)
				if (this->at(i) != other[i])
					return false;
			return true;
		}

	protected:
		inline void migrate(SizeType newCapacity)
		{
			auto oldData = this->data();
			auto oldCapacity = this->capacity();
			this->allocate(newCapacity);
			this->moveFrom(oldData);
			this->deallocate(oldData, oldCapacity);
		}
	};
}