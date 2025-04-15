#pragma once
#include "HashTableChained.h"
#include "HashTableOpenAddress.h"

template<typename HashTable>
concept HashTableType = requires(
    HashTable table, const typename HashTable::Node& node, 
    typename HashTable::ValueType value, size_t capacity) {
    // Type requirements
    typename HashTable::Node;
    typename HashTable::ValueType;

    // Construction/Destruction
    HashTable();
    std::destructible<HashTable>;

    // Node operations
    { HashTable::iterateNext(node) } -> std::same_as<typename HashTable::Node>;
    { table.find(value) } -> std::same_as<typename HashTable::Node>;
    { table.begin() } -> std::same_as<typename HashTable::Node>;
    { table.end() } -> std::same_as<typename HashTable::Node>;

    // Insertion operations
    { table.insert(value) } -> std::same_as<typename HashTable::Node>;

    // Deletion operations
    { table.erase(value) } -> std::same_as<bool>;
    { table.erase(node) } -> std::same_as<void>;

    // Size operations
    { table.empty() } -> std::same_as<bool>;
    { table.size() } -> std::same_as<size_t>;
    { table.loadFactor() } -> std::same_as<float>;
    { table.capacity() } -> std::same_as<size_t>;

    { table.clear(capacity) } -> std::same_as<void>;
    { table.reserve(capacity) } -> std::same_as<void>;

    // Copy operations if ValueType is copyable
        requires !std::is_copy_constructible_v<typename HashTable::ValueType> &&
    !std::is_copy_assignable_v<typename HashTable::ValueType> ||
        std::is_copy_constructible_v<HashTable> && std::is_copy_assignable_v<HashTable>;

    // Move operations must exist
        requires std::is_move_assignable_v<HashTable>&& std::is_move_constructible_v<HashTable>;
};