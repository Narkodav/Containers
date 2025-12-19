#pragma once
#include "Vector.h"
#include "Utilities/Macros.h"
#include "LinearContainer.h"

namespace Containers 
{
	template<typename CharType>
	size_t strlen(const CharType* str) {
		size_t i = 0;
		while (str[i] != 0) ++i;
		return i;
	}

	template<typename Left, typename Right>
	struct StringConcat {
		const Left& left;
		const Right& right;

		StringConcat(const Left& l, const Right& r) : left(l), right(r) {}

		size_t size() const {
			return left.size() + right.size();
		}

		template<typename StringType>
		void appendTo(StringType& result) const {
			left.appendTo(result);
			right.appendTo(result);
		}
	};

	template<typename Left, typename CharType>
	struct StringLiteralRightConcat {
		const Left& left;
		const CharType* right;
		size_t rightSize;

		StringLiteralRightConcat(const Left& l, const CharType* r) : left(l), right(r), rightSize(std::strlen(r)) {}

		size_t size() const {
			return left.size() + rightSize;
		}

		template<typename StringType>
		void appendTo(StringType& result) const {
			left.appendTo(result);
			result.append(right, rightSize);
		}
	};

	template<typename Right, typename CharType>
	struct StringLiteralLeftConcat {
		const CharType* left;
		size_t leftSize;
		const Right& right;

		StringLiteralLeftConcat(const CharType* l, const Right& r) : left(l), right(r), leftSize(std::strlen(l)) {}

		size_t size() const {
			return right.size() + leftSize;
		}

		template<typename StringType>
		void appendTo(StringType& result) const {
			result.append(left, leftSize);
			right.appendTo(result);
		}
	};

	template<typename Left, typename CharType, size_t rightSize>
	struct ConstStringLiteralRightConcat {
		const Left& left;
		const CharType* right;

		ConstStringLiteralRightConcat(const Left& l, const CharType(&r)[rightSize]) : left(l), right(r) {}

		size_t size() const {
			return left.size() + rightSize - 1;
		}

		template<typename StringType>
		void appendTo(StringType& result) const {
			left.appendTo(result);
			result.append(right, rightSize - 1);
		}
	};

	template<typename Right, typename CharType, size_t leftSize>
	struct ConstStringLiteralLeftConcat {
		const CharType* left;
		const Right& right;

		ConstStringLiteralLeftConcat(const CharType(&l)[leftSize + 1], const Right& r) : left(l), right(r) {}

		size_t size() const {
			return right.size() + leftSize;
		}

		template<typename StringType>
		void appendTo(StringType& result) const {
			result.append(left, leftSize);
			right.appendTo(result);
		}
	};

	template<typename CharType, AllocatorConcept<CharType> Alloc = TrivialAllocator<CharType>,
		size_t s_initialCapacity = 8, float s_growthFactor = 1.618f>
	class StringBase
	{
	public:
		static_assert(std::is_integral_v<CharType>, "CharType must be arithmetic type");
		static_assert(!std::is_same_v<CharType, bool>, "CharType cannot be a bool");

	public:
		using Container = Vector<CharType, Alloc, s_initialCapacity, s_growthFactor>;
		using ValueType = CharType;
		using Iterator = typename Container::Iterator;
		using ConstIterator = typename Container::ConstIterator;
		using SizeType = size_t;
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using value_type = ValueType;
		using size_type = SizeType;

	protected:
		Container m_data;

	public:

		StringBase() { m_data.resize(1); m_data[0] = '\0'; };
		~StringBase() = default;

		template <size_t size>
		constexpr StringBase(const CharType(&str)[size])
		{
			m_data.resize(size);
			for (size_t i = 0; i < size; ++i)
				m_data[i] = str[i];
		}

		template<typename T>
		constexpr StringBase(T&& str) requires (!std::is_array_v<T>
			&& (std::is_convertible_v<std::decay_t<T>, const CharType*>))
		{
			m_data.resize(strlen(str) + 1);
			for (size_t i = 0; i < m_data.size(); ++i)
				m_data[i] = str[i];
		}

		template <size_t size>
		constexpr StringBase& operator=(const CharType(&str)[size])
		{
			m_data.resize(size);
			for (size_t i = 0; i < size; ++i)
				m_data[i] = str[i];
			return *this;
		};

