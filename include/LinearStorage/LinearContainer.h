#pragma once

namespace Containers {

    //linear storage types

    //Any type that has a operator[]
    template<typename Container, typename ValueType, typename SizeType>
    concept IndexableType = requires(Container container, SizeType index) {
        { container[index] } -> std::same_as<ValueType&>;
        { std::as_const(container)[index] } -> std::same_as<const ValueType&>;
    };

    //Any type that stores size
    template<typename Container, typename SizeType>
    concept SizedContainer = requires(Container container) {
        { container.empty() } -> std::same_as<bool>;
        { container.size() } -> std::same_as<SizeType>;
    };

    //Any type that has iterators
    template<typename Container>
    concept IteratedContainer = requires(Container container) {
        // Iterator type requirements
        typename Container::Iterator;
        typename Container::ConstIterator;

        { container.begin() } -> std::same_as<typename Container::Iterator>;
        { container.end() } -> std::same_as<typename Container::Iterator>;
        { std::as_const(container).begin() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).end() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).cbegin() } -> std::same_as<typename Container::ConstIterator>;
        { std::as_const(container).cend() } -> std::same_as<typename Container::ConstIterator>;
    };


    template<typename Container>
    concept LinearContainerType = requires(Container container,
        typename Container::ValueType value, typename Container::SizeType index) {
        // Type requirements
        typename Container::ValueType;
        typename Container::SizeType;

        requires IndexableType<Container, typename Container::ValueType, 
            typename Container::SizeType>;

        // Iterator operations
        requires IteratedContainer<Container>;

        // Size operations
        requires SizedContainer<Container, typename Container::SizeType>;

        // Iterator requirements
        requires std::contiguous_iterator<typename Container::Iterator>;
        requires std::contiguous_iterator<typename Container::ConstIterator>;
    };

    template<typename Container>
    concept LinearContainerDynamicType = LinearContainerType<Container> && requires(Container container,
        typename Container::ValueType value, typename Container::SizeType index) {

        // Dynamic operations
            { container.pushBack(value) } -> std::same_as<void>;
            { container.popBack() } -> std::same_as<void>;
            { container.insert(container.begin(), value) } -> std::same_as<typename Container::Iterator>;
            { container.erase(container.begin()) } -> std::same_as<typename Container::Iterator>;

            // Size operations
            { container.resize(index) } -> std::same_as<void>;
            { container.reserve(index) } -> std::same_as<void>;
            { container.clear() } -> std::same_as<void>;

            // Move operations must exist
                requires std::is_move_assignable_v<Container>&& std::is_move_constructible_v<Container>;
    };

    template<typename Container>
    concept LinearContainerStaticType = LinearContainerType<Container> && requires {
        { Container{}.size() } -> std::same_as<size_t>;
        requires std::bool_constant<(Container{}.size(), true)>::value;
    };
}