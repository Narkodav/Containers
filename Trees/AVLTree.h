#pragma once
#include <cstdint>
#include <cassert>
#include <string>
#include <iostream>
#include <type_traits>
#include <stack>
#include <array>

template <typename T>
class AVLTree
{
public:
	static inline const std::array<char, 2> directionMults = { -1, 1 };

	enum class Direction : uint8_t
	{
		Left = 0,
		Right = 1,
		Root = 2,
		Num,
	};

	struct Node
	{
		int8_t balance;
		Direction dir;
		T value;
		Node* parent;
		Node* children[2] = { nullptr, nullptr };

		Node(int8_t balance, Direction dir, const T& value, Node* parent, Node* childLeft, Node* childRight) :
			balance{ balance }, dir{ dir }, value{ value }, parent{ parent }
		{
			this->children[static_cast<size_t>(Direction::Left)] = childLeft;
			this->children[static_cast<size_t>(Direction::Right)] = childRight;
		}

		Node(int8_t balance, Direction dir, T&& value, Node* parent, Node* childLeft, Node* childRight) :
			balance{ balance }, dir{ dir }, value{ std::move(value) }, parent{ parent }
		{
			this->children[static_cast<size_t>(Direction::Left)] = childLeft;
			this->children[static_cast<size_t>(Direction::Right)] = childRight;
		}

		Node(int8_t balance, Direction dir, Node* parent, Node* childLeft, Node* childRight) :
			balance{ balance }, dir{ dir }, parent{ parent }
		{
			this->children[static_cast<size_t>(Direction::Left)] = childLeft;
			this->children[static_cast<size_t>(Direction::Right)] = childRight;
		}

		// Only enable copy operations if T is copyable
		Node(const Node&) requires std::is_copy_constructible_v<T> = default;
		Node& operator=(const Node&) requires std::is_copy_assignable_v<T> = default;

		// Only enable move operations if T is moveable
		Node(Node&&) noexcept requires std::is_move_constructible_v<T> = default;
		Node& operator=(Node&&) noexcept requires std::is_move_assignable_v<T> = default;

		inline Node*& getChild(Direction direction)
		{
			return children[static_cast<size_t>(direction)];
		}

		inline const Node* getChild(Direction direction) const
		{
			return children[static_cast<size_t>(direction)];
		}

		void copyLayout(Node* other)
		{
			dir = other->dir;
			parent = other->parent;
			children[static_cast<size_t>(Direction::Left)] = other->getChild(Direction::Left);
			children[static_cast<size_t>(Direction::Right)] = other->getChild(Direction::Right);
		}
	};

private:

	Node* m_root = nullptr;
	size_t m_size = 0;
public:

	AVLTree() = default;

	AVLTree(const AVLTree& other)
		requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T> : m_size(other.m_size)
	{
		copyFromRoot(other.m_root);
	}

	AVLTree& operator=(const AVLTree& other)
		requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
	{
		if (this != &other)
		{
			m_size = other.m_size;
			copyFromRoot(other.m_root);
		}
		return *this;
	}

	AVLTree(AVLTree&& other)
	{
		m_size = std::exchange(other.m_size, 0);
		m_root = std::exchange(other.m_root, nullptr);
	};

	AVLTree& operator=(AVLTree&& other)
	{
		if (this != &other)
		{
			m_size = std::exchange(other.m_size, 0);
			m_root = std::exchange(other.m_root, nullptr);
		}
		return *this;
	};

	~AVLTree()
	{
		clear();
	}

	void clear() {
		Node* current = m_root;
		while (current != nullptr) {
			if (current->getChild(Direction::Left) != nullptr) {
				// Go to leftmost node
				current = current->getChild(Direction::Left);
			}
			else if (current->getChild(Direction::Right) != nullptr) {
				// No left child, go right
				current = current->getChild(Direction::Right);
			}
			else {
				// Leaf node, delete it and go up
				Node* parent = current->parent;
				if (parent != nullptr) {
					parent->getChild(current->dir) = nullptr;
				}
				delete current;
				current = parent;
			}
		}
		m_root = nullptr;
		m_size = 0;
	}

	size_t size() const
	{
		return m_size;
	}

	Node* getRoot()
	{
		return m_root;
	}

	const Node* getRoot() const
	{
		return m_root;
	}

	Node* find(const T& value)
	{
		return find(value, m_root);
	}

