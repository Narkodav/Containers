#pragma once
#include "Utilities/UnionStorage.h"
#include <memory>
#include <stdexcept>

namespace Containers {
	
	template <typename T, size_t s_size>
		requires std::is_default_constructible_v<T>
	class Array
	{
	public:
		using Iterator = T*;
		using ConstIterator = const T*;
		using ValueType = typename std::remove_reference_t<T>;
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = size_t;
	private:
		T m_data[s_size];

	public:
		constexpr Array() = default;
		constexpr ~Array() = default;

		constexpr Array(const Array& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T>
		{
			for (size_t i = 0; i < s_size; ++i)
				m_data[i] = other.m_data[i];
		}

		constexpr Array(Array&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
			requires std::is_move_assignable_v<T>
		{
			for (size_t i = 0; i < s_size; ++i)
				m_data[i] = std::move(other.m_data[i]);
		}

		constexpr Array& operator=(const Array& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T>
		{
			if (this != &other)
			{
				for (size_t i = 0; i < s_size; ++i)
					m_data[i] = other.m_data[i];
			}
			return *this;
		}

		constexpr Array& operator=(Array&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
			requires std::is_move_assignable_v<T>
		{
			if (this != &other)
			{
				for (size_t i = 0; i < s_size; ++i)
					m_data[i] = std::move(other.m_data[i]);
			}
			return *this;
		}

		constexpr Array(const T(&init)[s_size]) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			for (size_t i = 0; i < s_size; ++i) {
				m_data[i] = init[i];
			}
		}

		template<typename... Args>
		constexpr Array(Args&&... args)
			: m_data{ std::forward<Args>(args)... } // This will fail to compile if too many arguments
		{
			static_assert(sizeof...(Args) <= s_size, "Too many initializers for Array");
		}

		constexpr Array& operator=(const T(&init)[s_size]) noexcept(std::is_nothrow_copy_assignable_v<T>) {
			for (size_t i = 0; i < s_size; ++i) {
				m_data[i] = init[i];
			}
			return *this;
		}

		template<typename... Args>
		constexpr Array& operator=(Args&&... args)
		{
			static_assert(sizeof...(Args) <= s_size, "Too many initializers for Array");
			m_data = { std::forward<Args>(args)... };
			return *this;
		}

		constexpr Array(std::initializer_list<T> init) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T> {
			if (init.size() > s_size) {
				throw std::out_of_range("Initializer list size exceeds array size");
			}
			auto it = init.begin();
			for (size_t i = 0; i < init.size(); ++i) {
				m_data[i] = *it++;
			}
		}

		constexpr Array& operator=(std::initializer_list<T> init) noexcept(std::is_nothrow_copy_assignable_v<T>)
			requires std::is_copy_assignable_v<T> {
			if (init.size() > s_size) {
				throw std::out_of_range("Initializer list size exceeds array size");
			}
			auto it = init.begin();
			for (size_t i = 0; i < init.size(); ++i) {
				m_data[i] = *it++;
			}
			return *this;
		}

		constexpr T& operator[](size_t index) { return m_data[index]; };
		constexpr const T& operator[](size_t index) const { return m_data[index]; };
		constexpr const size_t size() const { return s_size; };

		constexpr T* data() { return m_data; }
		constexpr const T* data() const { return m_data; }
		constexpr bool empty() const { return s_size == 0; }
		T& at(size_t index) { if (index >= s_size) throw std::out_of_range("..."); return m_data[index]; }
		const T& at(size_t index) const { if (index >= s_size) throw std::out_of_range("..."); return m_data[index]; }
		constexpr T& front() { return m_data[0]; }
		constexpr const T& front() const { return m_data[0]; }
		constexpr T& back() { return m_data[s_size - 1]; }
		constexpr const T& back() const { return m_data[s_size - 1]; }

		constexpr Iterator begin() { return &m_data[0]; }
		constexpr Iterator end() { return &m_data[s_size]; }
		constexpr ConstIterator begin() const { return &m_data[0]; }
		constexpr ConstIterator end() const { return &m_data[s_size]; }
		constexpr ConstIterator cbegin() const { return begin(); }
		constexpr ConstIterator cend() const { return end(); }
	};
}