#pragma once
#include <type_traits>
#include <utility>
#include <concepts>
#include <stdexcept>

namespace Containers {

    template<typename T>
    struct NodeDoubleSided {
        T m_data;
        NodeDoubleSided* m_next, m_previous;
    };

    template<typename U>
    class BidirectionalListIteratorBase
    {
		using Node = NodeDoubleSided<U>;
    private:
        Node* m_node = nullptr;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = U;
        using difference_type = std::ptrdiff_t;
        using pointer = U*;
        using reference = U&;
        BidirectionalListIteratorBase() = default;
        BidirectionalListIteratorBase(Node* node) : Node(*node) {};

        reference operator*() {
            return m_node->m_data;
        };
        pointer operator->() {
            return &(m_node->m_data);
        }
        // Increment operator (in-order traversal)
        BidirectionalListIteratorBase& operator++() {
            Node* next = m_node->m_next;
            CONTAINERS_VERIFY(next != nullptr, "Iterator cannot be incremented past the end");
            m_node = next;
            return *this;
        }
        BidirectionalListIteratorBase operator++(int) {
            BidirectionalListIteratorBase tmp = *this;
            Node* next = m_node->m_next;
            CONTAINERS_VERIFY(next != nullptr, "Iterator cannot be incremented past the end");
            m_node = next;
            return tmp;
        }
        // Decrement operator (in-order traversal)
        BidirectionalListIteratorBase& operator--() {
            Node* previous = m_node->m_previous;
            CONTAINERS_VERIFY(previous != nullptr, "Iterator cannot be decremented past the beginning");
            m_node = previous;
            return *this;
        }

        BidirectionalListIteratorBase operator--(int) {
            BidirectionalListIteratorBase tmp = *this;
            Node* previous = m_node->m_previous;
            CONTAINERS_VERIFY(previous != nullptr, "Iterator cannot be decremented past the beginning");
            m_node = previous;
            return tmp;
        }

        bool operator==(const BidirectionalListIteratorBase& other) const
        {
            return m_node == other.m_node;
        };
        bool operator!=(const BidirectionalListIteratorBase& other) const
        {
            return  m_node != other.m_node;
        };
    };

    template<typename T, UniqueTypedAllocatorConcept<NodeDoubleSided<T>> Alloc = 
        UniqueTypedAllocator<NodeDoubleSided<T>>,  LifetimeManagerConcept<T> Life = 
        LifetimeManager<T>>
    class ListDoubleSidedTailed
    {
    public:
        using ValueType = T;
        using Node = NodeDoubleSided<ValueType>;

        using Iterator = BidirectionalListIteratorBase<ValueType>;
        using ConstIterator = BidirectionalListIteratorBase<const ValueType>;

    private:
        Node* m_head = nullptr;
        Node* m_tail = nullptr;
        [[no_unique_address]] Alloc m_allocator;
        [[no_unique_address]] Life m_lifetimeManager;

    public:

        ListDoubleSidedTailed() = default;

        ListDoubleSidedTailed(const ListDoubleSidedTailed& other)
            requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
        {
            copyList(other.m_head);
        }

        ListDoubleSidedTailed& operator=(const ListDoubleSidedTailed& other)
            requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
        {
            if (&other != this)
            {
                clear();
                copyList(other.m_head);
            }
            return *this;
        }

        ListDoubleSidedTailed(ListDoubleSidedTailed&& other) :
            m_head(other.m_head), m_tail(other.m_tail) {
            other.m_head = nullptr;
            other.m_tail = nullptr;
        };

        ListDoubleSidedTailed& operator=(ListDoubleSidedTailed&& other)
        {
            if (this != &other)
            {
                clear();
                m_head = other.m_head;
                m_tail = other.m_tail;
                other.m_head = nullptr;
                other.m_tail = nullptr;
            }
            return *this;
        }

        ~ListDoubleSidedTailed() {
            clear();
        }

