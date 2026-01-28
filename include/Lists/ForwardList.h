#pragma once
#include "../Utilities/ByteArray.h"
#include "../Utilities/Concepts.h"
#include "../Utilities/Macros.h"

#include <type_traits>
#include <utility>
#include <concepts>
#include <stdexcept>

#include "Lists.h"

namespace Containers
{

    template <typename T, UniqueTypedAllocatorType<ListForwardNode<T>> Alloc 
    = UniqueTypedAllocator<ListForwardNode<T>>, LifetimeManagerType<T> Life = LifetimeManager<T>>
    class ForwardList
    {
    public:
        using ValueType = T;
        using SizeType = size_t;
        using Node = ListForwardNode<T>;
        using SentinelNode = ListForwardSentinelNode<T>;

        using Iterator = ForwardListIteratorBase<ValueType, Node>;
        using ConstIterator = ForwardListIteratorBase<const ValueType, const Node>;

    private:
        SentinelNode m_sentinel;
        SizeType m_size;
        [[no_unique_address]] Alloc m_allocator;
        [[no_unique_address]] Life m_lifetimeManager;

    public:
        ForwardList() : m_size(0) {
            resetSentinel();
        }

        ForwardList(const ForwardList &other) : m_size(0)
        {
            resetSentinel();
            Node* current = m_sentinel.next();
            const Node* currentOther = other.m_sentinel.next();
            addRange(current, getSentinel(), currentOther, other.getSentinel());
            m_size = other.m_size;
        }

        ForwardList &operator=(const ForwardList &other)
        {
            if (&other != this) return *this;
            
            if(other.size() > size())
            {
                Node* current = m_sentinel.next();
                const Node* currentOther = other.m_sentinel.next();
                while(current != getSentinel()) {
                    m_lifetimeManager.destroy(current->value());
                    m_lifetimeManager.construct(current->value(), currentOther->value());
                    current = current->next();
                    currentOther = currentOther->next();
                }
                current = tail();
                addRange(current, getSentinel(), currentOther, getSentinel());
            }
            else
            {
                Node* current = m_sentinel.next();
                const Node* currentOther = other.m_sentinel.next();
                while(currentOther != other.getSentinel()) {
                    m_lifetimeManager.destroy(current->value());
                    m_lifetimeManager.construct(current->value(), currentOther->value());
                    current = current->next();
                    currentOther = currentOther->next();
                }
                clearOrphans(current->next(), getSentinel());
                current->setNext(getSentinel());                
            }
            m_size = other.m_size;           
            return *this;
        }

        ForwardList(ForwardList &&other) { 
            m_sentinel.setNext(other.m_sentinel.next());        
            other.resetSentinel();
            m_size = std::exchange(other.m_size, 0);
            m_allocator = std::exchange(other.m_allocator, Alloc());
            m_lifetimeManager = std::exchange(other.m_lifetimeManager, Life());
        };

        ForwardList &operator=(ForwardList &&other)
        {
            if (this != &other)
            {
                clear();
                m_sentinel.setNext(other.m_sentinel.next());
                other.resetSentinel();
                m_size = std::exchange(other.m_size, 0);
                m_allocator = std::exchange(other.m_allocator, Alloc());
                m_lifetimeManager = std::exchange(other.m_lifetimeManager, Life());
            }
            return *this;
        }

        ~ForwardList()
        {
            clear();
        }

        void clear()
        {
            clearOrphans(head(), getSentinel());   
            resetSentinel();
            m_size = 0;
        }

        template <typename... Args>
        void pushFront(Args&&... args)
        {
            Node *newNode = construct(std::forward<Args>(args)...);
            Node* headNode = head();
            newNode->setNext(headNode);
            m_sentinel.setNext(newNode);
            ++m_size;
        }

        void popFront()
        {
            CONTAINERS_VERIFY(!empty(), "Cannot pop an empty list");
            Node* headNode = head();
            m_sentinel.setNext(headNode->next());
            deleteNode(headNode);
            --m_size;
        }

