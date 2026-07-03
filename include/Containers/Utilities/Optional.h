#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "Containers/Utilities/ReusableStorage.h"
#include "Containers/Utilities/Concepts.h"

namespace Containers
{
    template <typename T, LifetimeManagerType<T> Life = LifetimeManager<T>>
    class Optional {
    private:
        ReusableStorage<T> m_data;
        bool m_occupied;

    public:
        Optional() = default;
        template<typename... Args>
        Optional(Args...&& args) { emplace(std::forward<Args>(args)...); }

        inline constexpr T& value() noexcept { return m_data.value(); }
        inline constexpr const T& value() const noexcept { return m_data.value(); }
        
        inline constexpr T& valueLaundered() noexcept { return m_data.valueLaundered(); }
        inline constexpr const T& valueLaundered() const noexcept { return m_data.valueLaundered(); }
        
        // Pointer operators
        inline constexpr T* operator->() noexcept { return m_data.data(); }
        inline constexpr const T* operator->() const noexcept { return m_data.data(); }
        
        // Reference operators
        inline constexpr T& operator*() noexcept { return m_data.value(); }
        inline constexpr const T& operator*() const noexcept { return m_data.value(); }

        bool empty() const { return !m_occupied; }

        template<typename... Args>
        Optional& emplace(Args...&& args) {
            Life life;
            life.construct(m_data.data(), std::forward<Args>(args)...);
            m_occupied = true;
            return *this;
        }

        template<typename... Args>
        Optional& clear() {
            Life life;
            life.destroy(m_data.data());
            m_occupied = false;
            return *this;
        }
    };
}