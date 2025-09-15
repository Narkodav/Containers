#pragma once
#include <cstdint>
#include <cassert>
#include <string>
#include <iostream>
#include <type_traits>
#include <stack>

namespace Containers {
	template <typename T, typename Comparator>
	class RedBlackTree
	{
	public:
		enum class Color : bool
		{
			Black = 0,
			Red = 1,
		};

		enum class Direction : uint8_t
		{
			Left = 0,
			Right = 1,
			Root = 2,
			Num,
		};

		struct Node
		{
			Color color;
			Direction dir;
			T value;
			Node* parent;
			Node* children[2] = { nullptr, nullptr };

			Node(Color color, Direction dir, const T& value, Node* parent, Node* childLeft, Node* childRight) :
				color{ color }, dir{ dir }, value{ value }, parent{ parent }
			{
				this->children[static_cast<size_t>(Direction::Left)] = childLeft;
				this->children[static_cast<size_t>(Direction::Right)] = childRight;
			}

			Node(Color color, Direction dir, T&& value, Node* parent, Node* childLeft, Node* childRight) :
				color{ color }, dir{ dir }, value{ std::move(value) }, parent{ parent }
			{
				this->children[static_cast<size_t>(Direction::Left)] = childLeft;
				this->children[static_cast<size_t>(Direction::Right)] = childRight;
			}

			Node(Color color, Direction dir, Node* parent, Node* childLeft, Node* childRight) :
				color{ color }, dir{ dir }, parent{ parent }
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

			void copyLayout(Node* other) //doesn't copy color
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

		RedBlackTree() = default;

		RedBlackTree(const RedBlackTree& other)
			requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T> : m_size(other.m_size)
		{
			copyFromRoot(other.m_root);
		}

		RedBlackTree& operator=(const RedBlackTree& other)
			requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
		{
			if (this != &other)
			{
				m_size = other.m_size;
				copyFromRoot(other.m_root);
			}
			return *this;
		}

		RedBlackTree(RedBlackTree&& other)
		{
			m_size = std::exchange(other.m_size, 0);
			m_root = std::exchange(other.m_root, nullptr);
		};

		RedBlackTree& operator=(RedBlackTree&& other)
		{
			if (this != &other)
			{
				m_size = std::exchange(other.m_size, 0);
				m_root = std::exchange(other.m_root, nullptr);
			}
			return *this;
		};

		~RedBlackTree()
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

		Node* getLeftmost()
		{
			return const_cast<Node*>(std::as_const(*this).getLeftmost());
		}

		const Node* getLeftmost() const
		{
			if (m_root == nullptr)
				return nullptr;
			Node* node = m_root;
			while (node->getChild(Direction::Left) != nullptr)
				node = node->getChild(Direction::Left);
			return node;
		}

		static const Node* traverseRight(const Node* node)
		{
			if (node == nullptr)
				return nullptr;

			if (node->getChild(Direction::Right) != nullptr) {
				// If there's a right child, go right once and then
				// as far left as possible
				const Node* current = node->getChild(Direction::Right);
				while (current->getChild(Direction::Left) != nullptr)
					current = current->getChild(Direction::Left);
				return current;
			}
			else {
				// Otherwise, go up until we find a parent where we came from the left
				const Node* current = node;
				const Node* parent = node->parent;
				while (parent != nullptr && current->dir == Direction::Right) {
					current = parent;
					parent = parent->parent;
				}
				return parent;
			}
		}

		static const Node* traverseLeft(const Node* node)
		{
			if (node == nullptr)
				return nullptr;

			if (node->getChild(Direction::Left) != nullptr) {
				// If there's a left child, go left once and then
				// as far right as possible
				const Node* current = node->getChild(Direction::Left);
				while (current->getChild(Direction::Right) != nullptr)
					current = current->getChild(Direction::Right);
				return current;
			}
			else {
				// Otherwise, go up until we find a parent where we came from the right
				const Node* current = node;
				const Node* parent = node->parent;
				while (parent != nullptr && current->dir == Direction::Left) {
					current = parent;
					parent = parent->parent;
				}
				return parent;
			}
		}

		static Node* traverseRight(Node* node)
		{
			return const_cast<Node*>(traverseRight(static_cast<const Node*>(node)));
		}

		static Node* traverseLeft(Node* node)
		{
			return const_cast<Node*>(traverseLeft(static_cast<const Node*>(node)));
		}

		Node* find(const T& value)
		{
			return const_cast<Node*>(std::as_const(*this).find(value, const_cast<const Node*>(m_root)));
		}

		Node* find(const T& value, Node* root)
		{
			return const_cast<Node*>(std::as_const(*this).find(value, const_cast<const Node*>(root)));
		}

		const Node* find(const T& value) const
		{
			return find(value, m_root);
		}

		const Node* find(const T& value, const Node* root) const
		{
			const Node* node = root;
			while (node != nullptr)
			{
				if (Comparator()(node->value, value))
					node = node->getChild(Direction::Right);
				else if (Comparator()(value, node->value))
					node = node->getChild(Direction::Left);
				else return node;
			}
			return nullptr;
		}

