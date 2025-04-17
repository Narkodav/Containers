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

	ByteArray() : m_bytes(nullptr)
	{};

	explicit ByteArray(size_t capacity) : m_bytes(new uint8_t[capacity])
	{};

	template <typename T>
	explicit ByteArray(T* array) : m_bytes(reinterpret_cast<uint8_t*>(array))
	{};

	template <typename T>
	explicit ByteArray(T* array, size_t size) : m_bytes(reinterpret_cast<uint8_t*>(array))
	{
		std::memcpy(m_bytes, array, size * sizeof(T));
	};

	~ByteArray() {
	};

	void allocate(size_t capacity)
	{
		m_bytes = new uint8_t[capacity];
	}

	void destroy()
	{
		delete[] m_bytes;
	}

	template <typename T>
	void emplace(T&& value, size_t offset)
	{
		::new (static_cast<void*>(m_bytes + offset)) std::decay_t<T>(std::forward<T>(value));
	}

	template <typename T, typename... Args>
	void emplace(size_t offset, Args&&... args)
	{
		::new (static_cast<void*>(m_bytes + offset)) std::decay_t<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	void emplaceRange(T&& value, size_t range, size_t offset)
	{
		::new (static_cast<void*>(m_bytes + offset)) std::decay_t<T>[range](std::forward<T>(value));
	}

	template <typename T, typename... Args>
	void emplaceRange(size_t range, size_t offset, Args&&... args)
	{
		for (size_t i = 0; i < range; ++i) {
			::new (static_cast<void*>(m_bytes + i * sizeof(T))) std::decay_t<T>(std::forward<Args>(args)...);
		}
	}

	template <typename T>
	void erase(size_t offset)
	{
		(reinterpret_cast<T*>(m_bytes + offset))->~T();
	}

	template <typename T>
	void erase(size_t range, size_t offset)
	{
		for (size_t i = 0; i < range; ++i)
			(reinterpret_cast<T*>(m_bytes + offset + i * sizeof(T)))->~T();;
	}

	template <typename T>
	void copy(const T* array, size_t range, size_t offset)
	{
		for (size_t i = 0; i < range; ++i)
			::new (reinterpret_cast<T*>(m_bytes + offset + i * sizeof(T))) T(array[i]);
	}

	template <typename T>
	T* get(size_t offset)
	{
		return reinterpret_cast<T*>(m_bytes + offset);
	}

	template<typename T = uint8_t>
	T* data()
	{
		return reinterpret_cast<T*>(m_bytes);
	}

	template <typename T>
	const T* get(size_t offset) const
	{
		return reinterpret_cast<T*>(m_bytes + offset);
	}

	const uint8_t* data() const
	{
		return m_bytes;
	}
};

