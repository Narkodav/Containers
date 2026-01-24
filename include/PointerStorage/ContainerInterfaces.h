#pragma once
#include <memory>
#include <stdexcept>

#include "InitializerList.h"
#include "PointerContainers.h"

namespace Containers
{
	template<typename Derived, typename V, typename S>
	class IteratorContainerInterface : private CRTPBase<IteratorContainerInterface, Derived, V, S> {
		friend class CRTPBase<IteratorContainerInterface, Derived, V, S>;
	public:
		using ValueType = V;
		using SizeType = S;
		using Iterator = PointerIteratorBase<ValueType>;
		using ConstIterator = PointerIteratorBase<const ValueType>;

		constexpr ValueType& front() { return *this->derived().data(); }
		constexpr const ValueType& front() const { return *this->derived().data(); }
		constexpr ValueType& back() {
			CONTAINERS_VERIFY(this->derived().size() > 0, "Container is empty");
			return this->data()[this->derived().size() - 1];
		}
		constexpr const ValueType& back() const {
			CONTAINERS_VERIFY(this->derived().size() > 0, "Container is empty");
			return this->derived().data()[this->derived().size() - 1];
		}

		constexpr ValueType& at(SizeType index) {
			CONTAINERS_VERIFY(this->derived().size() > index, "Index out of range");
			return this->derived().data()[index];
		}

		constexpr const ValueType& at(SizeType index) const {
			CONTAINERS_VERIFY(this->derived().size() > index, "Index out of range");
			return this->derived().data()[index];
		}

		constexpr ValueType& operator[](SizeType index) { return at(index); }
		constexpr const ValueType& operator[](SizeType index) const { return at(index); }

		constexpr Iterator begin() { return this->derived().data(); }
		constexpr Iterator end() { return this->derived().data() + this->derived().size(); }
		constexpr ConstIterator begin() const { return this->derived().data(); }
		constexpr ConstIterator end() const { return this->derived().data() + this->derived().size(); }
		constexpr ConstIterator cbegin() const { return begin(); }
		constexpr ConstIterator cend() const { return end(); }

		inline Iterator asMutable(ConstIterator it) {
			CONTAINERS_VERIFY(it >= begin() && it <= end(), "Iterator out of range");
			return Iterator(const_cast<ValueType*>(it.data()));
		}
	};

	template <typename Derived, typename V, typename S, typename Life>
	class LifetimeManagedContainerInterface : private CRTPBase<LifetimeManagedContainerInterface, Derived, V, S, Life> {
		friend class CRTPBase<LifetimeManagedContainerInterface, Derived, V, S, Life>;
	public:
		using ValueType = V;
		using SizeType = S;
		using LifetimeManagerType = Life;

	protected:
		[[no_unique_address]] LifetimeManagerType m_lifetimeManager;

		template<typename... Args>
		void construct(SizeType index, Args&&... args) {
			m_lifetimeManager.construct(this->derived().data() + index, std::forward<Args>(args)...);
		}

		void destroy(SizeType index) {
			m_lifetimeManager.construct(this->derived().data() + index);
		}

		template<typename... Args>
		void construct(ValueType* ptr, Args&&... args) {
			m_lifetimeManager.construct(ptr, std::forward<Args>(args)...);
		}

		void destroy(ValueType* ptr) {
			m_lifetimeManager.construct(ptr);
		}