		template<typename U>
		Node* insert(U&& value)
			requires (std::is_same_v<std::decay_t<U>, T> &&
		(std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T> ||
			std::is_move_constructible_v<T> || std::is_move_assignable_v<T>))
		{
			Node* node = m_root;
			Node* parent = nullptr;
			Direction dir = Direction::Root;
			while (node != nullptr)
			{
				parent = node;
				if (Comparator()(node->value, value))
				{
					node = node->getChild(Direction::Right);
					dir = Direction::Right;
				}
				else if (Comparator()(value, node->value))
				{
					node = node->getChild(Direction::Left);
					dir = Direction::Left;
				}
				else
				{
					if constexpr (std::is_rvalue_reference_v<U&&> && std::is_move_assignable_v<T>)
						node->value = std::move(value);
					else if constexpr (std::is_copy_assignable_v<T>)
						node->value = value;
					return node;
				}
			}

			if constexpr (std::is_rvalue_reference_v<U&&>)
			{
				if constexpr (std::is_move_constructible_v<T>)
					node = new Node(Color::Red, dir, std::move(value), parent, nullptr, nullptr);
				else
				{
					node = new Node(Color::Red, dir, parent, nullptr, nullptr);
					node->value = std::move(value);
				}
			}
			else
			{
				if constexpr (std::is_copy_constructible_v<T>)
					node = new Node(Color::Red, dir, value, parent, nullptr, nullptr);
				else
				{
					node = new Node(Color::Red, dir, parent, nullptr, nullptr);
					node->value = value;
				}
			}

			if (parent != nullptr)
			{
				parent->getChild(dir) = node;
				insertFixup(node);
			}
			else
			{
				m_root = node;
				m_root->color = Color::Black;
			}
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
			m_size--;
			Color originalColor = node->color;

			if (node->getChild(Direction::Left) == nullptr) //right might be nil
			{
				Node* successor = node->getChild(Direction::Right);
				Node* parent = node->parent;
				if (parent != nullptr)
					parent->getChild(node->dir) = successor;
				else m_root = successor;

				if (successor != nullptr)
				{
					successor->dir = node->dir;
					successor->parent = node->parent;
				}

				if (originalColor == Color::Black)
					eraseFixup(successor, node->dir, parent);
			}
			else if (node->getChild(Direction::Right) == nullptr)
			{
				Node* successor = node->getChild(Direction::Left);
				Node* parent = node->parent;
				if (parent != nullptr)
					parent->getChild(node->dir) = successor;
				else m_root = successor;

				//no need to copy children, successor is the only child
				successor->dir = node->dir;
				successor->parent = node->parent;

				if (originalColor == Color::Black)
					eraseFixup(successor, successor->dir, parent);
			}
			else
			{
				Node* nodeRightChild = node->getChild(Direction::Right);
				Node* nodeLeftChild = node->getChild(Direction::Left);
				Node* successor = nodeRightChild;

				// Find successor (smallest value in right subtree)
				if (successor->getChild(Direction::Left) == nullptr)
				{
					if (node->parent != nullptr)
						node->parent->getChild(node->dir) = successor;
					else m_root = successor;

					successor->getChild(Direction::Left) = nodeLeftChild;
					nodeLeftChild->parent = successor;
					successor->dir = node->dir;
					successor->parent = node->parent;
					auto successorOriginalColor = successor->color;
					successor->color = originalColor;

					if (successorOriginalColor == Color::Black)
						eraseFixup(successor->getChild(Direction::Right), Direction::Right, successor);
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

					if (node->parent != nullptr)
						node->parent->getChild(node->dir) = successor;
					else m_root = successor;

					successor->copyLayout(node);
					auto successorOriginalColor = successor->color;
					successor->color = originalColor;

					nodeRightChild->parent = successor;
					nodeLeftChild->parent = successor;

					if (successorOriginalColor == Color::Black)
						eraseFixup(successorRightChild, Direction::Left, successorParent);
				}
			}
			delete node;
		}

