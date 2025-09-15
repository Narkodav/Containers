#pragma once
#include "RedBlackTree.h"
#include "AVLTree.h"

#include <concepts>
#include <type_traits>
#include <utility>

namespace Containers {
	template<typename Node, typename T>
	concept NodeType = requires(Node node) {
		{ node.value } -> std::convertible_to<T>;
	};

	template<typename Tree, typename T>
	concept TreeType = requires(Tree tree, const T & value, T && rvalue,
		Tree::Node * node, const Tree::Node * constNode) {
		// Require nested Node type with a value field of type T
		typename Tree::Node;
		requires NodeType<typename Tree::Node, T>;

		Tree();
		std::destructible<Tree>;

		// Core operations
		{ tree.insert(rvalue) } -> std::same_as<typename Tree::Node*>; // rvalue
		{ tree.erase(value) } -> std::same_as<void>;
		{ tree.erase(node) } -> std::same_as<void>;
		{ tree.find(value) } -> std::same_as<typename Tree::Node*>;
		{ tree.size() } -> std::same_as<size_t>;
		{ tree.getRoot() } -> std::same_as<typename Tree::Node*>;
		{ tree.getLeftmost() } -> std::same_as<typename Tree::Node*>;

		// Const versions of operations
		{ std::as_const(tree).find(value) } -> std::same_as<const typename Tree::Node*>;
		{ std::as_const(tree).size() } -> std::same_as<size_t>;
		{ std::as_const(tree).getRoot() } -> std::same_as<const typename Tree::Node*>;
		{ std::as_const(tree).getLeftmost() } -> std::same_as<const typename Tree::Node*>;

		//static functions
		{ Tree::traverseLeft(node) } -> std::same_as<typename Tree::Node*>;
		{ Tree::traverseRight(node) } -> std::same_as<typename Tree::Node*>;
		{ Tree::traverseLeft(constNode) } -> std::same_as<const typename Tree::Node*>;
		{ Tree::traverseRight(constNode) } -> std::same_as<const typename Tree::Node*>;

		// Copy operations if T is copyable
			requires !std::is_copy_constructible_v<T> && !std::is_copy_assignable_v<T> ||
		std::is_copy_constructible_v<Tree> && std::is_copy_assignable_v<Tree>;

		// Move operations must exist
			requires std::is_move_assignable_v<Tree>&& std::is_move_constructible_v<Tree>;
	};
}