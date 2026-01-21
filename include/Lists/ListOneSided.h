#pragma once
#include "../Utilities/ByteArray.h"
#include "../Utilities/Concepts.h"
#include "../Utilities/Macros.h"

#include <type_traits>
#include <utility>
#include <concepts>
#include <stdexcept>

namespace Containers {

    template<typename T>
    struct NodeOneSided {
        T m_data;
        NodeOneSided* m_next;
    };


    template<typename T, UniqueTypedAllocatorConcept<NodeOneSided<T>> Alloc = UniqueTypedAllocator<NodeOneSided<T>>,
        LifetimeManagerConcept<T> Life = LifetimeManager<T>>
    class ListOneSided
    {
    public:
        using ValueType = T;
        using Node = NodeOneSided<T>;

        template<typename U>
        class IteratorBase
        {
        private:
            Node* m_node = nullptr;
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = U;
            using difference_type = std::ptrdiff_t;
            using pointer = U*;
            using reference = U&;
            IteratorBase() = default;
            IteratorBase(Node* node) : Node(*node) {};

            reference operator*() {
                return m_node->m_data;
            };
            pointer operator->() {
                return &(m_node->m_data);
            }
            // Increment operator (in-order traversal)
            IteratorBase& operator++() {
                Node* next = m_node->m_next;
                CONTAINERS_VERIFY(next != nullptr, "Iterator cannot be incremented past the end");
                m_node = next;
                return *this;
            }
            IteratorBase operator++(int) {
                IteratorBase tmp = *this;
                Node* next = m_node->m_next;
                CONTAINERS_VERIFY(next != nullptr, "Iterator cannot be incremented past the end");
                m_node = next;
                return tmp;
            }
            bool operator==(const IteratorBase& other) const
            {
                return m_node == other.m_node;
            };
            bool operator!=(const IteratorBase& other) const
            {
                return  m_node != other.m_node;
            };
        };

		using Iterator = IteratorBase<ValueType>;
		using ConstIterator = IteratorBase<const ValueType>;

    private:
        Node* m_head = nullptr;
        [[no_unique_address]] Alloc m_allocator;
        [[no_unique_address]] Life m_lifetimeManager;

    public:

        ListOneSided() = default;

        ListOneSided(const ListOneSided& other)
            requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
        {
            copyList(other.m_head);
        }

        ListOneSided& operator=(const ListOneSided& other)
            requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
        {
            if (&other != this)
            {
                clear();
                copyList(other.m_head);
            }
            return *this;
        }

        ListOneSided(ListOneSided&& other) : m_head(other.m_head) { other.m_head = nullptr; };

        ListOneSided& operator=(ListOneSided&& other)
        {
            if (this != &other)
            {
                clear();
				m_head = std::exchange(other.m_head, nullptr);
            }
            return *this;
        }

        ~ListOneSided() {
            clear();
        }

        void clear()
        {
            Node* current = m_head;
            Node* next = nullptr;
            while (current != nullptr) {
                next = current->m_next;
				m_lifetimeManager.destroy(&current->m_data);
				m_allocator.deallocate(current);
                current = next;
            }
            m_head = nullptr;
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void insert(Node* current, U&& value) //inserts the element after current
        {
            if (current == nullptr) {
                throw std::runtime_error("Cannot insert after null node");
            }
            Node* next = current->m_next;
            Node* newNext = copyConstruct(std::forward<U>(value));
            current->m_next = newNext;
            newNext->m_next = next;
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

            current->m_next = copyConstruct(std::forward<U>(value));
        }

        template<typename U>
            requires std::is_same_v<ValueType, std::decay_t<U>>
        void pushFront(U&& value) //inserts the element to the front
        {
            Node* newHead = copyConstruct(std::forward<U>(value));
            newHead->m_next = m_head;
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
            }
            else
            {
                Node* prior = m_head;
                while (prior->m_next != current) {
                    prior = prior->m_next;
                }
                prior->m_next = current->m_next;
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
                Node* prior = nullptr;
                do {
                    prior = current;
                    current = current->m_next;
                } while (current->m_next != nullptr);
                prior->m_next = nullptr;
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