		template<typename T>
		StringBase& operator=(T&& str) requires (!std::is_array_v<T>
			&& std::is_convertible_v<std::decay_t<T>, const CharType*>)
		{
			m_data.resize(std::strlen(str) + 1);
			for (size_t i = 0; i < m_data.size(); ++i)
				m_data[i] = str[i];
			return *this;
		};

		StringBase(size_t size, CharType value = '\0') {
			m_data.resize(size + 1);
			for (size_t i = 0; i < size; ++i)
				m_data[i] = value;
			m_data[size] = '\0';
		};

		StringBase(const std::basic_string<CharType>& str)
		{
			m_data.resize(str.size() + 1);
			for (size_t i = 0; i < str.size(); ++i)
				m_data[i] = str[i];
			m_data[str.size()] = '\0';
		}

		StringBase& operator=(const std::basic_string<CharType>& str)
		{
			m_data.resize(str.size() + 1);
			for (size_t i = 0; i < str.size(); ++i)
				m_data[i] = str[i];
			m_data[str.size()] = '\0';
			return *this;
		};

		StringBase(const StringBase& other) : m_data(other.m_data) {};

		StringBase(StringBase&& other) requires std::is_move_constructible_v<Alloc>
			: m_data(std::move(other.m_data)) {};

		StringBase& operator=(const StringBase& other) {
			if (this == &other)
				return *this;
			m_data = other.m_data;
			return *this;
		};

		StringBase& operator=(StringBase&& other) requires std::is_move_assignable_v<Alloc>
		{
			if (this == &other)
				return *this;
			m_data = std::move(other.m_data);
			return *this;
		};

		inline CharType& operator[](size_t index) { return m_data[index]; };
		inline const CharType& operator[](size_t index) const { return m_data[index]; };
		inline CharType& at(size_t index) { return m_data.at(index); };
		inline const CharType& at(size_t index) const { return m_data.at(index); }
		inline const size_t size() const { return m_data.size() - 1; };
		inline bool empty() const { return m_data.size() <= 1; };
		inline const size_t capacity() const { return m_data.capacity(); };
		inline const CharType* data() const { return m_data.data(); };
		inline const CharType* cStr() const { return m_data.data(); };
		inline CharType* data() { return m_data.data(); };

		inline CharType& back() { return m_data[m_data.size() - 2]; };
		inline const CharType& back() const { return m_data[m_data.size() - 2]; };
		inline CharType& front() { return m_data[0]; };
		inline const CharType& front() const { return m_data[0]; };

		inline Iterator begin() { return m_data.begin(); }
		inline Iterator end() { return m_data.end() - 1; }
		inline ConstIterator begin() const { return m_data.begin(); }
		inline ConstIterator end() const { return m_data.end() - 1; }
		inline ConstIterator cbegin() const { return m_data.cbegin(); }
		inline ConstIterator cend() const { return m_data.cend() - 1; }

		void assign(CharType* data, size_t size) requires std::is_same_v<Allocator<CharType>, Alloc>
		{
			m_data.assign(data, size);
		}

		ReleaseData<CharType, Alloc> release() {
			return m_data.release();
		}

		void reserve(size_t capacity) {
			m_data.reserve(capacity);
		};

		void resize(size_t size)
		{
			m_data.resize(size + 1);
			m_data[size] = '\0';
		};

		void clear() {
			m_data.clear();
			m_data.pushBack('\0');
		};

		template<typename U>
		void pushBack(U&& data)
		{
			m_data.pushBack('\0');
			m_data[m_data.size() - 2] = std::forward<U>(data);
		}

		void popBack()
		{
			m_data.popBack();
			m_data.back() = '\0';
		}

		template<typename U, typename ItType>
		Iterator insert(ItType pos, U&& value) requires
			(std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>)
		{
			return m_data.insert(pos, std::forward<U>(value));
		}

		template<typename U, typename ItType>
		Iterator insert(ItType pos, size_t count, U&& value) requires
			(std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>)
		{
			return m_data.insert(pos, count, std::forward<U>(value));
		}

