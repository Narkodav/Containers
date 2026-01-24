#pragma once
#include "ContainerInterfaces.h"

namespace Containers
{
	template <typename T, TypedAllocatorConcept<T> Alloc = TypedAllocator<T>,
		LifetimeManagerConcept<T> Life = LifetimeManager<T>, std::integral S>
	class HeapBuffer : public PointerContainerBase<HeapBuffer<T, Alloc, Life, S>, T, S>,
		VectorContainerInterface<T, S, Alloc>, 
		LifetimeManagedContainerInterface<HeapBuffer<T, Alloc, Life, S>, T, S, Life>
	{
	public:
		using IteratorBase = PointerContainerBase<HeapBuffer<T, Alloc, Life, S>, T, S>;
		using ValueType = T;
		using SizeType = S;
		using LifetimeManagerType = Life;
		using AllocatorType = Alloc;
		using Iterator = typename IteratorBase::Iterator;
		using ConstIterator = typename IteratorBase::ConstIterator;

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