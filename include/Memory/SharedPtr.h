#pragma once
#pragma once
#include "../Utilities/ByteArray.h"
#include "../Utilities/Concepts.h"
#include "../Utilities/Macros.h"

#include <memory>
#include <stdexcept>
#include <initializer_list>
#include <vector>
#include <utility>
#include <atomic>

namespace Memory {

	class Counter {
	private:
		size_t m_strongCount = 1;
		size_t m_weakCount = 1;
	public:
		inline void incStrong() noexcept {
			++m_strongCount;
		}
		inline bool decStrong() noexcept {
			return --m_strongCount == 0;
		}
		inline void incWeak() noexcept {
			++m_weakCount;
		}
		inline bool decWeak() noexcept {
			return --m_weakCount == 0;
		}
		inline size_t strongCount() const noexcept {
			return m_strongCount;
		}
		inline size_t weakCount() const noexcept {
			return m_weakCount;
		}

		bool strongIncrementIfNotZero() noexcept {
			if (m_strongCount == 0) {
				return false;
			}
			++m_strongCount;
			return true;
		}
	};

	class AtomicCounter {
	private:
		std::atomic<size_t> m_strongCount = 1;
		std::atomic<size_t> m_weakCount = 0;
	public:
		inline void incStrong() noexcept {
			++m_strongCount;
		}
		inline bool decStrong() noexcept {
			return m_strongCount.fetch_sub(1);
		}
		inline void incWeak() noexcept {
			++m_weakCount;
		}
		inline bool decWeak() noexcept {
			return m_weakCount.fetch_sub(1);
		}
		inline size_t strongCount() const noexcept {
			return m_strongCount;
		}
		inline size_t weakCount() const noexcept {
			return m_weakCount;
		}

		bool strongIncrementIfNotZero() noexcept {
			size_t current = m_strongCount.load();
			while (current != 0) {
				if (m_strongCount.compare_exchange_weak(current, current + 1)) {
					return true;
				}
			}
			return false;
		}
	};

	template<typename T, typename Alloc, typename Life, typename Count>
	class ControlBlock {
	protected:
		Count m_refCount;
		[[no_unique_address]] Alloc m_alloc;
		[[no_unique_address]] Life m_lifetimeManager;

	public:
		ControlBlock(Alloc&& alloc, Life&& lifetimeManager) noexcept(
			std::is_nothrow_move_constructible_v<Alloc>&& std::is_nothrow_move_constructible_v<Life>)
			requires std::is_move_constructible_v<Alloc> && std::is_move_constructible_v<Life>
			: m_alloc(std::move(alloc)), m_lifetimeManager(std::move(lifetimeManager)) {
		}

		ControlBlock(const ControlBlock&) noexcept = delete;
		ControlBlock& operator=(const ControlBlock&) noexcept = delete;

		ControlBlock(ControlBlock&&) noexcept = delete;
		ControlBlock& operator=(ControlBlock&&) noexcept = delete;

		~ControlBlock() = default;

		inline Alloc&& releaseAllocator() noexcept {
			return std::move(m_alloc);
		}

		inline Counter& getCounter() noexcept {
			return m_refCount;
		}

		inline const Counter& getCounter() const noexcept {
			return m_refCount;
		}

		inline Alloc& getAllocator() noexcept {
			return m_alloc;
		}

		inline Life& getLifetimeManager() noexcept {
			return m_lifetimeManager;
		}

		inline void destroy() noexcept {
			m_lifetimeManager.destroy(reinterpret_cast<T*>(
				reinterpret_cast<uintptr_t>(this) + sizeof(ControlBlock)));
		}

		inline Alloc dealloc() noexcept {
			Alloc alloc = std::move(m_alloc);
			this->~ControlBlock();
			alloc.deallocate(reinterpret_cast<void*>(this),
				sizeof(ControlBlock) + sizeof(T), alignof(T));
			return alloc;
		}
	};

	template<typename T, typename Alloc, typename Life, typename Count>
	class ControlBlock<T[], Alloc, Life, Count> : public ControlBlock<T, Alloc, Life, Count> {
		using Base = ControlBlock<T, Alloc, Life, Count>;
	private:
		size_t m_size = 0;

	public:
		using Base::Base;

		ControlBlock(Alloc&&, Life&&) = delete;

		ControlBlock(Alloc&& alloc, Life&& lifetimeManager, size_t size) noexcept(
			std::is_nothrow_move_constructible_v<Alloc>&& std::is_nothrow_move_constructible_v<Life>)
			requires std::is_move_constructible_v<Alloc>&& std::is_move_constructible_v<Life>
		: Base(std::move(alloc), std::move(lifetimeManager)), m_size(size) {
		}

