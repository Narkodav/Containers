#pragma once
#include "Utilities/Concepts.h"
#include "Vector.h"
#include "Array.h"
#include "linearContainer.h"

#include <memory>
#include <stdexcept>
#include <vector>
#include <initializer_list>

namespace Containers {

	template<typename T, size_t s_size = 0>
	class Span {
	public:
		using ValueType = T;
		using Iterator = ValueType*;
		using ConstIterator = const ValueType*;
		using SizeType = size_t;
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = SizeType;

	private:
		T* m_data = nullptr;

		template<typename Convertible>
		static inline const bool s_inheritanceConstructorCondition =
			std::is_base_of_v<T, Convertible> || std::is_base_of_v<Convertible, T>;

		template<typename Convertible>
		static inline const bool s_trivialConstructorCondition =
			sizeof(Convertible) == sizeof(T) &&
			std::is_trivially_copyable_v<Convertible> &&
			std::is_trivially_copyable_v<T>;

	public:
		constexpr Span() = default;
		constexpr ~Span() = default;

		constexpr Span(const Span&) = default;
		constexpr Span(Span&&) = default;
		constexpr Span& operator=(const Span&) = default;
		constexpr Span& operator=(Span&&) = default;		

		template<LinearContainerStaticType Container>
		constexpr Span(Container& arr) requires
			std::same_as<T, typename Container::ValueType> &&
			(!s_inheritanceConstructorCondition<typename Container::ValueType> &&
				!s_trivialConstructorCondition<typename Container::ValueType>)
			: m_data(arr.data()) {
		}

		template<LinearContainerStaticType Container>
		Span(Container& arr)
			requires(s_inheritanceConstructorCondition<typename Container::ValueType>)
			: m_data(arr.data()) {
		}

		template<LinearContainerStaticType Container>
		Span(Container& arr)
			requires(!s_inheritanceConstructorCondition<typename Container::ValueType> &&
			s_trivialConstructorCondition<typename Container::ValueType>)
			: m_data(reinterpret_cast<T*>(arr.data())) {
		}
				
		constexpr Span(T* data) : m_data(data) {};

		template<typename ItType>
		constexpr Span(ItType begin) requires std::contiguous_iterator<ItType>
			: m_data(begin) {
		};

		T& operator[](size_t index) { return m_data[index]; };
		const T& operator[](size_t index) const { return m_data[index]; };
		T& at(size_t index) { if (index >= s_size) throw std::out_of_range("..."); return m_data[index]; }
		const T& at(size_t index) const { if (index >= s_size) throw std::out_of_range("..."); return m_data[index]; }
		constexpr const size_t size() const { return s_size; };
		const bool empty() const { return m_data == nullptr; };
		const T* data() const { return m_data; };
		T* data() { return m_data; };

		T& back() { return m_data[s_size - 1]; };
		const T& back() const { return m_data[s_size - 1]; };
		T& front() { return m_data[0]; };
		const T& front() const { return m_data[0]; };

		inline Iterator begin() { return m_data; }
		inline Iterator end() { return m_data + s_size; }
		inline ConstIterator begin() const { return m_data; }
		inline ConstIterator end() const { return m_data + s_size; }
		inline ConstIterator cbegin() const { return begin(); }
		inline ConstIterator cend() const { return end(); }

		void clear() { m_data = nullptr; };
		void assign(T* data) { m_data = data; }

		template<typename ItType>
		void assign(ItType begin) requires std::contiguous_iterator<ItType>
		{
			m_data = begin;
		}

		template<LinearContainerType Container>
		bool operator==(const Container& other) const requires
			requires(const ValueType& a, const typename Container::ValueType& b) {
				{ a == b } -> std::convertible_to<bool>;
		} {
			if (this->size() != other.size()) return false;
			for (size_t i = 0; i < this->size(); ++i) {
				if (!((*this)[i] == other[i])) return false;
			}
			return true;
		}
	};

	template<typename T>
	class Span<T, 0>
	{
	public:
		using ValueType = T;
		using Iterator = ValueType*;
		using ConstIterator = const ValueType*;
		using SizeType = size_t;
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = SizeType;

