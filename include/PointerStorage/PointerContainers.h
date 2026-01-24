#pragma once
#include "../Utilities/Concepts.h"
#include "../Utilities/Templates.h"

namespace Containers {

    //Contiguous storage types

    //Any type that has a operator[]
    template<typename Container, typename ValueType, typename SizeType>
    concept IndexableContainer = requires(Container & container, SizeType index) {
        { container[index] } -> std::convertible_to<ValueType&>;
        { std::as_const(container)[index] } -> std::convertible_to<const ValueType&>;
    };

    //Any type that stores size
    template<typename Container, typename SizeType>
    concept SizedContainer = requires(Container & container) {
        { std::as_const(container).empty() } -> std::convertible_to<bool>;
        { std::as_const(container).size() } -> std::convertible_to<SizeType>;
    };

    template<typename Container, typename SizeType>
    concept StaticSizedContainer = requires(Container & container) {
        { Container::size() } -> std::convertible_to<SizeType>;
    };

    //Any type that has iterators
    template<typename Container>
    concept IteratedContainer = requires(Container& container) {
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

    template<typename Container, typename ValueType, typename SizeType>
    concept GrowableContainer = requires(Container & c, ValueType & value, SizeType index) {
        { c.pushBack(value) } -> std::same_as<void>;
        { c.popBack() } -> std::same_as<void>;
        { c.resize(index) } -> std::same_as<void>;
        { c.clear() } -> std::same_as<void>;
    };

    template<typename Container, typename ValueType, typename SizeType>
    concept DynamicContainer = requires(Container& c, ValueType& value, SizeType index) {
        { c.reserve(index) } -> std::same_as<void>;
        { c.capacity() } -> std::same_as<SizeType>;
    };

    template<typename Container>
    concept PointerContainerType = requires {
        typename Container::ValueType;
        typename Container::SizeType;
    } && IteratedContainer<Container> &&
        IndexableContainer<Container, typename Container::ValueType, typename Container::SizeType>&&
        PointerIteratorType<typename Container::Iterator, typename Container::ValueType>&&
        PointerIteratorType<typename Container::ConstIterator, const typename Container::ValueType>;

    template<typename Container>
    concept ArrayContainerType = StaticSizedContainer<Container, typename Container::SizeType> && PointerContainerType<Container>;

    template<typename Container>
    concept VectorContainerType = PointerContainerType<Container> && 
        GrowableContainer<Container, typename Container::ValueType, typename Container::SizeType> &&
		DynamicContainer<Container, typename Container::ValueType, typename Container::SizeType>;

    template<typename Container>
    concept SizableContainerType = (StaticSizedContainer<Container, typename Container::SizeType> ||
        SizedContainer<Container, typename Container::SizeType>) &&
        IndexableContainer<Container, typename Container::ValueType, typename Container::SizeType>;
}