#pragma once
#include <memory>
#include <stdexcept>

#include "InitializerList.h"
#include "PointerContainers.h"
#include "ContainerInterfaces.h"
#include "Span.h"

namespace Containers
{
	template <typename T, size_t s_size, LifetimeManagerType<T> Life = LifetimeManager<T>>
	class Array : public ArrayContainerBase<T, s_size, Life>,
		public IteratorContainerInterface<Array<T, s_size, Life>, T, size_t>,
		public LifetimeManagedContainerInterface<Array<T, s_size, Life>, T, size_t, Life>,
		public SubspanInterface<Array<T, s_size, Life>, T, size_t,
		typename IteratorContainerInterface<Array<T, s_size, Life>, T, size_t>::Iterator,
		typename IteratorContainerInterface<Array<T, s_size, Life>, T, size_t>::ConstIterator>
	{
		friend class IteratorContainerInterface<Array<T, s_size, Life>, T, size_t>;
		friend class LifetimeManagedContainerInterface<Array<T, s_size, Life>, T, size_t, Life>;
		friend class SubspanInterface<Array<T, s_size, Life>, T, size_t,
		typename IteratorContainerInterface<Array<T, s_size, Life>, T, size_t>::Iterator,
		typename IteratorContainerInterface<Array<T, s_size, Life>, T, size_t>::ConstIterator>;
		
	public:
		static_assert(!std::is_reference_v<T>, "Array does not support references");
		static_assert(std::is_destructible_v<T>, "Array requires destructible types");

		static_assert(std::is_default_constructible_v<Life>, "LifetimeManager must be default constructible");
		static_assert(std::is_destructible_v<Life>, "LifetimeManager must be destructible");

		using IteratorBase = IteratorContainerInterface<Array<T, s_size, Life>, T, size_t>;
		using ValueType = T;
		using SizeType = size_t;
		using Iterator = IteratorBase::Iterator;
		using ConstIterator = IteratorBase::ConstIterator;

		constexpr Array()
		{
			this->rangeConstruct(size());
		};

		constexpr ~Array()
		{
			this->rangeDestroy(size());
		};

		constexpr Array(const Array &other) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T>
		{
			this->rangeConstruct(size(), other.data());
		}

		constexpr Array(Array &&other) noexcept(std::is_nothrow_move_assignable_v<T>)
			requires std::is_move_assignable_v<T>
		{
			this->m_lifetimeManager = std::exchange(other.m_lifetimeManager, Life());
			this->rangeMoveConstruct(size(), other.data());
			this->rangeDestroy(other.data(), size());
			other.rangeConstruct(size());
		}

		constexpr Array &operator=(const Array &other) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T>
		{
			if (this != &other)
				this->rangeConstruct(size(), other.data());
			return *this;
		}

		constexpr Array &operator=(Array &&other) noexcept(std::is_nothrow_move_assignable_v<T>)
			requires std::is_move_assignable_v<T>
		{
			if (this != &other)
			{
				this->rangeDestroy(size());
				this->m_lifetimeManager = std::exchange(other.m_lifetimeManager, Life());
				this->rangeMoveConstruct(size(), other.data());
				this->rangeDestroy(other.data(), size());
				other.rangeConstruct(size());
			}
			return *this;
		}

		constexpr Array(InitializerList<T> init) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T>
		{
			CONTAINERS_VERIFY(init.size() > s_size, "Initializer list size exceeds array size");
			this->rangeConstruct(size(), init.begin());
		}

		constexpr Array &operator=(InitializerList<T> init) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T>
		{
			CONTAINERS_VERIFY(init.size() > s_size, "Initializer list size exceeds array size");
			this->rangeConstruct(size(), init.begin());
			return *this;
		}

		static inline constexpr size_t size() { return s_size; };
	};
}