		inline void destroy() noexcept {
			std::remove_all_extents_t<T>* data = reinterpret_cast<std::remove_all_extents_t<T>*>(
				reinterpret_cast<uintptr_t>(this) + sizeof(ControlBlock));
			for (size_t i = 0; i < m_size; ++i)
				this->m_lifetimeManager.destroy(data + i);
		}

		inline Alloc dealloc() noexcept {
			Alloc alloc = std::move(this->m_alloc);
			size_t size = m_size;
			this->~ControlBlock();
			alloc.deallocate(reinterpret_cast<void*>(this),
				sizeof(ControlBlock) + sizeof(std::remove_all_extents_t<T>) * size, 
				alignof(std::remove_all_extents_t<T>));
			return alloc;
		}
	};

	template<typename T,
		typename Alloc = Containers::RawAllocator,
		typename Life = Containers::LifetimeManager<std::remove_all_extents_t<T>>,
		typename Count = Counter>
	class SharedPtr
	{
		using ControlBlockType = ControlBlock<T, Alloc, Life, Count>;
		using Type = std::remove_all_extents_t<T>;
		using Pointer = std::remove_all_extents_t<T>*;
	private:
		Pointer m_storage = nullptr;

	public:

		SharedPtr() = default;
		SharedPtr(std::nullptr_t) : m_storage(nullptr) {};

		SharedPtr(SharedPtr&&) noexcept = default;
		SharedPtr& operator=(SharedPtr&&) noexcept = default;

		template<typename... Args>
		SharedPtr(Alloc&& alloc, Life&& life, Args... args) requires (!std::is_array_v<T>) {
			Alloc& allocator = alloc;
			void* mem = allocator.allocate(sizeof(ControlBlockType) + sizeof(Type), alignof(Type));
			if (!mem) {
				throw std::bad_alloc();
			}
			m_storage = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(mem) + sizeof(ControlBlockType));

			auto* controlBlock = getControlBlock();
			life.construct(m_storage, std::forward<Args>(args)...);
			::new (static_cast<void*>(controlBlock)) ControlBlockType(std::move(alloc), std::move(life));
		}

		SharedPtr(Alloc&& alloc, Life&& life, size_t size) requires std::is_array_v<T> {
			Alloc& allocator = alloc;
			void* mem = allocator.allocate(sizeof(ControlBlockType) + sizeof(Type) * size, alignof(Type));
			if (!mem) {
				throw std::bad_alloc();
			}
			m_storage = reinterpret_cast<Pointer>(reinterpret_cast<uintptr_t>(mem) + sizeof(ControlBlockType));

			auto* controlBlock = getControlBlock();

			for (size_t i = 0; i < size; ++i)
				life.construct(m_storage + i);
			::new (static_cast<void*>(controlBlock)) ControlBlockType(std::move(alloc), std::move(life), size);
		}

		SharedPtr(Pointer storage) noexcept : m_storage(storage) {};

		SharedPtr(const SharedPtr& other) noexcept
			: m_storage(other.m_storage) {
			if (m_storage) getControlBlock()->getCounter().incStrong();
		}

		SharedPtr& operator=(const SharedPtr& other) noexcept {
			if (this != &other) {
				reset();
				m_storage = other.m_storage;
				if (m_storage) getControlBlock()->getCounter().incStrong();
			}
			return *this;
		}

		~SharedPtr() {
			reset();
		}

		void reset() noexcept {
			if (m_storage) {
				destroySelf();
			}
		}

		Pointer get() noexcept {
			return m_storage;
		}

		const Pointer get() const noexcept {
			return m_storage;
		}

		bool operator==(const SharedPtr& other) const noexcept {
			return m_storage == other.m_storage;
		}

		bool operator!=(const SharedPtr& other) const noexcept {
			return m_storage != other.m_storage;
		}

		bool operator==(std::nullptr_t) const noexcept {
			return m_storage == nullptr;
		}

		bool operator!=(std::nullptr_t) const noexcept {
			return m_storage != nullptr;
		}

		bool operator==(const Pointer other) const noexcept {
			return m_storage == other;
		}

		bool operator!=(const Pointer other) const noexcept {
			return m_storage != other;
		}

		Type& operator[](size_t index) noexcept requires std::is_array_v<T> {
			CONTAINERS_VERIFY(m_storage != nullptr, "Dereferencing null UniquePtr");
			return m_storage[index];
		}
		const Type& operator[](size_t index) const noexcept requires std::is_array_v<T> {
			CONTAINERS_VERIFY(m_storage != nullptr, "Dereferencing null UniquePtr");
			return m_storage[index];
		}

		size_t useCount() const noexcept {
			if (m_storage) return getControlBlock()->getCounter().strongCount();
			return 0;
		}

