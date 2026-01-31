#pragma once
#pragma once

#include "SharedPtr.h"

namespace Memory {	

	template<typename T,
		typename Alloc = Containers::RawAllocator,
		typename Life = Containers::LifetimeManager<std::remove_all_extents_t<T>>,
		typename Count = Counter>
	class WeakPtr
	{
		using ControlBlockType = ControlBlock<T, Alloc, Life, Count>;
		using Type = std::remove_all_extents_t<T>;
		using Pointer = std::remove_all_extents_t<T>*;
	private:
		Pointer m_storage = nullptr;

	public:

		WeakPtr() = default;

		WeakPtr(WeakPtr&&) noexcept = default;
		WeakPtr& operator=(WeakPtr&&) noexcept = default;

		WeakPtr(const WeakPtr& other) noexcept
			: m_storage(other.m_storage) {
			if (m_storage) getControlBlock()->getCounter().incWeak();
		}

		WeakPtr& operator=(const WeakPtr& other) noexcept {
			if (this != &other) {
				reset();
				m_storage = other.m_storage;
				if (m_storage) getControlBlock()->getCounter().incWeak();
			}
			return *this;
		}

		WeakPtr(SharedPtr<T, Alloc, Life, Count>& other) noexcept
			: m_storage(other.get()) {
			if (m_storage) getControlBlock()->getCounter().incWeak();
		}

		WeakPtr& operator=(SharedPtr<T, Alloc, Life, Count>& other) noexcept {
			reset();
			m_storage = other.get();
			if (m_storage) getControlBlock()->getCounter().incWeak();
			return *this;
		}

		~WeakPtr() {
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

		bool operator==(const WeakPtr& other) const noexcept {
			return m_storage == other.m_storage;
		}

		bool operator!=(const WeakPtr& other) const noexcept {
			return m_storage != other.m_storage;
		}

		bool operator==(const SharedPtr<T, Alloc, Life, Count>& other) const noexcept {
			return m_storage == other.get();
		}

		bool operator!=(const SharedPtr<T, Alloc, Life, Count>& other) const noexcept {
			return m_storage != other.get();
		}

		bool operator==(std::nullptr_t) const noexcept {
			return m_storage == nullptr;
		}

		bool operator!=(std::nullptr_t) const noexcept {
			return m_storage != nullptr;
		}

		Type& operator[](size_t index) noexcept requires std::is_array_v<T> {
			CONTAINERS_VERIFY(m_storage != nullptr, "Dereferencing null UniquePtr");
			return m_storage[index];
		}
		const Type& operator[](size_t index) const noexcept requires std::is_array_v<T> {
			CONTAINERS_VERIFY(m_storage != nullptr, "Dereferencing null UniquePtr");
			return m_storage[index];
		}

		bool expired() const noexcept {
			if (m_storage) {
				return getControlBlock()->getCounter().strongCount() == 0;
			}
			return true;
		}

		SharedPtr<T, Alloc, Life, Count> lock() noexcept {
			if (getControlBlock()->getCounter().strongIncrementIfNotZero())
				return SharedPtr<T, Alloc, Life, Count>(m_storage);
			return nullptr;
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
			if (counter.decWeak()) controlBlock->dealloc();
			m_storage = nullptr;
		}
	};
};