	Node* find(const T& value, Node* root)
	{
		Node* node = root;
		while (node != nullptr)
		{
			if (node->value == value)
				return node;
			else if (node->value < value)
				node = node->getChild(Direction::Right);
			else node = node->getChild(Direction::Left);
		}
		return nullptr;
	}

	const Node* find(const T& value) const
	{
		return find(value, m_root);
	}

	const Node* find(const T& value, Node* root) const
	{
		Node* node = root;
		while (node != nullptr)
		{
			if (node->value == value)
				return node;
			else if (node->value < value)
				node = node->getChild(Direction::Right);
			else node = node->getChild(Direction::Left);
		}
		return nullptr;
	}

	template<typename U>
	Node* insert(U&& value)
		requires (std::is_same_v<std::decay_t<U>, T> &&
	(std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T> ||
		std::is_move_constructible_v<T> || std::is_move_assignable_v<T>))
	{
		size_t height = 0;
		Node* node = m_root;
		Node* parent = nullptr;
		Direction dir = Direction::Root;
		while (node != nullptr)
		{
			parent = node;
			if (node->value == value)
			{
				if constexpr (std::is_rvalue_reference_v<U&&> && std::is_move_assignable_v<T>)
					node->value = std::move(value);
				else if constexpr (std::is_copy_assignable_v<T>)
					node->value = value;

				while (node != m_root)
				{
					node->parent->balance = node->dir == Direction::Right ?
						node->parent->balance - 1 : node->parent->balance + 1;
					node = node->parent;
				}
				return node;
			}
			else if (node->value < value)
			{
				node->balance++;
				node = node->getChild(Direction::Right);
				dir = Direction::Right;
			}
			else
			{
				node->balance--;
				node = node->getChild(Direction::Left);
				dir = Direction::Left;
			}
		}

		if constexpr (std::is_rvalue_reference_v<U&&>)
		{
			if constexpr (std::is_move_constructible_v<T>)
				node = new Node(0, dir, std::move(value), parent, nullptr, nullptr);
			else
			{
				node = new Node(0, dir, parent, nullptr, nullptr);
				node->value = std::move(value);
			}
		}
		else
		{
			if constexpr (std::is_copy_constructible_v<T>)
				node = new Node(0, dir, value, parent, nullptr, nullptr);
			else
			{
				node = new Node(0, dir, parent, nullptr, nullptr);
				node->value = value;
			}
		}

		if (parent != nullptr)
		{
			parent->getChild(dir) = node;
			insertFixup(node);
		}
		else m_root = node;
		m_size++;

		return node;
	}

	void erase(const T& value)
	{
		Node* node = find(value, m_root);
		if (node == nullptr)
			return;
		erase(node);
	}

	void erase(Node* node)
	{
		Node* parent = node->parent;
		Direction dir = node->dir;

		// Standard BST deletion logic here
		if (node->getChild(Direction::Left) == nullptr) {
			auto* successor = node->getChild(Direction::Right);
			if (successor != nullptr) {
				successor->parent = parent;
				successor->dir = node->dir;
				successor->balance = node->balance /*- directionMults[static_cast<size_t>(Direction::Right)]*/;
			}

			if (parent)
			{
				parent->getChild(node->dir) = successor;
				eraseFixup(parent, Direction::Right);
			}
			else m_root = successor;
		}
		else if (node->getChild(Direction::Right) == nullptr) {
			auto* successor = node->getChild(Direction::Left);
			successor->parent = parent;
			successor->dir = node->dir;
			successor->balance = node->balance /*- directionMults[static_cast<size_t>(Direction::Right)]*/;

			if (parent)
			{
				parent->getChild(node->dir) = successor;
				eraseFixup(parent, Direction::Left);
			}
			else m_root = successor;				
		}
		else {
			Node* nodeRightChild = node->getChild(Direction::Right);
			Node* nodeLeftChild = node->getChild(Direction::Left);
			Node* successor = nodeRightChild;

			// Find successor (smallest value in right subtree)
			if (successor->getChild(Direction::Left) == nullptr)
			{
				successor->getChild(Direction::Left) = nodeLeftChild;
				nodeLeftChild->parent = successor;
				successor->dir = node->dir;
				successor->parent = parent;
				successor->balance = node->balance;

				if (parent != nullptr)
				{
					parent->getChild(node->dir) = successor;
					eraseFixup(parent, Direction::Right);
				}
				else m_root = successor;
			}
			else
			{
				do
				{
					successor = successor->getChild(Direction::Left);
				} while (successor->getChild(Direction::Left));

				Node* successorRightChild = successor->getChild(Direction::Right);
				Node* successorParent = successor->parent;

				successorParent->getChild(successor->dir) = successorRightChild;
				if (successorRightChild != nullptr)
				{
					successorRightChild->parent = successorParent;
					successorRightChild->dir = successor->dir;
				}

				if (parent != nullptr)
					parent->getChild(node->dir) = successor;
				else m_root = successor;

				successor->copyLayout(node);
				successor->balance = node->balance;

				nodeRightChild->parent = successor;
				nodeLeftChild->parent = successor;
				// Rebalance from parent upwards
				eraseFixup(successorParent, Direction::Left);					
			}
		}

		delete node;
		m_size--;
	}
		