		size_t refCount() const noexcept {
			if (m_storage) return getControlBlock()->getCounter().weakCount();
			return 0;
		}

	private:
		ControlBlockType* getControlBlock() noexcept {
			CONTAINERS_VERIFY(m_storage != nullptr, "Dereferencing null SharedPtr");
			return reinterpret_cast<ControlBlockType*>(reinterpret_cast<uintptr_t>(m_storage)
				- sizeof(ControlBlockType));
		}

		const ControlBlockType* getControlBlock() const noexcept {
			CONTAINERS_VERIFY(m_storage != nullptr, "Dereferencing null SharedPtr");
			return reinterpret_cast<ControlBlockType*>(reinterpret_cast<uintptr_t>(m_storage)
				- sizeof(ControlBlockType));
		}

		void destroySelf() noexcept {
			auto* controlBlock = getControlBlock();
			auto& counter = controlBlock->getCounter();
			if (counter.decStrong()) {
				controlBlock->destroy();
				if (counter.decWeak()) controlBlock->dealloc();
			}
			m_storage = nullptr;
		}
	};

	template<typename T,
		typename Alloc = Containers::RawAllocator,
		typename Life = Containers::LifetimeManager<T>,
		typename Count = Counter,
		typename... Args>
	SharedPtr<T, Alloc, Life, Count> makeShared(Args... args)
		noexcept(std::is_nothrow_default_constructible_v<T>&&
			std::is_nothrow_default_constructible_v<Alloc>&&
			std::is_nothrow_default_constructible_v<Life>)
		requires std::is_default_constructible_v<T>&&
			std::is_default_constructible_v<Alloc> &&
			std::is_default_constructible_v<Life> &&
			(!std::is_array_v<T>)
	{
		return SharedPtr<T, Alloc, Life, Count>(std::move(Alloc()),
			std::move(Life()), std::forward<Args>(args)...);
	}

	template<typename T,
		typename Alloc = Containers::RawAllocator,
		typename Life = Containers::LifetimeManager<std::remove_all_extents_t<T>>,
		typename Count = Counter,
		typename... Args>
	SharedPtr<T, Alloc, Life, Count> makeShared(size_t size)
		noexcept(std::is_nothrow_default_constructible_v<std::remove_all_extents_t<T>>&&
			std::is_nothrow_default_constructible_v<Alloc>&&
			std::is_nothrow_default_constructible_v<Life>)
		requires std::is_default_constructible_v<std::remove_all_extents_t<T>>&&
			std::is_default_constructible_v<Alloc>&&
			std::is_default_constructible_v<Life>&&
			std::is_array_v<T>
	{
		return SharedPtr<T, Alloc, Life, Count>(std::move(Alloc()), std::move(Life()), size);
	}

	template<typename T,
		typename Alloc = Containers::RawAllocator,
		typename Life = Containers::LifetimeManager<std::remove_all_extents_t<T>>>
	using AtomicSharedPtr = SharedPtr<T, Alloc, Life, AtomicCounter>;

	template<typename T,
		typename Alloc = Containers::RawAllocator,
		typename Life = Containers::LifetimeManager<T>,
		typename... Args>
	AtomicSharedPtr<T, Alloc, Life> makeSharedAtomic(Args... args)
		noexcept(std::is_nothrow_default_constructible_v<T>&&
			std::is_nothrow_default_constructible_v<Alloc>&&
			std::is_nothrow_default_constructible_v<Life>)
		requires std::is_default_constructible_v<T>&&
	std::is_default_constructible_v<Alloc>&&
		std::is_default_constructible_v<Life> &&
		(!std::is_array_v<T>)
	{
		return AtomicSharedPtr<T, Alloc, Life>(std::move(Alloc()),
			std::move(Life()), std::forward<Args>(args)...);
	}

	template<typename T,
		typename Alloc = Containers::RawAllocator,
		typename Life = Containers::LifetimeManager<std::remove_all_extents_t<T>>,
		typename... Args>
	AtomicSharedPtr<T, Alloc, Life> makeSharedAtomic(size_t size)
		noexcept(std::is_nothrow_default_constructible_v<std::remove_all_extents_t<T>>&&
			std::is_nothrow_default_constructible_v<Alloc>&&
			std::is_nothrow_default_constructible_v<Life>)
		requires std::is_default_constructible_v<std::remove_all_extents_t<T>>&&
	std::is_default_constructible_v<Alloc>&&
		std::is_default_constructible_v<Life>&&
		std::is_array_v<T>
	{
		return AtomicSharedPtr<T, Alloc, Life>(std::move(Alloc()), std::move(Life()), size);
	}
};

