#pragma once
#include "ContainerInterfaces.h"

namespace Containers
{
	template <typename T, TypedAllocatorType<T> Alloc = TypedAllocator<T>, LifetimeManagerType<T> Life = LifetimeManager<T>>
	class HeapBuffer  : public VectorContainerBase<T, size_t, Alloc>,
		public LifetimeManagedContainerInterface<HeapBuffer<T, Alloc, Life>, T, size_t, Life>
	{
		friend class LifetimeManagedContainerInterface<HeapBuffer<T, Alloc, Life>, T, size_t, Life>;
		friend class VectorContainerBase<T, size_t, Alloc>;
	public:
		using ValueType = T;
		using SizeType = size_t;
		using LifetimeManagerType = Life;
		using AllocatorType = Alloc;

		HeapBuffer(SizeType capacity) {
			this->allocate(capacity);
		};

		~HeapBuffer() {
			this->deallocate();
		};

		void reallocate(SizeType capacity) {
			this->reallocate(capacity);
		}

		template<typename... Args>
		void construct(SizeType index, Args... args) {
			this->construct(index, std::forward<Args>(args)...);
		}

		void destroy(SizeType index) {
			this->destroy(index);
		}

		SizeType size() const { return this->capacity(); }
	};

}