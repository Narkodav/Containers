#pragma once
#include <type_traits>
#include <utility>
#include <concepts>
#include <stdexcept>

#include "../Utilities/Concepts.h"
#include "../Utilities/ReusableStorage.h"

namespace Containers
{
    template <typename Node, typename T>
    concept ForwardListNodeType = requires(Node& node) {
        {node.next()} -> std::convertible_to<Node*>;
        {node.data()} -> std::convertible_to<T*>;
        {node.value()} -> std::convertible_to<T&>;
        {std::as_const(node).next()} -> std::convertible_to<const Node*>;
        {std::as_const(node).data()} -> std::convertible_to<const T*>;
        {std::as_const(node).value()} -> std::convertible_to<const T&>;
    };

    template <typename Node, typename T>
    concept BidirectionalListNodeType = ForwardListNodeType<Node, T> && requires(Node& node) {
        {node.prev()} -> std::convertible_to<Node*>;
        {std::as_const(node).prev()} -> std::convertible_to<const Node*>;
    };

    template<typename T>
    class ListForwardNode;

    template<typename T>
    class ListForwardSentinelNode
    {
    protected:
        ListForwardNode<T> *m_next = nullptr;
    public:
        ListForwardSentinelNode() = default;
        ~ListForwardSentinelNode() = default;
        ListForwardSentinelNode(const ListForwardSentinelNode&) = delete;
        ListForwardSentinelNode& operator=(const ListForwardSentinelNode&) = delete;
        ListForwardSentinelNode(ListForwardSentinelNode&&) = delete;
        ListForwardSentinelNode& operator=(ListForwardSentinelNode&&) = delete;

        ListForwardNode<T>* next() noexcept { return m_next; }
        const ListForwardNode<T>* next() const noexcept { return m_next; }

        void setNext(ListForwardNode<T>* next) noexcept { m_next = next; }
    };
    
    template<typename T>
    class ListBidirectionalNode;

    template<typename T>
    class ListBidirectionalSentinelNode
    {
    protected:
        ListBidirectionalNode<T>* m_next = nullptr;
        ListBidirectionalNode<T>* m_prev = nullptr;
    public:
        ListBidirectionalSentinelNode() = default;
        ~ListBidirectionalSentinelNode() = default;
        ListBidirectionalSentinelNode(const ListBidirectionalSentinelNode&) = delete;
        ListBidirectionalSentinelNode& operator=(const ListBidirectionalSentinelNode&) = delete;
        ListBidirectionalSentinelNode(ListBidirectionalSentinelNode&&) = delete;
        ListBidirectionalSentinelNode& operator=(ListBidirectionalSentinelNode&&) = delete;

        ListBidirectionalNode<T>* prev() noexcept { return m_prev; }
        const ListBidirectionalNode<T>* prev() const noexcept { return m_prev; }

        void setPrev(ListBidirectionalNode<T>* prev) noexcept { 
            m_prev = prev;
        }

        ListBidirectionalNode<T>* next() noexcept { return m_next; }
        const ListBidirectionalNode<T>* next() const noexcept { return m_next; }

        void setNext(ListBidirectionalNode<T>* next) noexcept { 
            m_next = next;
        }
    };

    template<typename T>
    class ListNodeDataStorageBase {
    protected:
        ReusableStorage<T> m_data;
    public:
        ListNodeDataStorageBase() = default;
        ~ListNodeDataStorageBase() = default;
        ListNodeDataStorageBase(const ListNodeDataStorageBase&) = delete;
        ListNodeDataStorageBase& operator=(const ListNodeDataStorageBase&) = delete;
        ListNodeDataStorageBase(ListNodeDataStorageBase&&) = delete;
        ListNodeDataStorageBase& operator=(ListNodeDataStorageBase&&) = delete;

        T* data() noexcept { return m_data.data(); }
        const T* data() const noexcept { return m_data.data(); }
        T& value() noexcept { return m_data.value(); }
        const T& value() const noexcept { return m_data.value(); }
    };

    template<typename T>
    class ListForwardNode : public ListForwardSentinelNode<T>, public ListNodeDataStorageBase<T> {
    public:
        using ListForwardSentinelNode<T>::ListForwardSentinelNode;
        using ListNodeDataStorageBase<T>::ListNodeDataStorageBase;
    };

    template<typename T>
    class ListBidirectionalNode : public ListBidirectionalSentinelNode<T>, public ListNodeDataStorageBase<T> {
    public:
        using ListBidirectionalSentinelNode<T>::ListBidirectionalSentinelNode;
        using ListNodeDataStorageBase<T>::ListNodeDataStorageBase;
    };
    

    template <typename T, ForwardListNodeType<T> Node>
    class ForwardListIteratorBase
    {
    protected:
        Node *m_node = nullptr;

    public:
        using ValueType = T;
        using Pointer = ValueType*;
        using Reference = ValueType&;

        ForwardListIteratorBase() = default;
        ForwardListIteratorBase(Node *node) : m_node(node) {};

        ForwardListIteratorBase(const ForwardListIteratorBase&) = default;
        ForwardListIteratorBase& operator=(const ForwardListIteratorBase&) = default;
        ForwardListIteratorBase(ForwardListIteratorBase&&) = default;
        ForwardListIteratorBase& operator=(ForwardListIteratorBase&&) = default;

        Reference operator*()
        {
            return m_node->value();
        };
        Pointer operator->()
        {
            return m_node->data();
        };

        const Reference operator*() const
        {
            return m_node->value();
        }

        const ValueType* operator->() const
        {
            return m_node->data();
        };

        // Increment operator (in-order traversal)
        ForwardListIteratorBase &operator++()
        {
            Node *next = m_node->next();
            CONTAINERS_VERIFY(next != nullptr, "Iterator cannot be incremented past the end");
            m_node = next;
            return *this;
        }
        ForwardListIteratorBase operator++(int)
        {
            ForwardListIteratorBase tmp = *this;
            Node *next = m_node->next();
            CONTAINERS_VERIFY(next != nullptr, "Iterator cannot be incremented past the end");
            m_node = next;
            return tmp;
        }
        bool operator==(const ForwardListIteratorBase &other) const
        {
            return m_node == other.m_node;
        };
        bool operator!=(const ForwardListIteratorBase &other) const
        {
            return m_node != other.m_node;
        };

        Node* node() { return m_node; }
        const Node* node() const { return m_node; }
    };

    template <typename T, BidirectionalListNodeType<T> Node>
    class BidirectionalListIteratorBase : public ForwardListIteratorBase<T, Node>
    {
    public:
        using ForwardListIteratorBase<T, Node>::ForwardListIteratorBase;

        BidirectionalListIteratorBase &operator--()
        {
            Node *prev = this->m_node->prev();
            CONTAINERS_VERIFY(prev != nullptr, "Iterator cannot be decremented past the beginning");
            this->m_node = prev;
            return *this;
        }
        BidirectionalListIteratorBase operator--(int)
        {
            BidirectionalListIteratorBase tmp = *this;
            Node *prev = this->m_node->prev();
            CONTAINERS_VERIFY(prev != nullptr, "Iterator cannot be decremented past the beginning");
            this->m_node = prev;
            return tmp;
        }
    };
}