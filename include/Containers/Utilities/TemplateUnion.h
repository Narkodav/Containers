#pragma once
#include <cassert>
#include <type_traits>
#include <utility>
#include <memory>
#include <variant>

#include "Containers/Utilities/Concepts.h"

namespace Containers
{
    // Contains the amount of T in Ts
    template<typename T, typename First, typename... Rest>
    struct TupleHasType {
        static inline const size_t value = std::is_same_v<T, First> + TupleHasType<T, Rest...>::value;
    };

    template<typename T, typename First>
    struct TupleHasType<T, First> {
        static inline const size_t value = std::is_same_v<T, First>;
    };

    template<typename T, typename... Ts>
    struct TupleHasDuplicates {
        static inline const bool value = static_cast<bool>(TupleHasType<T, Ts...>::value) || TupleHasDuplicates<Ts...>::value;
    };

    template<typename T>
    struct TupleHasDuplicates<T> {
        static inline const bool value = false;
    };

    template<size_t index, typename T, typename First, typename... Rest>
    struct TupleTypeToIndex {
        static inline const size_t value = std::is_same_v<T, First> ? index : TupleTypeToIndex<index + 1, T, Rest...>::value;
    };

    template<size_t index, typename T, typename Last>
    struct TupleTypeToIndex<index, T, Last> {
        static inline const size_t value = std::is_same_v<T, Last> ? index : std::numeric_limits<size_t>::max();
    };

    template<size_t index, typename T, typename... Ts>
    struct TupleTypeAtIndexBase {
        using Type = std::conditional_t<index == 0, T, typename TupleTypeAtIndexBase<index - 1, Ts...>::Type>;
    };

    template<size_t index, typename T>
    struct TupleTypeAtIndexBase<index, T> {        
        using Type = T;
    };

    template<size_t index, typename... Ts>
    struct TupleTypeAtIndex {
        static_assert(index < sizeof...(Ts), "Type not found in tuple");
        using Type = TupleTypeAtIndexBase<index, Ts...>::Type;
    };

    template<template<typename Conditional>typename Condition, typename T, typename... Ts>
    struct AllTupleTypesSatisfyCondition {
        static inline const bool value = Condition<T>::value && AllTupleTypesSatisfyCondition<Condition, Ts...>::value;
    };

    template<template<typename Conditional>typename Condition, typename T>
    struct AllTupleTypesSatisfyCondition<Condition, T> {
        static inline const bool value = Condition<T>::value;
    };

    template<typename... Ts>
    struct AllTupleTypesTriviallyConstructible {
        static inline const bool value = AllTupleTypesSatisfyCondition<std::is_trivially_constructible, Ts...>::value;
    };

    template<typename... Ts>
    struct AllTupleTypesTriviallyCopyConstructible {
        static inline const bool value = AllTupleTypesSatisfyCondition<std::is_trivially_copy_constructible, Ts...>::value;
    };

    template<typename... Ts>
    struct AllTupleTypesTriviallyMoveConstructible {
        static inline const bool value = AllTupleTypesSatisfyCondition<std::is_trivially_move_constructible, Ts...>::value;
    };

    template<typename... Ts>
    struct AllTupleTypesTriviallyCopyAssignable {
        static inline const bool value = AllTupleTypesSatisfyCondition<std::is_trivially_copy_assignable, Ts...>::value;
    };

    template<typename... Ts>
    struct AllTupleTypesTriviallyMoveAssignable {
        static inline const bool value = AllTupleTypesSatisfyCondition<std::is_trivially_move_assignable, Ts...>::value;
    };

    template<typename... Ts>
    struct AllTupleTypesTriviallyDestructible {
        static inline const bool value = AllTupleTypesSatisfyCondition<std::is_trivially_destructible, Ts...>::value;
    };

    // Implements functions for a union fixture
    template<typename T, typename... Ts>
    union TemplateUnionDataBase {
    private:
        T head;
        TemplateUnionDataBase<Ts...> tail;
    public:
        constexpr TemplateUnionDataBase() noexcept = default;
        constexpr TemplateUnionDataBase() noexcept requires(!AllTupleTypesTriviallyConstructible<T, Ts...>::value) {}
        