		bool validateRedBlackProperties() const
		{
			if (m_root == nullptr)
				return true;

			// Property 1: Root must be black
			if (m_root->color != Color::Black) {
				return false;
			}

			// Track black height and validate all properties recursively
			int blackHeight = 0;
			return validateNode(m_root, blackHeight, findBlackHeight());
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
					other->color,
					other->dir,
					other->value,
					nullptr,
					nullptr, nullptr
				);
			else if (std::is_copy_assignable_v<T>)
			{
				Node* node = new Node(
					other->color,
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
				<< "(" << (node->color == Color::Red ? "Red," : "Black,")
				<< getDirString(node->dir) << ")\n";
			else std::cout << "(" << (node->color == Color::Red ? "Red," : "Black,")
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

		//assumes the red black properties are met
		size_t findBlackHeight() const
		{
			size_t counter = 0;
			Node* node = m_root;
			while (node != nullptr)
			{
				if (node->color == Color::Black)
					counter++;
				node = node->getChild(Direction::Left);
			}

			return counter;
		}

		bool validateNode(Node* node, int blackHeight, int hightActual, Node* parent = nullptr) const
		{
			if (node->color == Color::Black)
				blackHeight++;

			// Verify parent pointer is correct
			if (node->parent != parent) {
				//////printTree();
				__debugbreak();
				assert(false && "Property violation: Invalid parent pointer");
				return false;
			}

			// Property 2: Check for consecutive red nodes
			if (node->color == Color::Red && parent && parent->color == Color::Red) {
				//printTree();
				__debugbreak();
				assert(false && "Property violation: Two consecutive red nodes");
				return false;
			}

			// If this is a leaf node, verify black height
			if (node->getChild(Direction::Left) == nullptr ||
				node->getChild(Direction::Right) == nullptr) {
				if (blackHeight != hightActual) {
					//printTree();
					__debugbreak();
					assert(false && "Property violation: Inconsistent black height");
					return false;
				}
			}

			// Verify binary search tree property
			if (node->getChild(Direction::Left) != nullptr &&
				!Comparator()(node->getChild(Direction::Left)->value, node->value)) {
				//printTree();
				__debugbreak();
				assert(false && "Property violation: BST property - left child");
				return false;
			}

			if (node->getChild(Direction::Right) != nullptr &&
				!Comparator()(node->value, node->getChild(Direction::Right)->value)) {
				//printTree();
				__debugbreak();
				assert(false && "Property violation: BST property - right child");
				return false;
			}

			// Recursively validate children
			bool result = true;
			if (node->getChild(Direction::Left) != nullptr)
				result &= validateNode(node->getChild(Direction::Left), blackHeight, hightActual, node);
			if (node->getChild(Direction::Right) != nullptr)
				result &= validateNode(node->getChild(Direction::Right), blackHeight, hightActual, node);
			return result;
		}

		void insertFixup(Node* node)
		{
			while (node->parent && node->parent->color == Color::Red)
			{
				Direction uncleDir = static_cast<Direction>(1 - static_cast<size_t>(node->parent->dir));
				Node* parent = node->parent;
				Node* grandparent = parent->parent;
				Node* uncle = grandparent ? grandparent->getChild(uncleDir) : nullptr;

				if (uncle && uncle->color == Color::Red)
				{
					// Case 1: Uncle is red
					parent->color = Color::Black;
					uncle->color = Color::Black;
					grandparent->color = Color::Red;
					node = grandparent;
				}
				else
				{
					// Case 2 & 3: Uncle is black (or null)
					if (node->dir != parent->dir)
					{
						// Case 2: Node and parent are on opposite sides
						// switch the roles of parent and child to turn it into case 3
						node = parent;
						rotate(node, node->dir);
						parent = node->parent;
					}

					// Case 3: Node and parent are on same side
					parent->color = Color::Black;
					grandparent->color = Color::Red;
					rotate(grandparent, static_cast<Direction>(1 - static_cast<size_t>(parent->dir)));
				}
			}

			// Ensure root is black
			m_root->color = Color::Black;
		}

		void eraseFixup(Node* node, Direction nodeDir, Node* parent) //node might be nil
		{
			while ((!node || node->color == Color::Black) && node != m_root)
			{
				Direction opposite = static_cast<Direction>(1 - static_cast<size_t>(nodeDir));
				Node* sibling = parent->getChild(opposite);

				if (!sibling) // If sibling is null, we can't recolor or rotate
					break;

				// Case 1: Sibling is red
				if (sibling->color == Color::Red)
				{
					sibling->color = Color::Black;
					parent->color = Color::Red;
					rotate(parent, nodeDir);
					sibling = parent->getChild(opposite); // Update sibling after rotation
					if (!sibling) // Check if new sibling exists
						break;
				}

				// Case 2: Sibling is black and both its children are black (or null)
				Node* innerChild = sibling->getChild(nodeDir);
				Node* outerChild = sibling->getChild(opposite);

				bool innerChildBlack = !sibling->getChild(nodeDir) ||
					sibling->getChild(nodeDir)->color == Color::Black;
				bool outerChildBlack = !sibling->getChild(opposite) ||
					sibling->getChild(opposite)->color == Color::Black;

				if (innerChildBlack && outerChildBlack)
				{
					sibling->color = Color::Red;
					node = parent; //parent must exist since node cant be the root
					parent = node->parent;
					nodeDir = node->dir;
					if (!parent || node->color == Color::Red)
						break;
					continue;
				}

				// Case 3: Sibling is black, inner child is red, outer child is black
				if (outerChildBlack) //inner child must be red
				{
					sibling->color = Color::Red;
					innerChild->color = Color::Black;
					rotate(sibling, opposite);
					sibling = parent->getChild(opposite);
					outerChild = sibling->getChild(opposite);
					if (!sibling) // Check if new sibling exists
						break;
				}

				// Case 4: Sibling is black, outer child is red
				sibling->color = parent->color;
				parent->color = Color::Black;
				outerChild->color = Color::Black; //outer child must exist since its red

				rotate(parent, nodeDir);
				node = m_root; // This will break the loop
			}

			if (node)
				node->color = Color::Black;
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

}