#pragma once
#include "ListOneSided.h"
#include "ListDoubleSided.h"
#include "ListDoubleSidedTailed.h"

template<typename List>
concept ListType = requires(List list, typename List::Node * node, typename List::ValueType value) {
    // Type requirements
    typename List::Node;
    typename List::ValueType;

    // Construction/Destruction
    List();
    std::destructible<List>;

    // Node operations
    { list.getFront() } -> std::same_as<typename List::Node*>;
    { List::iterateNext(node) } -> std::same_as<typename List::Node*>;

    // Insertion operations
    { list.insertFront(value) } -> std::same_as<void>;
    { list.insertBack(value) } -> std::same_as<void>;

    // Deletion operations
    { list.deleteFront() } -> std::same_as<void>;
    { list.deleteBack() } -> std::same_as<void>;
    { list.deleteNode(node) } -> std::same_as<void>;

    // Size operations
    { list.empty() } -> std::same_as<bool>;

    { list.clear() } -> std::same_as<void>;

    // Copy operations if ValueType is copyable
        requires !std::is_copy_constructible_v<typename List::ValueType> && 
    !std::is_copy_assignable_v<typename List::ValueType> ||
        std::is_copy_constructible_v<List> && std::is_copy_assignable_v<List>;

    // Move operations must exist
        requires std::is_move_assignable_v<List>&& std::is_move_constructible_v<List>;
};

template<typename List>
concept ListDoubleSidedType = ListType<List> && requires(List list, typename List::Node * node) {
    { List::iteratePrevious(node) } -> std::same_as<typename List::Node*>;
};

template<typename List>
concept ListDoubleSidedTailedType = ListDoubleSidedType<List> && requires(List list, typename List::Node * node) {
    { list.getBack() } -> std::same_as<typename List::Node*>;
};