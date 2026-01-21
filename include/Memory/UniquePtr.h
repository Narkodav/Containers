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

namespace Memory {

	template<typename T, typename Deleter = Containers::DefaultDeleter<T>>
	class UniquePtr
	{
	private:
		T* m_ptr = nullptr;
		[[no_unique_address]] Deleter m_deleter;

	public:

		UniquePtr() noexcept = default;
		UniquePtr(std::nullptr_t) noexcept : m_ptr(nullptr) {};

		UniquePtr(T* ptr) noexcept(std::is_nothrow_default_constructible_v<Deleter>)
			requires std::is_default_constructible_v<Deleter>
		: m_ptr(ptr), m_deleter(Deleter()) {
		};
		UniquePtr(T* ptr, Deleter&& deleter) noexcept(std::is_nothrow_move_constructible_v<Deleter>)
			requires std::is_move_constructible_v<Deleter>
		: m_ptr(ptr), m_deleter(std::move(deleter)) {
		};

		~UniquePtr() {
			reset();
		}
		UniquePtr(const UniquePtr&) = delete;
		UniquePtr& operator=(const UniquePtr&) = delete;

		UniquePtr(UniquePtr&& other) noexcept(std::is_nothrow_move_constructible_v<Deleter>)
			requires std::is_move_constructible_v<Deleter>
		: m_ptr(other.m_ptr), m_deleter(std::move(other.m_deleter)) {
			other.m_ptr = nullptr;
		}

		UniquePtr& operator=(UniquePtr&& other) noexcept(std::is_nothrow_move_assignable_v<Deleter>)
			requires std::is_move_assignable_v<Deleter>
		{
			if (this != &other) {
				reset();
				m_ptr = other.m_ptr;
				m_deleter = std::move(other.m_deleter);
				other.m_ptr = nullptr;
			}
			return *this;
		}

		void reset() noexcept {
			if (m_ptr) {
				m_deleter(m_ptr);
				m_ptr = nullptr;
			}
		}

		T* release() noexcept {
			T* temp = m_ptr;
			m_ptr = nullptr;
			return temp;
		}

		Deleter releaseDeleter() noexcept {
			auto temp = std::move(m_deleter);
			m_deleter = Deleter();
			return std::move(temp);
		}

		void assign(T* ptr) noexcept {
			reset();
			m_ptr = ptr;
		}

		void assign(T* ptr, Deleter&& deleter)
			noexcept(std::is_nothrow_move_constructible_v<Deleter>)
			requires std::is_move_constructible_v<Deleter> {
			reset();
			m_ptr = ptr;
			m_deleter = std::move(deleter);
		}

		void assign(Deleter&& deleter)
			noexcept(std::is_nothrow_move_constructible_v<Deleter>)
			requires std::is_move_constructible_v<Deleter> {
			m_deleter = std::move(deleter);
		}

		T& operator*() noexcept {
			CONTAINERS_VERIFY(m_ptr != nullptr, "Dereferencing null UniquePtr");
			return *m_ptr;
		}

		const T& operator*() const noexcept {
			CONTAINERS_VERIFY(m_ptr != nullptr, "Dereferencing null UniquePtr");
			return *m_ptr;
		}

		T* operator->() noexcept {
			CONTAINERS_VERIFY(m_ptr != nullptr, "Dereferencing null UniquePtr");
			return m_ptr;
		}
		const T* operator->() const noexcept {
			CONTAINERS_VERIFY(m_ptr != nullptr, "Dereferencing null UniquePtr");
			return m_ptr;
		}
		T* get() noexcept {
			return m_ptr;
		}
		const T* get() const noexcept {
			return m_ptr;
		}
		explicit operator bool() const noexcept {
			return m_ptr != nullptr;
		}

		Deleter& getDeleter() noexcept {
			return m_deleter;
		}
		const Deleter& getDeleter() const noexcept {
			return m_deleter;
		}

		void swap(UniquePtr& other) noexcept {
			std::swap(m_ptr, other.m_ptr);
			std::swap(m_deleter, other.m_deleter);
		}

		bool operator==(const UniquePtr& other) const noexcept {
			return m_ptr == other.m_ptr;
		}
		bool operator!=(const UniquePtr& other) const noexcept {
			return m_ptr != other.m_ptr;
		}
		bool operator==(std::nullptr_t) const noexcept {
			return m_ptr == nullptr;
		}
		bool operator!=(std::nullptr_t) const noexcept {
			return m_ptr != nullptr;
		}
		bool operator==(const T* other) const noexcept {
			return m_ptr == other;
		}
		bool operator!=(const T* other) const noexcept {
			return m_ptr != other;
		}
	};

	template<typename T, typename Deleter>
	class UniquePtr<T[], Deleter> : public UniquePtr<T, Deleter>
	{
	public:
		using Base = UniquePtr<T, Deleter>;
		using Base::Base;

		T& operator[](size_t index) noexcept {
			CONTAINERS_VERIFY(this->get() != nullptr, "Dereferencing null UniquePtr");
			return this->get()[index];
		}
		const T& operator[](size_t index) const noexcept {
			CONTAINERS_VERIFY(this->get() != nullptr, "Dereferencing null UniquePtr");
			return this->get()[index];
		}
	};

	template<typename T, typename Deleter = Containers::DefaultDeleter<T>,
		typename... Args>
	UniquePtr<T, Deleter> makeUnique(Args... args)
		noexcept(std::is_nothrow_default_constructible_v<T>&&
			std::is_nothrow_default_constructible_v<Deleter>)
		requires std::is_default_constructible_v<T>&&
	std::is_default_constructible_v<Deleter> &&
		(!std::is_array_v<T>)
	{
		return UniquePtr<T, Deleter>(new T(std::forward<Args>(args)...), Deleter());
	}

	template<typename T, typename Deleter = Containers::DefaultDeleter<T>,
		typename... Args>
	UniquePtr<T, Deleter> makeUnique(Deleter&& deleter, Args... args)
		noexcept(std::is_nothrow_constructible_v<T>&&
			std::is_nothrow_move_constructible_v<Deleter>)
		requires std::is_constructible_v<T>&&
	std::is_move_constructible_v<Deleter> &&
		(!std::is_array_v<T>)
	{
		return UniquePtr<T, Deleter>(new T(std::forward<Args>(args)...), std::move(deleter));
	}

	template<typename T, typename Deleter = Containers::DefaultDeleter<T>>
	UniquePtr<T, Deleter> makeUnique(size_t size)
		noexcept(std::is_nothrow_default_constructible_v<std::remove_all_extents_t<T>>&&
			std::is_nothrow_default_constructible_v<Deleter>)
		requires std::is_default_constructible_v<std::remove_all_extents_t<T>>&&
	std::is_default_constructible_v<Deleter> &&
		std::is_array_v<T>
	{
		return UniquePtr<T, Deleter>(new std::remove_all_extents_t<T>[size], Deleter());
	}
};