	private:
		T* m_data = nullptr;
		size_t m_size = 0;

		template<typename Convertible>
		static inline const bool s_inheritanceConstructorCondition =
			std::is_base_of_v<T, Convertible> || std::is_base_of_v<Convertible, T>;

		template<typename Convertible>
		static inline const bool s_trivialConstructorCondition =
			sizeof(Convertible) == sizeof(T) &&
			std::is_trivially_copyable_v<Convertible> &&
			std::is_trivially_copyable_v<T>;

	public:
		Span() = default;
		~Span() = default;

		Span(const Span&) = default;
		Span(Span&&) = default;
		Span& operator=(const Span&) = default;
		Span& operator=(Span&&) = default;		

		template<LinearContainerType Container>
		Span(Container& arr) requires std::same_as<T, typename Container::ValueType>
			&& (!s_inheritanceConstructorCondition<typename Container::ValueType> &&
				!s_trivialConstructorCondition<typename Container::ValueType>)
			: m_data(arr.data()), m_size(arr.size()) {
		};

		template<LinearContainerType Container>
		Span(Container& arr, size_t size) requires std::same_as<T, typename Container::ValueType>
			&& (!s_inheritanceConstructorCondition<typename Container::ValueType> &&
				!s_trivialConstructorCondition<typename Container::ValueType>)
			: m_data(arr.data()), m_size(size) {
		};

		template<LinearContainerType Container>
		Span(Container& vec)
			requires(!s_inheritanceConstructorCondition<typename Container::ValueType>&&
		s_trivialConstructorCondition<typename Container::ValueType>)
			: m_data(reinterpret_cast<T*>(vec.data())), m_size(vec.size()) {
		}

		template<LinearContainerType Container>
		Span(Container& vec, size_t size)
			requires(!s_inheritanceConstructorCondition<typename Container::ValueType>&&
		s_trivialConstructorCondition<typename Container::ValueType>)
			: m_data(reinterpret_cast<T*>(vec.data())), m_size(size) {
		}

		template<typename ItType>
		Span(ItType begin, ItType end) requires std::contiguous_iterator<ItType>
			: m_data(begin), m_size(end - begin) {
		};

		template<size_t size>
		Span(T(&arr)[size]) : m_data(arr), m_size(size) {};
		Span(T* data, size_t size) : m_data(data), m_size(size) {};

		T& operator[](size_t index) { return m_data[index]; };
		const T& operator[](size_t index) const { return m_data[index]; };
		T& at(size_t index) { if (index >= m_size) throw std::out_of_range("..."); return m_data[index]; }
		const T& at(size_t index) const { if (index >= m_size) throw std::out_of_range("..."); return m_data[index]; }
		const size_t size() const { return m_size; };
		const bool empty() const { return m_size == 0; };
		const T* data() const { return m_data; };
		T* data() { return m_data; };

		T& back() { return m_data[m_size - 1]; };
		const T& back() const { return m_data[m_size - 1]; };
		T& front() { return m_data[0]; };
		const T& front() const { return m_data[0]; };

		inline Iterator begin() { return m_data; }
		inline Iterator end() { return m_data + m_size; }
		inline ConstIterator begin() const { return m_data; }
		inline ConstIterator end() const { return m_data + m_size; }
		inline ConstIterator cbegin() const { return begin(); }
		inline ConstIterator cend() const { return end(); }

		void clear() { m_size = 0; };
		void assign(T* data, size_t size) { m_data = data; m_size = size; }

		template<typename ItType>
		void assign(ItType begin, ItType end) requires std::contiguous_iterator<ItType>
		{
			m_data = begin;
			m_size = end - begin;
		};

		template<LinearContainerType Container>
		bool operator==(const Container& other) const requires
			requires(const ValueType& a, const typename Container::ValueType& b) {
				{ a == b } -> std::convertible_to<bool>;
			} {
			if (this->size() != other.size()) return false;
			for (size_t i = 0; i < this->size(); ++i) {
				if (!((*this)[i] == other[i])) return false;
			}
			return true;
		}
	};
}
