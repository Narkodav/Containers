#pragma once
#include <type_traits>
#include <utility>
#include <concepts>
#include <stdexcept>

namespace Containers {
    template<typename T>
    class ListDoubleSidedTailed
    {
    public:
        using ValueType = T;

        struct Node {
            ValueType m_data;
            Node* m_next, * m_previous;

            Node() : m_next(nullptr), m_previous(nullptr) {}

            template<typename U>
                requires std::is_same_v<ValueType, std::decay_t<U>>
            Node(U&& value) : m_data(std::forward<U>(value)), m_next(nullptr), m_previous(nullptr) {}

            template<typename U>
                requires std::is_same_v<ValueType, std::decay_t<U>>
            Node& operator=(U&& value) {
                m_data = std::forward<U>(value);
                return *this;
            };

            template<typename U>
                requires std::is_same_v<ValueType, std::decay_t<U>>
            static Node* copyConstruct(U&& data)
            {
                Node* newNode;
                if constexpr (std::is_rvalue_reference_v<decltype(data)>)
                {
                    if constexpr (std::is_move_constructible_v<T>)
                        newNode = new Node(std::move(data));
                    else if constexpr (std::is_move_assignable_v<T>)
                    {
                        newNode = new Node();
                        newNode->m_data = std::move(data);
                    }
                    else static_assert(false && "Cannot move a type without move constructor or assignment defined");
                }
                else // lvalue
                {
                    if constexpr (std::is_copy_constructible_v<T>)
                        newNode = new Node(data);
                    else if constexpr (std::is_copy_assignable_v<T>)
                    {
                        newNode = new Node();
                        newNode->m_data = data;
                    }
                    else static_assert(false && "Cannot copy a type without copy constructor or assignment defined");
                }
                return newNode;
            }

        };

    private:
        Node* m_head = nullptr;
        Node* m_tail = nullptr;

    public:
        // Iterator class to handle traversal

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
                delete current;
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
            Node* newNext = Node::copyConstruct(std::forward<U>(value));
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
            Node* newPrevious = Node::copyConstruct(std::forward<U>(value));
            current->m_previous = newPrevious;
            newPrevious->m_next = current;
            newPrevious->m_previous = previous;
            if (previous != nullptr)
                previous->m_next = newPrevious;
            else m_head = newPrevious;
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void insertBack(U&& value) //inserts the element to the back
        {
            Node* newNode = Node::copyConstruct(std::forward<U>(value));

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
        void insertFront(U&& value) //inserts the element to the front
        {
            Node* newNode = Node::copyConstruct(std::forward<U>(value));

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

        void deleteNode(Node* current) //deletes the current element
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
                    delete current;
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
                    delete current;
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
            delete current;
        }

        void deleteBack() //deletes the tail
        {
            if (m_head == nullptr) {
                throw std::runtime_error("List is empty");
            }

            Node* previous = m_tail->m_previous;
            if (previous != nullptr)
                previous->m_next = nullptr;
            else m_head = nullptr;
            delete m_tail;
            m_tail = previous;
        }

        void deleteFront() //deletes the head
        {
            if (m_head == nullptr) {
                throw std::runtime_error("List is empty");
            }

            Node* next = m_head->m_next;
            if (next != nullptr)
                next->m_previous = nullptr;
            else m_tail = nullptr;
            delete m_head;
            m_head = next;
        }

        Node* getFront() { return m_head; };
        Node* getBack() { return m_tail; };

        const Node* getFront() const { return m_head; };
        const Node* getBack() const { return m_tail; };

        static Node* iterateNext(Node* current) { return current->m_next; };
        static Node* iteratePrevious(Node* current) { return current->m_previous; };

        static const Node* iterateNext(const Node* current) { return current->m_next; };
        static const Node* iteratePrevious(const Node* current) { return current->m_previous; };

        bool empty() const { return m_head = nullptr; };

    private:

        void copyList(const Node* otherHead)
        {
            if (otherHead == nullptr)
            {
                m_head = nullptr;
                m_tail = nullptr;
                return;
            }

            m_head = Node::copyConstruct(otherHead->m_data);

            const Node* currentOther = otherHead->m_next;
            Node* currentThis = nullptr;
            Node* priorThis = m_head;

            while (currentOther != nullptr)
            {
                currentThis = Node::copyConstruct(currentOther->m_data);
                priorThis->m_next = currentThis;
                currentThis->m_previous = priorThis;
                currentOther = currentOther->m_next;
                priorThis = currentThis;
            }
            m_tail = currentThis;
        }
    };

}