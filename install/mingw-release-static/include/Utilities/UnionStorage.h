#pragma once
#include <cassert>
#include <type_traits>
#include <utility>
#include <memory>

namespace Containers
{
    //* Minimal zero - overhead storage wrapper with explicit lifetime control.
    //* User MUST explicitly destroy before modifying storage.

    template <typename T>
    class alignas(T) UnionStorage
    {
        union Data
        {
            T m_value;
            struct EmptyState
            {
            } m_empty; // Dummy member for empty state
        };
        Data m_data;
        bool m_engaged = false; // Debug-only tracking

    public:
        //---------------------------------------------------------------------
        // CONSTRUCTION (STARTS UNENGAGED)
        //---------------------------------------------------------------------
        UnionStorage() noexcept {}; // Starts disengaged

        // User MUST call destroy() before destructor if engaged
        ~UnionStorage() noexcept
        {
            assert(!m_engaged && "Value must be explicitly destroyed");
        }

        //-----------------------------------------------------------------------
        // EXPLICIT MOVE SEMANTICS (STRICT LIFETIME RULES)
        //-----------------------------------------------------------------------

        /// @brief Move-construct from source. Source MUST be destroyed after.
        /// @param src Must be engaged (debug assert)
        /// @warning Source becomes invalid after this operation
        inline void moveConstructFrom(UnionStorage &src) noexcept(std::is_nothrow_move_constructible_v<T>)
        {
            assert(!m_engaged && "Destination must be unengaged");
            assert(src.m_engaged && "Source must be engaged");
            new (&m_value) T(std::move(src.m_value));
            m_engaged = true;
        }

        /// @brief Copy-construct from source.
        /// @param src Must be engaged (debug assert)
        inline void copyConstructFrom(const UnionStorage &src) noexcept(std::is_nothrow_copy_constructible_v<T>)
        {
            assert(!m_engaged && "Destination must be unengaged");
            assert(src.m_engaged && "Source must be engaged");
            new (&m_value) T(src.m_value);
            m_engaged = true;
        }

        /// @brief Move-construct from T object.
        /// @param Destination must be not engaged
        inline void moveConstruct(T &&val) noexcept(std::is_nothrow_move_constructible_v<T>)
        {
            assert(!m_engaged && "Destination must be unengaged");
            new (&m_value) T(std::move(val));
            m_engaged = true;
        }

        /// @brief Copy-construct from T object.
        /// @param Destination must be not engaged
        inline void copyConstruct(const T &val) noexcept(std::is_nothrow_copy_constructible_v<T>)
        {
            assert(!m_engaged && "Destination must be unengaged");
            new (&m_value) T(val);
            m_engaged = true;
        }

        /// @brief Copy-construct from T object.
        /// @param Destination must be not engaged
        template <typename U>
            requires std::is_same_v<T, std::decay_t<U>>
        inline void perfectForwardConstruct(U &&val) noexcept(std::is_nothrow_constructible_v<T, U &&>)
        {
            assert(!m_engaged && "Destination must be unengaged");
            new (&m_value) T(std::forward<U>(val));
            m_engaged = true;
        }

        //---------------------------------------------------------------------
        // STATE INSPECTION
        //---------------------------------------------------------------------
        [[nodiscard]] inline bool isEngaged() const noexcept { return m_engaged; }

        //-------------------------------------------------------------------------
        // ACCESSORS (all noexcept)
        //-------------------------------------------------------------------------

        inline T &get() noexcept
        {
            assert(m_engaged && "Accessing unengaged storage");
            return reinterpret_cast<T &>(m_value);
        }

        inline const T &get() const noexcept
        {
            assert(m_engaged && "Accessing unengaged storage");
            return reinterpret_cast<const T &>(m_value);
        }

        //-------------------------------------------------------------------------
        // LIFETIME MANAGEMENT
        //-------------------------------------------------------------------------

        // Explicit construction
        template <typename... Args>
        inline void construct(Args &&...args)
        {
            assert(!m_engaged && "Double construction");
            new (&m_value) T(std::forward<Args>(args)...);
            m_engaged = true;
        }

        ///**
        // * Destroy contained value - USER MUST CALL BEFORE REUSE
        // * After this call, the storage is invalid until reset
        // */
        inline void destroy() noexcept(std::is_nothrow_destructible_v<T>)
        {
            assert(m_engaged && "Double destruction");
            std::destroy_at(&m_value);
            m_engaged = false;
        }

        ///**
        // * Reset storage with new value
        // * WARNING: Assumes the value is engaged, first destroys then constructs
        // */
        template <typename U>
        inline void reset(U &&new_value)
        {
            assert(m_engaged && "Value must be engaged to be reset");
            destroy();
            construct(std::forward<U>(new_value));
        }

        //-------------------------------------------------------------------------
        // NO IMPLICIT COPIES/MOVES
        //-------------------------------------------------------------------------
        UnionStorage(const UnionStorage &) = delete;
        UnionStorage &operator=(const UnionStorage &) = delete;
        UnionStorage(UnionStorage &&) = delete;
        UnionStorage &operator=(UnionStorage &&) = delete;
    };
}