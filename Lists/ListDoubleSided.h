#pragma once
#include <type_traits>
#include <utility>
#include <concepts>
#include <stdexcept>

namespace Containers {
    template<typename T>
    class ListDoubleSided
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

    public:
        // Iterator class to handle traversal

        ListDoubleSided() = default;

        ListDoubleSided(const ListDoubleSided& other)
            requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
        {
            copyList(other.m_head);
        }

        ListDoubleSided& operator=(const ListDoubleSided& other)
            requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
        {
            if (&other != this)
            {
                clear();
                copyList(other.m_head);
            }
            return *this;
        }

        ListDoubleSided(ListDoubleSided&& other) : m_head(other.m_head) { other.m_head = nullptr; };

        ListDoubleSided& operator=(ListDoubleSided&& other)
        {
            if (this != &other)
            {
                clear();
                m_head = other.m_head;
                other.m_head = nullptr;
            }
            return *this;
        }

        ~ListDoubleSided() {
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
            if (m_head == nullptr)
            {
                m_head = Node::copyConstruct(std::forward<U>(value));
                return;
            }

            Node* current = m_head;
            while (current->m_next != nullptr) {
                current = current->m_next;
            }

            Node* newNode = Node::copyConstruct(std::forward<U>(value));
            current->m_next = newNode;
            newNode->m_previous = current;
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void insertFront(U&& value) //inserts the element to the front
        {
            Node* newHead = Node::copyConstruct(std::forward<U>(value));
            newHead->m_next = m_head;
            m_head->m_previous = newHead;
            m_head = newHead;
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
                m_head = current->m_next;
                if (m_head != nullptr)
                    m_head->m_previous = nullptr;
            }
            else
            {
                current->m_previous->m_next = current->m_next;
                if (current->m_next != nullptr)
                    current->m_next->m_previous = current->m_previous;
            }
            delete current;
        }

        void deleteBack() //deletes the element from the back
        {
            if (m_head == nullptr) {
                throw std::runtime_error("List is empty");
            }

            Node* current = m_head;
            if (current->m_next != nullptr)
            {
                while (current->m_next != nullptr)
                    current = current->m_next;
                current->m_previous->m_next = nullptr;
            }
            else m_head = nullptr;
            delete current;
        }

        void deleteFront() //deletes the head
        {
            if (m_head == nullptr) {
                throw std::runtime_error("List is empty");
            }

            Node* newHead = m_head->m_next;
            delete m_head;
            m_head = newHead;
            if (m_head != nullptr)
                m_head->m_previous = nullptr;
        }

        Node* getFront() { return m_head; };

        const Node* getFront() const { return m_head; };

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
        }
    };

}