#pragma once
#include <cassert>
#include <type_traits>
#include <utility>
#include <memory>

//destruction and construction is manual and explicit
class ByteArray
{
public:
	uint8_t* m_bytes;

public:

	ByteArray() noexcept : m_bytes(nullptr)
	{};

	explicit ByteArray(size_t capacity) noexcept : m_bytes(new uint8_t[capacity])
	{};

	template <typename T>
	explicit ByteArray(T* array) noexcept : m_bytes(reinterpret_cast<uint8_t*>(array))
	{};

	template <typename T>
	explicit ByteArray(T* array, size_t size) noexcept : m_bytes(reinterpret_cast<uint8_t*>(array))
	{
		std::memcpy(m_bytes, array, size * sizeof(T));
	};

	explicit ByteArray(ByteArray&& other) noexcept
		: m_bytes(other.m_bytes)
	{
		other.m_bytes = nullptr;
	}

	ByteArray& operator=(ByteArray&& other) noexcept
	{
		if (this != &other) {
			// Transfer ownership
			m_bytes = other.m_bytes;
			other.m_bytes = nullptr;
		}
		return *this;
	}

	ByteArray(const ByteArray&) = delete;
	ByteArray& operator=(const ByteArray&) = delete;

	~ByteArray() noexcept {
	};

	void allocate(size_t capacity) noexcept
	{
		m_bytes = new uint8_t[capacity];
	}

	void destroy() noexcept
	{
		delete[] m_bytes;
		m_bytes = nullptr;
	}

	template <typename T>
	void emplace(T&& value, size_t offset) noexcept
	{
		::new (static_cast<void*>(m_bytes + offset)) std::decay_t<T>(std::forward<T>(value));
	}

	template <typename T, typename... Args>
	void emplace(size_t offset, Args&&... args) noexcept
	{
		::new (static_cast<void*>(m_bytes + offset)) std::decay_t<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	void emplaceRange(T&& value, size_t range, size_t offset) noexcept
	{
		::new (static_cast<void*>(m_bytes + offset)) std::decay_t<T>[range](std::forward<T>(value));
	}

	template <typename T, typename... Args>
	void emplaceRange(size_t range, size_t offset, Args&&... args) noexcept
	{
		for (size_t i = 0; i < range; ++i) {
			::new (static_cast<void*>(m_bytes + i * sizeof(T))) std::decay_t<T>(std::forward<Args>(args)...);
		}
	}

	template <typename T>
	void erase(size_t offset) noexcept
	{
		(reinterpret_cast<T*>(m_bytes + offset))->~T();
	}

	template <typename T>
	void erase(size_t range, size_t offset) noexcept
	{
		for (size_t i = 0; i < range; ++i)
			(reinterpret_cast<T*>(m_bytes + offset + i * sizeof(T)))->~T();;
	}

	template <typename T>
	static void erase(T* object) noexcept
	{
		object->~T();
	}

	template <typename T>
	void copy(const T* array, size_t range, size_t offset) noexcept
	{
		for (size_t i = 0; i < range; ++i)
			::new (reinterpret_cast<T*>(m_bytes + offset + i * sizeof(T))) T(array[i]);
	}

	template <typename T>
	T* get(size_t offset) noexcept
	{
		return reinterpret_cast<T*>(m_bytes + offset);
	}

	template<typename T = uint8_t>
	T* data() noexcept
	{
		return reinterpret_cast<T*>(m_bytes);
	}

	template <typename T>
	const T* get(size_t offset) const noexcept
	{
		return reinterpret_cast<T*>(m_bytes + offset);
	}

	const uint8_t* data() const noexcept
	{
		return m_bytes;
	}
};