        void clear()
        {
            Node* current = m_head;
            Node* next = nullptr;
            while (current != nullptr) {
                next = current->m_next;
				deleteNode(current);
                current = next;
            }
            m_head = nullptr;
            m_tail = nullptr;
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void insertNext(Node* current, U&& value) //inserts the element after current
        {
            if (current == nullptr) {
                throw std::runtime_error("Cannot insert after null node");
            }
            Node* next = current->m_next;
            Node* newNext = copyConstruct(std::forward<U>(value));
            current->m_next = newNext;
            newNext->m_previous = current;
            newNext->m_next = next;
            if (next != nullptr)
                next->m_previous = newNext;
            else m_tail = newNext;
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void insertPrevious(Node* current, U&& value) //inserts the element before current
        {
            if (current == nullptr) {
                throw std::runtime_error("Cannot insert after null node");
            }
            Node* previous = current->m_previous;
            Node* newPrevious = copyConstruct(std::forward<U>(value));
            current->m_previous = newPrevious;
            newPrevious->m_next = current;
            newPrevious->m_previous = previous;
            if (previous != nullptr)
                previous->m_next = newPrevious;
            else m_head = newPrevious;
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void pushBack(U&& value) //inserts the element to the back
        {
            Node* newNode = copyConstruct(std::forward<U>(value));

            if (m_tail == nullptr) {  // Empty list
                m_head = newNode;
                m_tail = newNode;
            }
            else {  // Non-empty list
                m_tail->m_next = newNode;
                newNode->m_previous = m_tail;
                m_tail = newNode;
            }
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void pushFront(U&& value) //inserts the element to the front
        {
            Node* newNode = copyConstruct(std::forward<U>(value));

            if (m_head == nullptr) {  // Empty list
                m_head = newNode;
                m_tail = newNode;
            }
            else {  // Non-empty list
                newNode->m_next = m_head;
                m_head->m_previous = newNode;
                m_head = newNode;
            }
        }

        void erase(Node* current) //deletes the current element
        {
            if (current == nullptr) {
                throw std::runtime_error("Cannot delete a null node");
            }

            if (m_head == nullptr) {
                throw std::runtime_error("List is empty");
            }

            if (current == m_head) {
                if (current == m_tail)
                {
                    m_head = nullptr;
                    m_tail = nullptr;
                    deleteNode(current);
                    return;
                }
                m_head = current->m_next;
                if (m_head != nullptr)
                    m_head->m_previous = nullptr;
            }
            else if (current == m_tail) {
                if (current == m_head)
                {
                    m_head = nullptr;
                    m_tail = nullptr;
                    deleteNode(current);
                    return;
                }
                m_tail = current->m_previous;
                if (m_tail != nullptr)
                    m_tail->m_next = nullptr;
            }
            else
            {
                current->m_previous->m_next = current->m_next;
                current->m_next->m_previous = current->m_previous;
            }
            deleteNode(current);
        }

        void popBack() //deletes the tail
        {
            if (m_head == nullptr) {
                throw std::runtime_error("List is empty");
            }

            Node* previous = m_tail->m_previous;
            if (previous != nullptr)
                previous->m_next = nullptr;
            else m_head = nullptr;
            deleteNode(m_tail);
            m_tail = previous;
        }

        void popFront() //deletes the head
        {
            if (m_head == nullptr) {
                throw std::runtime_error("List is empty");
            }

            Node* next = m_head->m_next;
            if (next != nullptr)
                next->m_previous = nullptr;
            else m_tail = nullptr;
			deleteNode(m_head);
            m_head = next;
        }

        Node* back() { return m_tail; };
        const Node* back() const { return m_tail; };

        Node* front() { return m_head; };
        const Node* front() const { return m_head; };

        bool empty() const { return m_head = nullptr; };

        Iterator begin() { return Iterator(m_head); }
        Iterator end() { return Iterator(nullptr); }
        ConstIterator begin() const { return ConstIterator(m_head); }
        ConstIterator end() const { return ConstIterator(nullptr); }
        ConstIterator cbegin() const { return begin(); }
        ConstIterator cend() const { return end(); }

    private:

        void copyList(const Node* otherHead)
        {
            if (otherHead == nullptr)
            {
                m_head = nullptr;
                m_tail = nullptr;
                return;
            }

            m_head = copyConstruct(otherHead->m_data);

            const Node* currentOther = otherHead->m_next;
            Node* currentThis = nullptr;
            Node* priorThis = m_head;

            while (currentOther != nullptr)
            {
                currentThis = copyConstruct(currentOther->m_data);
                priorThis->m_next = currentThis;
                currentThis->m_previous = priorThis;
                currentOther = currentOther->m_next;
                priorThis = currentThis;
            }
            m_tail = currentThis;
        }

        template<typename U>
            requires std::is_same_v<T, std::decay_t<U>>
        Node* copyConstruct(U&& data)
        {
            Node* newNode = m_allocator.allocate();
            m_lifetimeManager.construct(&newNode->m_data, std::forward<U>(data));
            return newNode;
        }

        void deleteNode(Node* node)
        {
            m_lifetimeManager.destroy(&node->m_data);
            m_allocator.deallocate(node);
        }
    };

}