		template<typename ItType, typename InputIt>
		Iterator insert(ItType pos, InputIt first, InputIt last) requires
			(std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>)
		{
			return m_data.insert(pos, first, last);
		}

		template<typename ItType>
		Iterator insert(ItType pos, std::initializer_list<CharType> list) requires
			(std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>)
		{
			return m_data.insert(pos, list);
		}

		template <typename ItType>
		Iterator erase(ItType pos) requires
			(std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>)
		{
			return m_data.erase(pos);
		}

		template <typename ItType>
		Iterator erase(ItType first, ItType last) requires
			(std::is_same_v<ItType, Iterator> ||
				std::is_same_v<ItType, ConstIterator>)
		{
			return m_data.erase(first, last);
		}

		void shrinkToFit() {
			m_data.shrinkToFit();
		}

		template <size_t size>
		constexpr StringBase& append(const CharType(&s)[size])
		{
			return append(s, s + size - 1);
		}

		template<typename T>
		StringBase& append(T&& s)
			requires (std::is_pointer_v<std::remove_reference_t<T>>&&
		std::same_as<std::remove_cv_t<std::remove_pointer_t<
			std::remove_reference_t<T>>>, CharType>)
		{
			size_t size = std::strlen(s);
			return append(s, s + size);
		}

		template <typename InputIt>
		StringBase& append(InputIt first, InputIt last)
		{
			size_t i = m_data.size() - 1;
			m_data.resize(m_data.size() + (last - first));
			for (; first != last; ++i, ++first)
				m_data[i] = *first;
			m_data[i] = '\0';
			return *this;
		}

		StringBase& append(const CharType* s, size_t n)
		{
			return append(s, s + n);
		}

		StringBase& append(const StringBase& str)
		{
			return append(str.begin(), str.end());
		}

		StringBase& append(const StringBase& str, size_t subpos, size_t sublen)
		{
			return append(str.begin() + subpos, str.begin() + subpos + sublen);
		}
		
		StringBase& append(size_t n, CharType c)
		{
			size_t i = m_data.size() - 1;
			m_data.resize(m_data.size() + n);
			for (; i < m_data.size(); ++i)
				m_data[i] = c;
			m_data[i] = '\0';
			return *this;
		}

		StringBase& appendTo(StringBase& result) const
		{
			return result.append(begin(), end());
		}

		// For C-style arrays: const Type(&right)[N]
		template<size_t N>
		auto operator+=(const CharType(&right)[N]) {
			this->append(right);
			return *this;
		}

		// For pointers: const Type*
		template<typename Right>
		auto operator+=(Right&& right)
			requires (std::is_pointer_v<std::remove_reference_t<Right>>&&
			std::same_as<std::remove_cv_t<std::remove_pointer_t<
			std::remove_reference_t<Right>>>, CharType>) {
			this->append(right);
			return *this;
		}

		// For everything else
		template<typename Right>
		auto operator+=(Right&& right)
			requires (!std::is_array_v<std::remove_reference_t<Right>> && !std::is_pointer_v<std::remove_reference_t<Right>>) {
			StringBase str;
			str.reserve(right.size() + m_data.size() - 1);
			str = *this;
			right.appendTo(str);
			m_data = std::move(str.m_data);
			return *this;
		}

		template<typename Right, typename Left>
		StringBase(const StringConcat<Right, Left>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
		}

		template<typename Right, typename Left>
		StringBase(const StringLiteralLeftConcat<Right, Left>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
		}

		template<typename Right, typename Left>
		StringBase(const StringLiteralRightConcat<Right, Left>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
		}

		template<typename Right, typename Left, size_t size>
		StringBase(const ConstStringLiteralLeftConcat<Right, Left, size>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
		}

		template<typename Right, typename Left, size_t size>
		StringBase(const ConstStringLiteralRightConcat<Right, Left, size>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
		}

		template<typename Right, typename Left>
		StringBase& operator=(const StringConcat<Right, Left>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
			return *this;
		}

		template<typename Right, typename Left>
		StringBase& operator=(const StringLiteralLeftConcat<Right, Left>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
			return *this;
		}

		template<typename Right, typename Left>
		StringBase& operator=(const StringLiteralRightConcat<Right, Left>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
			return *this;
		}

