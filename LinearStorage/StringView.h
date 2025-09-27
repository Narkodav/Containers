#pragma once
#include "String.h"

namespace Containers
{
	template<typename CharType, size_t s_size>
	class ConstStringViewBase
	{
	public:
		static_assert(std::is_integral_v<CharType>, "CharType must be arithmetic type");
		static_assert(!std::is_same_v<CharType, bool>, "CharType cannot be a bool");

		using ValueType = CharType;
		using ConstIterator = const ValueType*;
		using SizeType = size_t;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = SizeType;

	private:
		const CharType m_str[s_size];

	public:
		constexpr ConstStringViewBase(const CharType(&str)[s_size]) {
			m_str = str;
		}
		constexpr ~ConstStringViewBase() = default;

		constexpr ConstStringViewBase(const ConstStringViewBase&) = default;
		constexpr ConstStringViewBase(ConstStringViewBase&&) = default;
		constexpr ConstStringViewBase& operator=(const ConstStringViewBase&) = default;
		constexpr ConstStringViewBase& operator=(ConstStringViewBase&&) = default;

		inline constexpr const CharType* cStr() const { return m_str; };
		inline constexpr size_t size() const { return s_size - 1; }

		inline constexpr const CharType& operator[](size_t index) const { return m_str[index]; }
		inline constexpr const CharType& at(size_t index) const {
			CONTAINERS_VERIFY(index < s_size - 1, "Index out of range");
			return m_str[index];
		}
		inline constexpr operator const CharType* () const { return m_str; }

		inline constexpr const CharType& back() const { return m_str[s_size - 2]; };
		inline constexpr const CharType& front() const { return m_str[0]; };

		inline ConstIterator begin() const { return m_str; }
		inline ConstIterator end() const { return m_str + s_size - 1; }
		inline ConstIterator cbegin() const { return begin(); }
		inline ConstIterator cend() const { return end(); }
	};

	template<typename CharType>
	class StringViewBase {
	public:
		static_assert(std::is_integral_v<CharType>, "CharType must be arithmetic type");
		static_assert(!std::is_same_v<CharType, bool>, "CharType cannot be a bool");

		using ValueType = CharType;
		using Iterator = ValueType*;
		using ConstIterator = const ValueType*;
		using SizeType = size_t;
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = SizeType;

	private:
		const CharType* m_str = nullptr;
		size_t m_size = 0;
	public:

		StringViewBase(const char* str, size_t size) : m_str(str), m_size(size) {};

		template<typename T>
		StringViewBase(T&& str) requires
			!std::is_array_v<std::remove_reference_t<T>>
			: m_str(str), m_size(strlen(str)) {};

		template<AllocatorConcept<CharType> Alloc = TrivialAllocator<CharType>,
			size_t s_initialCapacity = 8, float s_growthFactor = 1.618f>
		StringViewBase(const StringBase<CharType, Alloc, s_initialCapacity, s_growthFactor>& str)
			: m_str(str.data()), m_size(str.size()) {};

		inline constexpr const CharType* cStr() const { return m_str; };
		inline constexpr size_t size() const { return m_size; }

		inline constexpr const CharType& operator[](size_t index) const { return m_str[index]; }
		inline constexpr const CharType& at(size_t index) const {
			CONTAINERS_VERIFY(index < m_size, "Index out of range");
			return m_str[index];
		}
		inline constexpr operator const CharType* () const { return m_str; }

		inline constexpr const CharType& back() const { return m_str[m_size - 1]; };
		inline constexpr const CharType& front() const { return m_str[0]; };

		inline ConstIterator begin() const { return m_str; }
		inline ConstIterator end() const { return m_str + m_size; }
		inline ConstIterator cbegin() const { return begin(); }
		inline ConstIterator cend() const { return end(); }
	};

	using StringView = StringViewBase<char>;
	using StringViewW = StringViewBase<wchar_t>;
	using StringViewU8 = StringViewBase<char8_t>;
	using StringViewU16 = StringViewBase<char16_t>;

	template<size_t s_size>
	using ConstStringView = ConstStringViewBase<char, s_size>;

	template<size_t s_size>
	using ConstStringViewW = ConstStringViewBase<wchar_t, s_size>;

	template<size_t s_size>
	using ConstStringViewU8 = ConstStringViewBase<char8_t, s_size>;

	template<size_t s_size>
	using ConstStringViewU16 = ConstStringViewBase<char16_t, s_size>;
}