        constexpr ~TemplateUnionDataBase() noexcept = default;
        constexpr ~TemplateUnionDataBase() noexcept requires(!AllTupleTypesTriviallyDestructible<T, Ts...>::value) {}

        template<size_t index>
        constexpr TupleTypeAtIndex<index, T, Ts...>::Type& get() {
            if constexpr (index == 0) {
                return head;
            } else {
                return tail.template get<index - 1>();
            }
        }

        template<size_t index>
        constexpr const TupleTypeAtIndex<index, T, Ts...>::Type& get() const {
            if constexpr (index == 0) {
                return head;
            } else {
                return tail.template get<index - 1>();
            }
        }

        template<size_t index, typename... Args>
        constexpr void set(Args... args) {
            LifetimeManager<T> life;
            if constexpr (index == 0) {
                life.construct(&head, std::forward<Args>(args)...);
            } else {
                tail.template set<index - 1>(std::forward<Args>(args)...);
            }
        }
    };

    template<typename T>
    concept CArrayType = std::is_array_v<T>;

    template<CArrayType T, typename... Ts>
    union TemplateUnionDataBase<T, Ts...> {
    private:
        T head;
        TemplateUnionDataBase<Ts...> tail;
    public:
        constexpr TemplateUnionDataBase() noexcept = default;
        constexpr TemplateUnionDataBase() noexcept requires(!AllTupleTypesTriviallyConstructible<T, Ts...>::value) {}
        
        constexpr ~TemplateUnionDataBase() noexcept = default;
        constexpr ~TemplateUnionDataBase() noexcept requires(!AllTupleTypesTriviallyDestructible<T, Ts...>::value) {}

        template<size_t index>
        constexpr TupleTypeAtIndex<index, T, Ts...>::Type& get() {
            if constexpr (index == 0) {
                return head;
            } else {
                return tail.template get<index - 1>();
            }
        }

        template<size_t index>
        constexpr const TupleTypeAtIndex<index, T, Ts...>::Type& get() const {
            if constexpr (index == 0) {
                return head;
            } else {
                return tail.template get<index - 1>();
            }
        }

        template<size_t index, typename... Args>
        constexpr void set(Args... args) {
            if constexpr (index == 0) {
                static_assert(index != 0, "Cannot set a C array");
            } else {
                tail.template set<index - 1>(std::forward<Args>(args)...);
            }
        }
    };

    template<typename T>
    concept TriviallyCopyableNotArrayType = (std::is_trivially_copyable_v<T> && !std::is_array_v<T>);

    template<TriviallyCopyableNotArrayType T, typename... Ts>
    union TemplateUnionDataBase<T, Ts...> {
    private:
        T head;
        TemplateUnionDataBase<Ts...> tail;
    public:
        constexpr TemplateUnionDataBase() noexcept = default;
        constexpr TemplateUnionDataBase() noexcept requires(!AllTupleTypesTriviallyConstructible<T, Ts...>::value) {}
        
        constexpr ~TemplateUnionDataBase() noexcept = default;
        constexpr ~TemplateUnionDataBase() noexcept requires(!AllTupleTypesTriviallyDestructible<T, Ts...>::value) {}

        template<size_t index>
        constexpr TupleTypeAtIndex<index, T, Ts...>::Type& get() {
            if constexpr (index == 0) {
                return head;
            } else {
                return tail.template get<index - 1>();
            }
        }

        template<size_t index>
        constexpr const TupleTypeAtIndex<index, T, Ts...>::Type& get() const {
            if constexpr (index == 0) {
                return head;
            } else {
                return tail.template get<index - 1>();
            }
        }

        template<size_t index, typename Arg>
        constexpr void set(Arg&& arg) {
            if constexpr (index == 0) {
                this->head = std::forward<Arg>(arg);
            } else {
                tail.template set<index - 1>(std::forward<Arg>(arg));
            }
        }
    };

