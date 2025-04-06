#pragma once
#include <concepts>
#include <type_traits>
#include <utility>

template<typename Node, typename T>
concept NodeType = requires(Node node) {
	{ node.value } -> std::convertible_to<T>;
};

template<typename Tree, typename T>
concept TreeType = requires(Tree tree, const T & value, T && rvalue, Tree::Node * node) {
	// Require nested Node type with a value field of type T
	typename Tree::Node;
	requires NodeType<typename Tree::Node, T>;

	// Core operations
	{ tree.insert(rvalue) } -> std::same_as<typename Tree::Node*>; // rvalue
	{ tree.erase(value) } -> std::same_as<void>;
	{ tree.erase(node) } -> std::same_as<void>;
	{ tree.find(value) } -> std::same_as<typename Tree::Node*>;
	{ tree.size() } -> std::same_as<size_t>;

	// Const versions of operations
	{ std::as_const(tree).find(value) } -> std::same_as<const typename Tree::Node*>;
	{ std::as_const(tree).size() } -> std::same_as<size_t>;

	// Copy operations if T is copyable
		requires !std::is_copy_constructible_v<T> && !std::is_copy_assignable_v<T> ||
	std::is_copy_constructible_v<Tree> && std::is_copy_assignable_v<Tree>;

	// Move operations must exist
		requires std::is_move_assignable_v<Tree>&& std::is_move_constructible_v<Tree>;
};