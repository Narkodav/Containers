#pragma once
#include <memory>
#include <stdexcept>
#include <concepts>

#include "InitializerList.h"
#include "Array.h"

namespace Containers
{

	template <typename T>
	concept EnumType = std::is_enum_v<T>;

	template <typename T, EnumType E, E s_size>
		requires std::is_default_constructible_v<T>
	class EnumArray : public Array<T, static_cast<size_t>(s_size)>
	{
	public:
		using ValueType = T;
		using Iterator = ValueType *;
		using ConstIterator = const ValueType *;
		using SizeType = size_t;

		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = SizeType;

		using Base = Array<T, static_cast<size_t>(s_size)>;

		using Base::Base;

		constexpr T &operator[](E index)
		{
			return Base::operator[](static_cast<size_t>(index));
		};
		constexpr const T &operator[](E index) const
		{
			return Base::operator[](static_cast<size_t>(index));
		};
	};
}