#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Containers
{
    template<typename T>
    class alignas(T) ReusableStorage {
    private:
        alignas(T) std::byte m_data[sizeof(T)];
    public:
        inline constexpr T* data() noexcept { return reinterpret_cast<T*>(m_data); }        
        inline constexpr const T* data() const noexcept { return reinterpret_cast<const T*>(m_data); }
        
        inline constexpr T& value() noexcept { return *data(); }
        inline constexpr const T& value() const noexcept { return *data(); }

        inline constexpr T* dataLaundered() noexcept { return std::launder(data()); }        
        inline constexpr const T* dataLaundered() const noexcept { return std::launder(data()); }
        
        inline constexpr T& valueLaundered() noexcept { return *dataLaundered(); }
        inline constexpr const T& valueLaundered() const noexcept { return *dataLaundered(); }
        
        // Pointer operators
        inline constexpr T* operator->() noexcept { return data(); }
        inline constexpr const T* operator->() const noexcept { return data(); }
        
        // Reference operators
        inline constexpr T& operator*() noexcept { return value(); }
        inline constexpr const T& operator*() const noexcept { return value(); }
        
        // Raw storage access (for memcpy, etc.)
        inline constexpr std::byte* raw() noexcept { return m_data; }
        inline constexpr const std::byte* raw() const noexcept { return m_data; }
        
        // Size/alignment utilities
        static inline constexpr size_t size() noexcept { return sizeof(T); }
        static inline constexpr size_t alignment() noexcept { return alignof(T); }
        
        // Array conversion helpers
        inline constexpr static T* toArray(ReusableStorage* arr) noexcept {
            return reinterpret_cast<T*>(arr);
        }
        
        inline constexpr static const T* toArray(const ReusableStorage* arr) noexcept {
            return reinterpret_cast<const T*>(arr);
        }
        
        inline constexpr static ReusableStorage* fromArray(T* arr) noexcept {
            return reinterpret_cast<ReusableStorage*>(arr);
        }
        
        inline constexpr static const ReusableStorage* fromArray(const T* arr) noexcept {
            return reinterpret_cast<const ReusableStorage*>(arr);
        }
    };

    static_assert(sizeof(ReusableStorage<uint8_t>) == sizeof(uint8_t));
    static_assert(alignof(ReusableStorage<uint8_t>) == alignof(uint8_t));
    static_assert(std::is_trivial_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_constructible_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_default_constructible_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_destructible_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_copyable_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_assignable_v<ReusableStorage<uint8_t>, ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_copy_assignable_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_move_assignable_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_copy_constructible_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_trivially_move_constructible_v<ReusableStorage<uint8_t>>);
    static_assert(std::is_standard_layout_v<ReusableStorage<uint8_t>>);
}