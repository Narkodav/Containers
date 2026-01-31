#pragma once
#include "Span.h"

namespace Containers
{
	template<typename CharType>
	class StringViewBase : public Span<CharType> {
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

		using Span<CharType, 0>::Span;
		using Span<CharType, 0>::operator=;
		using Span<CharType, 0>::operator==;

		StringViewBase(CharType* str) : Span<CharType, 0>(str, strlen(str)) {};

		inline const CharType* cStr() const { return this->data(); };

		inline const StringViewBase subStr(size_t offset, size_t length) const {
			return this->subSpan(offset, length);
		};

		//template<size_t size>
		//bool operator==(const CharType(&str)[size]) const {
		//	if (this->size() != size) return false;
		//	for (size_t i = 0; i < size; ++i) {
		//		if (!((*this)[i] == str[i])) return false;
		//	}
		//	return true;
		//}		

		bool operator==(const CharType* str) const {
			if (this->size() != Containers::strlen(str)) return false;
			for (size_t i = 0; i < this->size(); ++i) {
				if (!((*this)[i] == str[i])) return false;
			}
			return true;
		}
	};

	using StringView = StringViewBase<const char>;
	using StringViewW = StringViewBase<const wchar_t>;
	using StringViewU8 = StringViewBase<const char8_t>;
	using StringViewU16 = StringViewBase<const char16_t>;
}