	void printTree() const
	{
		if (m_root == nullptr) {
			std::cout << "<empty tree>\n";
			return;
		}
		printNode(m_root, "", true);
	}

private:

	Node* getNodeCopy(const Node* other)
	{
		if constexpr (std::is_copy_constructible_v<T>)
			return new Node(
				other->height,
				other->dir,
				other->value,
				nullptr,
				nullptr, nullptr
			);
		else if (std::is_copy_assignable_v<T>)
		{
			Node* node = new Node(
				other->height,
				other->dir,
				nullptr,
				nullptr, nullptr
			);
			node->value = other->value;
			return node;
		}
		else static_assert(std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>,
			"T must be copyable or assignable");
	}

	void copyFromRoot(const Node* otherRoot)
	{
		if (otherRoot == nullptr) {
			m_root = nullptr;
			return;
		}

		// Create root node
		m_root = getNodeCopy(otherRoot);

		// Stack holds pairs of (source node, destination node)
		std::stack<std::pair<const Node*, Node*>> nodeStack;
		nodeStack.push({ otherRoot, m_root });

		while (!nodeStack.empty()) {
			auto [srcNode, destNode] = nodeStack.top();
			nodeStack.pop();

			// Copy left child
			if (srcNode->getChild(Direction::Left) != nullptr) {
				destNode->getChild(Direction::Left) = getNodeCopy(srcNode->getChild(Direction::Left));
				destNode->getChild(Direction::Left)->parent = destNode;
				nodeStack.push({
					srcNode->getChild(Direction::Left),
					destNode->getChild(Direction::Left)
					});
			}

			// Copy right child
			if (srcNode->getChild(Direction::Right) != nullptr) {
				destNode->getChild(Direction::Right) = getNodeCopy(srcNode->getChild(Direction::Right));
				destNode->getChild(Direction::Right)->parent = destNode;
				nodeStack.push({
					srcNode->getChild(Direction::Right),
					destNode->getChild(Direction::Right)
					});
			}
		}
	}

	template<typename S, typename T>
	class is_streamable {
		template<typename SS, typename TT>
		static auto test(int)
			-> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());

		template<typename, typename>
		static auto test(...) -> std::false_type;

