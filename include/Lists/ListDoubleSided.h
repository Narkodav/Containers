#pragma once
#include <type_traits>
#include <utility>
#include <concepts>
#include <stdexcept>

#include "ListDoubleSidedTailed.h"

namespace Containers {

    template<typename T, UniqueTypedAllocatorConcept<NodeDoubleSided<T>> Alloc =
        UniqueTypedAllocator<NodeDoubleSided<T>>, LifetimeManagerConcept<T> Life =
        LifetimeManager<T>>
    class ListDoubleSided
    {
    public:
        using ValueType = T;
        using Node = NodeDoubleSided<T>;

        using Iterator = BidirectionalListIteratorBase<ValueType>;
        using ConstIterator = BidirectionalListIteratorBase<const ValueType>;

    private:
        Node* m_head = nullptr;
        [[no_unique_address]] Alloc m_allocator;
        [[no_unique_address]] Life m_lifetimeManager;

    public:

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
				deleteNode(current);
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
            Node* newNext = copyConstruct(std::forward<U>(value));
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
            if (m_head == nullptr)
            {
                m_head = copyConstruct(std::forward<U>(value));
                return;
            }

            Node* current = m_head;
            while (current->m_next != nullptr) {
                current = current->m_next;
            }

            Node* newNode = copyConstruct(std::forward<U>(value));
            current->m_next = newNode;
            newNode->m_previous = current;
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void pushFront(U&& value) //inserts the element to the front
        {
            Node* newHead = copyConstruct(std::forward<U>(value));
            newHead->m_next = m_head;
            m_head->m_previous = newHead;
            m_head = newHead;
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
            deleteNode(current);
        }

        void popBack() //deletes the element from the back
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
            deleteNode(current);
        }

        void popFront() //deletes the head
        {
            if (m_head == nullptr) {
                throw std::runtime_error("List is empty");
            }

            Node* newHead = m_head->m_next;
			deleteNode(m_head);
            m_head = newHead;
            if (m_head != nullptr)
                m_head->m_previous = nullptr;
        }

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