		template<typename... Args>
		void reconstruct(SizeType index, Args&&... args) {
			destroy(index);
			construct(index, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void reconstruct(ValueType* ptr, Args&&... args) {
			destroy(ptr);
			construct(ptr, std::forward<Args>(args)...);
		}

		inline void rangeÑonstruct(ValueType* ptr, size_t count) noexcept {
			m_lifetimeManager.rangeÑonstruct(ptr, count);
		}

		inline void rangeÑonstruct(ValueType* ptr, size_t count, const ValueType* src) noexcept {
			m_lifetimeManager.rangeÑonstruct(ptr, count, src);
		}

		inline void rangeMoveÑonstruct(ValueType* ptr, size_t count, ValueType* src) noexcept {
			m_lifetimeManager.rangeMoveÑonstruct(ptr, count, src);
		}

		inline void rangeDestroy(ValueType* ptr, size_t count) noexcept {
			m_lifetimeManager.rangeDestroy(ptr, count);
		}


		inline void rangeÑonstruct(size_t count) noexcept {
			m_lifetimeManager.rangeÑonstruct(this->derived().data(), count);
		}

		inline void rangeÑonstruct(size_t count, const ValueType* src) noexcept {
			m_lifetimeManager.rangeÑonstruct(this->derived().data(), count, src);
		}

		inline void rangeMoveÑonstruct(size_t count, ValueType* src) noexcept {
			m_lifetimeManager.rangeMoveÑonstruct(this->derived().data(), count, src);
		}

		inline void rangeDestroy(size_t count) noexcept {
			m_lifetimeManager.rangeDestroy(this->derived().data(), count);
		}
	};

	template <typename T, size_t s_capacity, typename Life>
	class ArrayContainerBase
	{
	public:
		using ValueType = T;
		using SizeType = size_t;

	protected:
		alignas(ValueType) std::byte m_data[s_capacity * sizeof(ValueType)];
		static inline const SizeType s_capacity = s_capacity;
	public:
		constexpr ValueType* data() { return reinterpret_cast<ValueType*>(m_data); }
		constexpr const ValueType* data() const { return reinterpret_cast<const ValueType*>(m_data); }
		constexpr static SizeType capacity() { return s_capacity; }
	};

	template <typename V, typename S, typename A>
	class VectorContainerBase
	{
	public:
		using ValueType = V;
		using SizeType = S;
		using AllocatorType = A;

	protected:
		alignas(ValueType) ValueType* m_data;
		size_t m_capacity = 0;
		[[no_unique_address]] AllocatorType m_allocator;

	public:
		constexpr ValueType* data() { return m_data; }
		constexpr const ValueType* data() const { return m_data; }
		constexpr SizeType capacity() const { return m_capacity; }

	protected:
		void allocate(SizeType capacity) {
			m_capacity = capacity;
			m_data = m_allocator.allocate(m_capacity);
		}
		void deallocate() {
			m_allocator.deallocate(m_data, m_capacity);
		}

		void deallocate(ValueType* ptr, SizeType capacity) {
			m_allocator.deallocate(ptr, capacity);
		}

		void reallocate(SizeType capacity) {
			deallocate();
			allocate(capacity);
		}
	};

	// requires Derived to derive from LifetimeManagedContainerInterface 
	// and either VectorContainerInterface or ArrayBuffer and optionally implement grow()
	template <typename Derived, typename V, typename S, typename I, typename CI>
	class SizedContainerInterface : private CRTPBase<SizedContainerInterface, Derived, V, S, I, CI> {
		friend class CRTPBase<SizedContainerInterface, Derived, V, S, I, CI>;
	private:

		template <typename T, typename = void>
		struct HasGrowTrait : std::false_type {};

		template <typename T>
		struct HasGrowTrait<T, std::void_t<decltype(std::declval<T>().grow())>>
			: std::true_type {
		};

		template <typename T, typename = void>
		struct HasParametrisedGrowTrait : std::false_type {};

		template <typename T>
		struct HasParametrisedGrowTrait<T, std::void_t<decltype(std::declval<T>().grow(std::declval<S>()))>>
			: std::true_type {
		};

	public:
		using ValueType = V;
		using SizeType = S;
		using Iterator = I;
		using ConstIterator = CI;

	protected:
		SizeType m_size = 0;

	public:

		SizeType size() const { return m_size; }
		bool empty() const { return m_size == 0; }

		void clear() {
			auto& derived = this->derived();
			derived.rangeDestroy(m_size);
			m_size = 0;
		}

		void resize(SizeType size) {
			auto& derived = this->derived();
			if (size < m_size) {
				for (SizeType i = size; i < m_size; ++i)
					derived.destroy(i);
			}
			else if (size > m_size) {
				if constexpr (HasGrowTrait<Derived>::value)
				{
					if (size > derived.capacity())
						derived.grow(size);
				}
				else {
					CONTAINERS_VERIFY(size <= derived.capacity(), "Cannot grow past capacity");
				}
				for (SizeType i = m_size; i < size; ++i)
					derived.construct(i);
			}
			m_size = size;
		}

		template<typename... Args>
		void pushBack(Args&&... args) requires std::constructible_from<ValueType, Args...> {
			auto& derived = this->derived();
			if constexpr (HasGrowTrait<Derived>::value)
			{
				if (m_size >= derived.capacity())
					derived.grow();
			}
			else {
				CONTAINERS_VERIFY(derived.size() <= derived.capacity(), "Cannot grow past capacity");
			}
			derived.construct(derived.size(), std::forward<Args>(args)...);
			++derived.m_size;
		}

		void popBack() {
			auto& derived = this->derived();
			CONTAINERS_VERIFY(derived.size() > 0, "Popping an empty container");
			derived.destroy(--derived.m_size);
		}

		template<typename... Args>
		Iterator insert(ConstIterator it, Args&&... args) requires std::constructible_from<ValueType, Args...> {
			auto& derived = this->derived();
			auto pos = derived.asMutable(it);
			CONTAINERS_VERIFY(pos <= derived.end() && pos >= derived.begin(), "Index out of range");
			if constexpr (HasGrowTrait<Derived>::value)
			{
				if (derived.size() >= derived.capacity())
					derived.grow();
			}
			else {
				CONTAINERS_VERIFY(derived.size() < derived.capacity(), "Cannot grow past capacity");
			}
			for (Iterator i = derived.end(); i > pos; --i)
			{
				derived.construct(i, std::move(*(i - 1)));
				derived.destroy(i - 1);
			}
			derived.construct(pos, std::forward<Args>(args)...);
			++derived.m_size;
			return pos;
		}

		//template<typename... Args>
		//Iterator insert(SizeType index, Args&&... args) requires std::constructible_from<ValueType, Args...> {
		//	auto& derived = this->derived();
		//	return insert(derived.begin() + index, std::forward<Args>(args)...);
		//}

		Iterator erase(ConstIterator it) {
			auto& derived = this->derived();
			auto pos = derived.asMutable(it);
			CONTAINERS_VERIFY(pos < derived.end() && pos >= derived.begin(), "Index out of range");
			for (; pos < derived.end() - 1; ++pos)
				derived.reconstruct(pos, std::move(*(pos + 1)));
			derived.destroy(pos);
			--derived.m_size;
			return derived.asMutable(it);
		}

		Iterator erase(SizeType index) {
			auto& derived = this->derived();
			return erase(derived.begin() + index);
		}

		Iterator erase(ConstIterator first, ConstIterator last) {
			auto& derived = this->derived();
			CONTAINERS_VERIFY(first < derived.end() && first >= derived.begin(), "Index out of range");
			CONTAINERS_VERIFY(last < derived.end() && last >= derived.begin(), "Index out of range");
			CONTAINERS_VERIFY(first <= last, "Index out of range");
			auto count = last - first;
			Iterator i = derived.asMutable(first);
			for (; i < last; ++i)
				derived.reconstruct(i, std::move(*(i + count)));
			for (; i < derived.end(); ++i)
				derived.destroy(i);
			derived.m_size -= count;
			return derived.asMutable(first);
		}

		Iterator erase(ConstIterator first, SizeType count) {
			auto& derived = this->derived();
			CONTAINERS_VERIFY(first < derived.end() && first >= derived.begin(), "Index out of range");
			CONTAINERS_VERIFY(count < m_size, "Index out of range");
			Iterator i = derived.asMutable(first);
			for (; i < derived.end() - count; ++i)
				derived.reconstruct(i, std::move(*(i + count)));
			for (; i < derived.end(); ++i)
				derived.destroy(i);
			derived.m_size -= count;
			return derived.asMutable(first);
		}

		Iterator insert(ConstIterator it, SizeType count, const ValueType& data) {
			auto& derived = this->derived();
			CONTAINERS_VERIFY(it <= derived.end() && it >= derived.begin(), "Index out of range");
			if constexpr (HasParametrisedGrowTrait<Derived>::value)
			{
				auto required = derived.size() + count;
				if(required > derived.capacity())
					derived.grow(required);
			}
			else {
				CONTAINERS_VERIFY(derived.size() + count <= derived.capacity(), "Cannot grow past capacity");
			}
			for (Iterator i = derived.end() - 1; i >= it; --i)
			{
				derived.construct(i + count, std::move(*i));
				derived.destroy(i);
				derived.construct(i, data);
			}
			derived.m_size += count;
			return derived.asMutable(it);
		}

		//Iterator insert(SizeType index, SizeType count, const ValueType& data) {
		//	auto& derived = this->derived();
		//	return insert(derived.begin() + index, count, data);
		//}

		template<ContiguousIteratorType OtherIterator>
		Iterator insert(ConstIterator it, OtherIterator first, OtherIterator last) {
			auto& derived = this->derived();
			CONTAINERS_VERIFY(it <= derived.end() && it >= derived.begin(), "Index out of range");
			size_t count = last - first;
			if constexpr (HasParametrisedGrowTrait<Derived>::value)
			{
				auto required = derived.size() + count;
				if (required > derived.capacity())
					derived.grow(required);
			}
			else {
				CONTAINERS_VERIFY(derived.size() + count <= derived.capacity(), "Cannot grow past capacity");
			}
			for (Iterator i = derived.end() - 1; i >= it; --i, --last)
			{
				derived.construct(i + count, std::move(*i));
				derived.destroy(i);
				derived.construct(i, *last);
			}
			derived.m_size += count;
			return derived.asMutable(it);
		}

		//template<ContiguousIteratorType OtherIterator>
		//Iterator insert(SizeType index, OtherIterator first, OtherIterator last) {
		//	auto& derived = this->derived();
		//	return insert(derived.begin() + index, first, last);
		//}

		//template<ContiguousIteratorType OtherIterator>
		//Iterator insert(ConstIterator it, OtherIterator first, SizeType count) {
		//	return insert(it, first, first + count);
		//}

		//template<ContiguousIteratorType OtherIterator>
		//Iterator insert(SizeType index, OtherIterator first, SizeType count) {
		//	auto& derived = this->derived();
		//	return insert(derived.begin() + index, first, first + count);
		//}

		Iterator insert(ConstIterator it, InitializerList<ValueType> list) {
			return insert(it, list.begin(), list.end());
		}

		//Iterator insert(SizeType index, InitializerList<ValueType> list) {
		//	auto& derived = this->derived();
		//	return insert(derived.begin() + index, list.begin(), list.end());
		//}

	protected:

		inline void moveFrom(ValueType* from) {
			auto& derived = this->derived();
			for (SizeType i = 0; i < m_size; i++)
			{
				derived.construct(i, std::move(from[i]));
				derived.destroy(from + i);
			}
		}
	};
}

