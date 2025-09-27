#pragma once
#include "Vector.h"
#include "Array.h"

namespace Containers {
    //linear storage type
    template<typename Container>
    concept LinearContainerType = requires(Container container,
        typename Container::ValueType value, typename Container::SizeType index) {
        // Type requirements
        typename Container::ValueType;
        typename Container::SizeType;

        // Iterator type requirements
        typename Container::Iterator;
        typename Container::ConstIterator;

        // Construction/Destruction
        Container();
        std::destructible<Container>;

        { container[index] } -> std::same_as<typename Container::ValueType&>;
        { std::as_const(container[index]) } -> std::same_as<const typename Container::ValueType&>;

        // Iterator operations
        { container.begin() } -> std::same_as<typename Container::Iterator>;
        { container.end() } -> std::same_as<typename Container::Iterator>;
        { std::as_const(container).begin() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).end() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).cbegin() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).cend() } -> std::same_as<typename Container::ConstIterator>;

        // Size operations
        { container.empty() } -> std::same_as<bool>;
        { container.size() } -> std::same_as<typename Container::SizeType>;

        // Iterator requirements
            requires std::random_access_iterator<typename Container::Iterator>;
            requires std::random_access_iterator<typename Container::ConstIterator>;
            requires std::convertible_to<
                typename std::iterator_traits<typename Container::Iterator>::value_type,
                    typename Container::ValueType
            >;

        // Copy operations if ValueType is copyable
            requires !std::is_copy_constructible_v<typename Container::ValueType> ||
                    std::is_copy_constructible_v<Container> && std::is_copy_assignable_v<Container>;
    };

    template<typename Container>
    concept LinearContainerDynamicType = LinearContainerType<Container> && requires(Container container,
        typename Container::ValueType value, typename Container::SizeType index) {

        // Dynamic operations
            { container.pushBack(value) } -> std::same_as<void>;
            { container.popBack() } -> std::same_as<void>;
            { container.insert(index, value) } -> std::same_as<typename Container::Iterator>;
            { container.erase(index) } -> std::same_as<typename Container::Iterator>;

            // Size operations
            { container.resize(index) } -> std::same_as<void>;
            { container.reserve(index) } -> std::same_as<void>;
            { container.clear() } -> std::same_as<void>;

            // Move operations must exist
                requires std::is_move_assignable_v<Container>&& std::is_move_constructible_v<Container>;
    };
}