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
#include "../Memory/UniquePtr.h"

namespace Containers
{

	template <typename T, TypedAllocatorType<T> Alloc, LifetimeManagerType<T> Life>
	struct ReleaseDeleter
	{
		size_t m_size = 0;
		size_t m_capacity = 0;
		[[no_unique_address]] Alloc m_allocator;
		[[no_unique_address]] Life m_lifetimeManager;

		void operator()(T *data) noexcept
		{
			for (size_t i = 0; i < m_size; ++i)
				m_lifetimeManager.destroy(data + i);
			m_size = 0;
			m_capacity = 0;
			m_allocator.deallocate(data, m_capacity);
		};
	};

	template <typename T, TypedAllocatorType<T> Alloc = TypedAllocator<T>,
		LifetimeManagerType<T> Life = LifetimeManager<T>, size_t s_initialCapacity = 16, float s_growthFactor = 1.618f>
	class Vector : public VectorContainerBase<T, size_t, Alloc>,
		public IteratorContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t>,
		public LifetimeManagedContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t, Life>,
		public SizedContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t,
		typename IteratorContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t>::Iterator,
		typename IteratorContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t>::ConstIterator>,
		public SubspanInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t,
		typename IteratorContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t>::Iterator,
		typename IteratorContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t>::ConstIterator>
	{
		friend class SizedContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t,
		typename IteratorContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t>::Iterator,
		typename IteratorContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t>::ConstIterator>;
		friend class LifetimeManagedContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t, Life>;
		friend class VectorContainerBase<T, size_t, Alloc>;

		static_assert(!std::is_reference_v<T>, "Vector does not support references");
		static_assert(std::is_destructible_v<T>, "Vector requires destructible types");

		static_assert(std::is_default_constructible_v<Alloc>, "Allocator must be default constructible");
		static_assert(std::is_destructible_v<Alloc>, "Allocator must be destructible");

		static_assert(std::is_default_constructible_v<Life>, "LifetimeManager must be default constructible");
		static_assert(std::is_destructible_v<Life>, "LifetimeManager must be destructible");

		static_assert(s_growthFactor > 1.0f, "Growth factor must be greater than 1");

	public:
		using IteratorBase = IteratorContainerInterface<Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>, T, size_t>;
		using ValueType = T;
		using SizeType = size_t;
		using LifetimeManagerType = Life;
		using AllocatorType = Alloc;
		using Iterator = typename IteratorBase::Iterator;
		using ConstIterator = typename IteratorBase::ConstIterator;

		using ReleasePtr = Memory::UniquePtr<T[], ReleaseDeleter<T, Alloc, Life>>;

		Vector()
		{
			this->allocate(s_initialCapacity);
		}

		Vector(SizeType size)
		{
			this->allocate((s_initialCapacity + 1) * size);
			this->m_size = size;
			this->rangeConstruct(this->size());
		}

		Vector(SizeType size, const T &value)
		{
			this->allocate((s_initialCapacity + 1) * size);
			this->m_size = size;
			for (SizeType i = 0; i < this->size(); ++i)
				this->construct(i, value);
		}

		~Vector()
		{
			this->clear();
			this->deallocate();
		}

		Vector(InitializerList<T> list)
		{
			this->allocate((s_initialCapacity + 1) * list.size());
			this->m_size = list.size();
			this->rangeConstruct(this->size(), list.begin());
		}

		Vector &operator=(InitializerList<T> list)
		{
			this->clear();
			if (list.size() > this->capacity())
				this->reallocate((s_initialCapacity + 1) * list.size());
			this->m_size = list.size();
			this->rangeConstruct(this->size(), list.begin());
			return *this;
		}

		Vector(const Vector &other)
		{
			this->allocate(other.capacity());
			this->m_size = other.m_size;
			this->rangeConstruct(this->size(), other.data());
		}

		Vector(Vector &&other)
		{
			this->m_allocator = std::exchange(other.m_allocator, AllocatorType());
			this->m_lifetimeManager = std::exchange(other.m_lifetimeManager, LifetimeManagerType());
			this->m_capacity = other.m_capacity;
			this->m_size = std::exchange(other.m_size, 0);
			this->m_data = other.m_data;
			other.allocate(s_initialCapacity);
		}

		Vector &operator=(const Vector &other)
			requires std::is_copy_constructible_v<T>
		{
			if (this == &other)
				return *this;
			this->clear();
			if (other.size() > this->capacity())
				this->reallocate((s_initialCapacity + 1) * other.size());
			this->m_size = other.m_size;
			this->rangeConstruct(this->size(), other.data());
			return *this;
		}

		Vector &operator=(Vector &&other)
			requires(std::is_move_assignable_v<Alloc>) &&
					std::is_move_constructible_v<Life>
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
			other.allocate(s_initialCapacity);
			return *this;
		}

		ReleasePtr release()
		{
			ReleasePtr data;
			typename ReleasePtr::Deleter deleter;
			deleter.m_allocator = std::exchange(this->m_allocator, Alloc());
			deleter.m_lifetimeManager = std::exchange(this->m_lifetimeManager, Life());
			deleter.m_size = std::exchange(this->m_size, 0);
			deleter.m_capacity = std::exchange(this->m_capacity, s_initialCapacity);
			data = ReleasePtr(this->m_data, std::move(deleter));
			this->allocate(s_initialCapacity);
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

		void swap(Vector &other) noexcept
		{
			std::swap(this->m_data, other.m_data);
			std::swap(this->m_size, other.m_size);
			std::swap(this->m_capacity, other.m_capacity);
			std::swap(this->m_allocator, other.m_allocator);
			std::swap(this->m_lifetimeManager, other.m_lifetimeManager);
		}

		bool operator==(const Vector &other) const
		{
			if (this->size() != other.m_size)
				return false;
			for (SizeType i = 0; i < this->size(); i++)
				if (this->at(i) != other[i])
					return false;
			return true;
		}

		void shrink()
		{
			if (this->size() == this->capacity())
				return;
			auto newCapacity = this->capacity() / s_growthFactor;
			if (this->size() > newCapacity)
				newCapacity = this->size();

			migrate(newCapacity);
		}

	protected:
		void grow()
		{
			migrate(s_growthFactor * (this->capacity() + 1));
		}

		void grow(SizeType newCapacity)
		{
			migrate(s_growthFactor * newCapacity);
		};

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