	public:
		static const bool value = decltype(test<S, T>(0))::value;
	};

	void printNode(const Node* node, std::string prefix, bool isRoot) const
	{
		if (node == nullptr)
			return;

		// Print current node
		if (!isRoot)
			std::cout << prefix << "| ";

		// Print value and color
		printValue(node);

		// Prepare prefix for children
		std::string childPrefix = prefix;
		if (!isRoot)
			childPrefix += "| ";
		else
			childPrefix += "  ";

		// Print right child first (will appear lower in the output)
		if (node->getChild(Direction::Right))
			printNode(node->getChild(Direction::Right), childPrefix, false);

		// Print left child
		if (node->getChild(Direction::Left))
			printNode(node->getChild(Direction::Left), childPrefix, false);
	}

	void printValue(const Node* node) const
	{
		if constexpr (is_streamable<std::ostream, T>::value)
			std::cout << node->value << "\t"
			<< "(" << (int)node->balance << ","
			<< getDirString(node->dir) << ")\n";
		else std::cout << "(" << (int)node->balance << ","
			<< getDirString(node->dir) << ")\n";
	}

	std::string getDirString(Direction dir) const
	{
		switch (static_cast<size_t>(dir))
		{
		case static_cast<size_t>(Direction::Left):
			return "Left";
		case static_cast<size_t>(Direction::Right):
			return "Right";
		default:
			return "Root";
		}
	}

	void insertFixup(Node* node)
	{
		while (node != m_root && node->parent != 0)
		{
			Node* parent = node->parent;
			Direction dir = node->dir;
			Direction dirOpposite = static_cast<Direction>(1 - static_cast<size_t>(dir));

			// Simple case - no rotation needed yet
			if ((dir == Direction::Left && parent->balance == -1) ||
				(dir == Direction::Right && parent->balance == 1))
			{
				node = parent;
				continue;
			}

			if (node->balance == directionMults[static_cast<size_t>(dir)]) // Single right or left rotation
			{
				rotate(parent, dirOpposite);
				parent->balance = 0;
				node->balance = 0;
			}
			else // Double rotation (left - right, right - left)
			{
				Node* grandChild = node->getChild(dirOpposite);
				rotate(node, dir);
				rotate(parent, dirOpposite);

				// Update balance factors based on grandChild's balance
				if (grandChild->balance == directionMults[static_cast<size_t>(dir)])
				{
					parent->balance = -directionMults[static_cast<size_t>(dir)];
					node->balance = 0;
				}
				else if (grandChild->balance == -directionMults[static_cast<size_t>(dir)])
				{
					parent->balance = 0;
					node->balance = directionMults[static_cast<size_t>(dir)];
				}
				else // grandChild->balance == 0
				{
					parent->balance = 0;
					node->balance = 0;
				}
				grandChild->balance = 0;
			}
			break; // After rotation, tree is balanced
		}
	}

	void eraseFixup(Node* node, Direction deletedDirection)
	{
		while (node != nullptr)
		{
			// Update balance factor
			node->balance -= directionMults[static_cast<size_t>(deletedDirection)];

			// Check if tree is balanced at this node
			if (node->balance <= 1 && node->balance >= -1) {
				deletedDirection = node->dir;
				node = node->parent;
				continue;
			}

			if (node->balance > 1)
			{
				Node* child = node->getChild(Direction::Right);
				if (child->balance >= 0)
				{
					rotate(node, Direction::Left);
					if (child->balance == 0) {  // Special case for deletion
						node->balance = 1;
						child->balance = -1;
					}
					else {  // Normal case
						node->balance = 0;
						child->balance = 0;
					}
				}
				else
				{
					Node* grandChild = child->getChild(Direction::Left);
					rotate(child, Direction::Right);
					rotate(node, Direction::Left);

					// Update balance factors based on grandChild's original balance
					if (grandChild->balance == 1) {
						node->balance = -1;
						child->balance = 0;
					}
					else if (grandChild->balance == -1) {
						node->balance = 0;
						child->balance = 1;
					}
					else {  // grandChild->balance == 0
						node->balance = 0;
						child->balance = 0;
					}
					grandChild->balance = 0;
				}
			}
			else if (node->balance < -1)
			{
				Node* child = node->getChild(Direction::Left);
				if (child->balance <= 0)
				{
					rotate(node, Direction::Right);
					if (child->balance == 0) {  // Special case for deletion
						node->balance = -1;
						child->balance = 1;
					}
					else {  // Normal case
						node->balance = 0;
						child->balance = 0;
					}
				}
				else
				{
					Node* grandChild = child->getChild(Direction::Right);
					rotate(child, Direction::Left);
					rotate(node, Direction::Right);

					// Update balance factors based on grandChild's original balance
					if (grandChild->balance == -1) {
						node->balance = 1;
						child->balance = 0;
					}
					else if (grandChild->balance == 1) {
						node->balance = 0;
						child->balance = -1;
					}
					else {  // grandChild->balance == 0
						node->balance = 0;
						child->balance = 0;
					}
					grandChild->balance = 0;
				}
			}

			deletedDirection = node->dir;
			node = node->parent;
		}		
	}

	void rotate(Node* node, Direction dir)
	{
		Direction otherDir = static_cast<Direction>(1 - static_cast<size_t>(dir));
		Node* child = node->getChild(otherDir);
		Node* subtree = child->getChild(dir);
		Node* parent = node->parent;

		if (parent != nullptr)
		{
			parent->getChild(node->dir) = child;
			child->dir = node->dir;
		}
		else
		{
			child->dir = Direction::Root;
			m_root = child;
		}

		child->parent = parent;

		child->getChild(dir) = node;
		node->parent = child;
		node->dir = dir;

		node->getChild(otherDir) = subtree;

		if (subtree != nullptr)
		{
			subtree->dir = otherDir;
			subtree->parent = node;
		}
	}
};