    template<typename T>
    union TemplateUnionDataBase<T> {
    private:
        T head;
    public:
        constexpr TemplateUnionDataBase() noexcept = default;
        constexpr TemplateUnionDataBase() noexcept requires(!AllTupleTypesTriviallyConstructible<T>::value) {}
        
        constexpr ~TemplateUnionDataBase() noexcept = default;
        constexpr ~TemplateUnionDataBase() noexcept requires(!AllTupleTypesTriviallyDestructible<T>::value) {}

        template<size_t index>
        constexpr TupleTypeAtIndex<index, T>::Type& get() {
            static_assert(index == 0, "Index out of bounds");
            return head;
        }

        template<size_t index>
        constexpr const TupleTypeAtIndex<index, T>::Type& get() const {
            static_assert(index == 0, "Index out of bounds");
            return head;
        }

        template<size_t index, typename... Args>
        constexpr void set(Args... args) {            
            static_assert(index == 0, "Index out of bounds");
            LifetimeManager<T> life;
            life.construct(&head, std::forward<Args>(args)...);
        }
    };
    
    template<typename T, typename... Ts>
    struct TupleMaximumByteSize {
        static inline const size_t value = std::max(sizeof(T), TupleMaximumByteSize<Ts...>::value);
    };

    template<typename T>
    struct TupleMaximumByteSize<T> {
        static inline const size_t value = sizeof(T);
    };


    template<typename... Ts>
    class TemplateUnion {
    public:
        using Bytes = std::byte[TupleMaximumByteSize<Ts...>::value];
    private:
        TemplateUnionDataBase<Bytes, Ts...> m_storage;
        
    public:

        constexpr TemplateUnion() noexcept = default;
        constexpr TemplateUnion() noexcept requires(!AllTupleTypesTriviallyConstructible<Ts...>::value) {}
        
        constexpr ~TemplateUnion() noexcept = default;
        constexpr ~TemplateUnion() noexcept requires(!AllTupleTypesTriviallyDestructible<Ts...>::value) {}

        constexpr TemplateUnion(const TemplateUnion& other) noexcept = default;
        constexpr TemplateUnion(TemplateUnion&& other) noexcept = default;
        constexpr TemplateUnion& operator=(const TemplateUnion& other) noexcept = default;
        constexpr TemplateUnion& operator=(TemplateUnion&& other) noexcept = default;

        template<size_t index>
        constexpr TupleTypeAtIndex<index, Ts...>::Type& get() { return m_storage.template get<index + 1>(); }

        template<size_t index>
        constexpr const TupleTypeAtIndex<index, Ts...>::Type& get() const { return m_storage.template get<index + 1>(); }

        template<typename T>
        constexpr T& get() requires (static_cast<bool>(TupleHasType<T, Ts...>::value) 
            && !TupleHasDuplicates<Ts...>::value) {
            constexpr size_t index = TupleTypeToIndex<0, T, Bytes, Ts...>::value;
            static_assert(index != std::numeric_limits<size_t>::max(), "TemplateUnion doesnt contain required type");
            return m_storage.template get<index>();
        }

        template<typename T>
        constexpr const T& get() const requires (static_cast<bool>(TupleHasType<T, Ts...>::value) 
            && !TupleHasDuplicates<Ts...>::value) {
            constexpr size_t index = TupleTypeToIndex<0, T, Bytes, Ts...>::value;
            static_assert(index != std::numeric_limits<size_t>::max(), "TemplateUnion doesnt contain required type");
            return m_storage.template get<index>();
        }

        constexpr Bytes& getBytes() { return m_storage.template get<0>(); }
        constexpr const Bytes& getBytes() const { return m_storage.template get<0>(); }

        template<size_t index, typename... Args>
        constexpr auto& set(Args... args) {
            m_storage.template set<index + 1>(std::forward<Args>(args)...);
            return *this;
        }

        template<typename T, typename... Args>
        constexpr auto& set(Args... args) {
            constexpr size_t index = TupleTypeToIndex<0, T, Bytes, Ts...>::value;
            static_assert(index != std::numeric_limits<size_t>::max(), "TemplateUnion doesnt contain required type");
            m_storage.template set<index>(std::forward<Args>(args)...);
            return *this;
        }


    };
}