		template<typename Right, typename Left, size_t size>
		StringBase& operator=(const ConstStringLiteralLeftConcat<Right, Left, size>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
			return *this;
		}

		template<typename Right, typename Left, size_t size>
		StringBase& operator=(const ConstStringLiteralRightConcat<Right, Left, size>& expr)
		{
			StringBase str;
			str.reserve(expr.size());
			expr.appendTo(str);
			m_data = std::move(str.m_data);
			return *this;
		}

		//template<typename Expr>
		//StringBase(const Expr& expr) requires
		//	std::is_same_v<std::decay_t<Expr>, StringConcat> ||
		//	std::is_same_v<std::decay_t<Expr>, StringLiteralLeftConcat> ||
		//	std::is_same_v<std::decay_t<Expr>, StringLiteralRightConcat> ||
		//	std::is_same_v<std::decay_t<Expr>, ConstStringLiteralLeftConcat> ||
		//	std::is_same_v<std::decay_t<Expr>, ConstStringLiteralRightConcat>
		//{
		//	m_data.reserve(expr.size());
		//	expr.appendTo(*this);
		//}

		//template<typename Expr>
		//StringBase& operator=(const Expr& expr) requires
		//	std::is_same_v<std::decay_t<Expr>, StringConcat> ||
		//	std::is_same_v<std::decay_t<Expr>, StringLiteralLeftConcat> ||
		//	std::is_same_v<std::decay_t<Expr>, StringLiteralRightConcat> ||
		//	std::is_same_v<std::decay_t<Expr>, ConstStringLiteralLeftConcat> ||
		//	std::is_same_v<std::decay_t<Expr>, ConstStringLiteralRightConcat>
		//{
		//	m_data.clear();
		//	m_data.reserve(expr.size());
		//	expr.appendTo(*this);
		//	return *this;
		//}

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

		bool operator==(const CharType* str) const {
			if (this->size() != Containers::strlen(str)) return false;
			for (size_t i = 0; i < this->size(); ++i) {
				if (!((*this)[i] == str[i])) return false;
			}
			return true;
		}

		friend std::ostream& operator<<(std::ostream& os, const StringBase& obj) {
			os << obj.data();
			return os;
		}

		friend std::istream& operator>>(std::istream& is, StringBase& obj) {
			is >> obj.data();
			return is;
		}
	};

	using String = StringBase<char>;
	using StringW = StringBase<wchar_t>;
	using String8 = StringBase<char8_t>;
	using String16 = StringBase<char16_t>;
	using String32 = StringBase<char32_t>;

	//template<typename Left, typename Right>
	//auto operator+(const Left& left, const Right& right) requires std::is_same_v<Left, Right> {
	//	return StringConcat<Left, Right>(left, right);
	//}

	//template<typename Left, typename Right>
	//auto operator+(const Left& left, Right&& right) requires (!std::is_array_v<Right>) {
	//	using CharType = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<Right>>>;
	//	return StringLiteralRightConcat<Left, CharType>(left, right);
	//}

	//template<typename Left, typename CharType, size_t rightSize>
	//auto operator+(const Left& left, const CharType(&right)[rightSize]) {
	//	return ConstStringLiteralRightConcat<Left, CharType, rightSize>(left, right);
	//}


	// For C-style arrays: const Type(&right)[N]
	template<typename Left, typename CharType, size_t N>
	auto operator+(Left&& left, const CharType(&right)[N]) {
		return ConstStringLiteralRightConcat<Left, CharType, N>(std::forward<Left>(left), right);
	}

	// For pointers: const Type*
	template<typename Left, typename Right>
	auto operator+(Left&& left, Right&& right) 
		requires std::is_pointer_v<std::remove_reference_t<Right>> {
		using CharType = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<Right>>>;
		return StringLiteralRightConcat<Left, CharType>(std::forward<Left>(left), std::forward<Right>(right));
	}

	// For everything else
	template<typename Left, typename Right>
	auto operator+(Left&& left, Right&& right) 
		requires (!std::is_array_v<std::remove_reference_t<Right>> && !std::is_pointer_v<std::remove_reference_t<Right>>) {
		return StringConcat<Left, Right>(std::forward<Left>(left), std::forward<Right>(right));
	}
}