        Iterator erase(ConstIterator cit)
        {
            Node* node = asMutable(cit).node();
            Node* next = node->next();
            node->setNext(next->next());
            deleteNode(next);
            --m_size;
            return Iterator(node->next());
        }

        Iterator erase(ConstIterator cbegin, ConstIterator cend)
        {
            if (cbegin == cend) return Iterator(asMutable(cend));
            Node* begin = asMutable(cbegin).node();
            Node* end = asMutable(cend).node();
            Node* start = begin->next();
            if(start == end) return Iterator(end);
            begin->setNext(end);
            m_size -= clearOrphansCounted(start, end);
            return Iterator(end);
        }

        template<typename... Args>
        Iterator insert(ConstIterator cit, Args... args)
        {            
            Node* node = asMutable(cit).node();
            Node* newNode = construct(std::forward<Args>(args)...);
            newNode->setNext(node->next());
            node->setNext(newNode);
            ++m_size;
            return Iterator(newNode);
        }
        
        // inserting at end will insert to the beggining of the list
        Iterator insert(ConstIterator cit, ConstIterator begin, ConstIterator end)
        {
            Node* node = asMutable(cit).node();
            m_size += addRangeCounted(node, node->next(), begin, end);            
            return Iterator(node);
        }

        ValueType& front() { return head()->value(); }
        const ValueType& front() const { return head()->value(); }

        bool empty() const { return m_size == 0; }
        SizeType size() const { return m_size; }

        Iterator begin() { return Iterator(head()); }
        Iterator end() { return Iterator(getSentinel()); }
        ConstIterator begin() const { return ConstIterator(head()); }
        ConstIterator end() const { return ConstIterator(getSentinel()); }
        ConstIterator cbegin() const { return begin(); }
        ConstIterator cend() const { return end(); }

        inline Iterator asMutable(ConstIterator it)
		{
			return Iterator(const_cast<Node *>(it.node()));
		}

    private:

        void clearOrphans(Node* orphan, const Node* end) {
            while(orphan != end) {
                Node* next = orphan->next();
                deleteNode(orphan);
                orphan = next;
            }
        }

        SizeType clearOrphansCounted(Node* orphan, const Node* end) {
            SizeType count = 0;
            while(orphan != end) {
                Node* next = orphan->next();
                deleteNode(orphan);
                orphan = next;
                ++count;
            }
            return count;
        }

        void addRange(Node* dst, Node* dstEnd, const Node* src, const Node* srcEnd)
        {
            while(src != srcEnd) {
                Node* newNode = construct(src->value());
                dst->setNext(newNode);
                dst = newNode;
                src = src->next();
            }
            dst->setNext(dstEnd);
        }

        SizeType addRangeCounted(Node*& dst, Node* dstEnd, const Node* src, const Node* srcEnd)
        {
            SizeType count = 0;
            while(src != srcEnd) {
                Node* newNode = construct(src->value());
                dst->setNext(newNode);
                dst = newNode;
                src = src->next();
                ++count;
            }
            dst->setNext(dstEnd);
            return count;
        }
        
        template <typename... Args>
        Node* construct(Args&&... args)
        {
            Node *newNode = m_allocator.allocate();
            m_lifetimeManager.construct(newNode->data(), std::forward<Args>(args)...);
            return newNode;
        }

        void deleteNode(Node *node)
        {
            m_lifetimeManager.destroy(node->data());
            m_allocator.deallocate(node);
        }

        Node* getSentinel() { return static_cast<Node*>(&m_sentinel); }
        const Node* getSentinel() const { return static_cast<Node*>(&m_sentinel); }

        void resetSentinel() {
            m_sentinel.setNext(getSentinel());
        }

        Node* head() { return static_cast<Node*>(m_sentinel.next()); } //next of sentinel is head
        const Node* head() const { return static_cast<Node*>(m_sentinel.next()); }
    };

}