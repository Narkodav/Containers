#pragma once
#include <type_traits>
#include <utility>
#include <concepts>
#include <stdexcept>

#include "Lists.h"
#include "../Utilities/Concepts.h"

namespace Containers
{

    template <typename T, UniqueTypedAllocatorType<ListBidirectionalNode<T>> Alloc 
    = UniqueTypedAllocator<ListBidirectionalNode<T>>, LifetimeManagerType<T> Life = LifetimeManager<T>>
    class BidirectionalList
    {
    public:
        using ValueType = T;
        using SizeType = size_t;
        using Node = ListBidirectionalNode<T>;
        using SentinelNode = ListBidirectionalSentinelNode<T>;

        using Iterator = BidirectionalListIteratorBase<ValueType, Node>;
        using ConstIterator = BidirectionalListIteratorBase<const ValueType, const Node>;

    private:
        SentinelNode m_sentinel;
        SizeType m_size = 0;
        [[no_unique_address]] Alloc m_allocator;
        [[no_unique_address]] Life m_lifetimeManager;

    public:
        BidirectionalList() : m_size(0) {
            resetSentinel();
        }

        BidirectionalList(const BidirectionalList &other) : m_size(0)
        {
            resetSentinel();
            Node* current = m_sentinel.next();
            const Node* currentOther = other.m_sentinel.next();
            addRange(current, getSentinel(), currentOther, other.getSentinel());
            m_size = other.m_size;
        }

        BidirectionalList &operator=(const BidirectionalList &other)
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
                m_sentinel.setPrev(current);
                clearOrphans(current->next(), getSentinel());
                current->setNext(getSentinel());                
            }
            m_size = other.m_size;           
            return *this;
        }

        BidirectionalList(BidirectionalList &&other) { 
            m_sentinel.setNext(other.m_sentinel.next());
            m_sentinel.setPrev(other.m_sentinel.prev());            
            other.resetSentinel();
            m_size = std::exchange(other.m_size, 0);
            m_allocator = std::exchange(other.m_allocator, Alloc());
            m_lifetimeManager = std::exchange(other.m_lifetimeManager, Life());
        };

        BidirectionalList &operator=(BidirectionalList &&other)
        {
            if (this != &other)
            {
                clear();
                m_sentinel.setNext(other.m_sentinel.next());
                m_sentinel.setPrev(other.m_sentinel.prev());            
                other.resetSentinel();
                m_size = std::exchange(other.m_size, 0);
                m_allocator = std::exchange(other.m_allocator, Alloc());
                m_lifetimeManager = std::exchange(other.m_lifetimeManager, Life());
            }
            return *this;
        }

        ~BidirectionalList()
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
        void pushBack(Args&&... args)
        {
            Node *newNode = construct(std::forward<Args>(args)...);
            Node* tailNode = tail();
            tailNode->setNext(newNode);
            newNode->setPrev(tailNode);
            newNode->setNext(getSentinel());
            m_sentinel.setPrev(newNode);
            ++m_size;
        }

        template <typename... Args>
        void pushFront(Args&&... args)
        {
            Node *newNode = construct(std::forward<Args>(args)...);
            Node* headNode = head();
            headNode->setPrev(newNode);
            newNode->setNext(headNode);
            newNode->setPrev(getSentinel());
            m_sentinel.setNext(newNode);
            ++m_size;
        }

        void popBack()
        {
            CONTAINERS_VERIFY(!empty(), "Cannot pop an empty list");
            Node* tailNode = tail();
            tailNode->prev()->setNext(getSentinel());
            m_sentinel.setPrev(tailNode->prev());
            deleteNode(tailNode);
            --m_size;
        }

        void popFront()
        {
            CONTAINERS_VERIFY(!empty(), "Cannot pop an empty list");
            Node* headNode = head();
            headNode->next()->setPrev(getSentinel());
            m_sentinel.setNext(headNode->next());
            deleteNode(headNode);
            --m_size;
        }

        Iterator erase(ConstIterator cit)
        {
            if (cit == end()) return end();
            Node* node = asMutable(cit).node();
            Node* next = node->next();
            node->prev()->setNext(next);
            next->setPrev(node->prev());
            deleteNode(node);
            --m_size;
            return Iterator(next);
        }

        Iterator erase(ConstIterator cbegin, ConstIterator cend)
        {
            if (cbegin == cend) return Iterator(asMutable(cend));
            Node* begin = asMutable(cbegin).node();
            Node* end = asMutable(cend).node();
            begin->prev()->setNext(end);
            end->setPrev(begin->prev());
            m_size -= clearOrphansCounted(begin, end);
            return Iterator(end);
        }

        template<typename... Args>
        Iterator insert(ConstIterator cit, Args... args)
        {            
            Node* node = asMutable(cit).node();
            Node* newNode = construct(std::forward<Args>(args)...);            
            newNode->setNext(node);
            newNode->setPrev(node->prev());
            node->prev()->setNext(newNode);
            node->setPrev(newNode);
            ++m_size;
            return Iterator(newNode);
        }
        
        Iterator insert(ConstIterator cit, ConstIterator begin, ConstIterator end)
        {
            Node* node = asMutable(cit).node();
            Node* start = node->prev();
            m_size += addRangeCounted(start, node, begin, end);            
            return Iterator(start);
        }

        ValueType& front() { return head()->value(); }
        const ValueType& front() const { return head()->value(); }

        ValueType& back() { return tail()->value(); }
        const ValueType& back() const { return tail()->value(); }

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
                newNode->setPrev(dst);
                dst = newNode;
                src = src->next();
            }
            dst->setNext(dstEnd);
            dstEnd->setPrev(dst);
        }

        SizeType addRangeCounted(Node* dst, Node* dstEnd, const Node* src, const Node* srcEnd)
        {
            SizeType count = 0;
            while(src != srcEnd) {
                Node* newNode = construct(src->value());
                dst->setNext(newNode);
                newNode->setPrev(dst);
                dst = newNode;
                src = src->next();
                ++count;
            }
            dst->setNext(dstEnd);
            dstEnd->setPrev(dst);
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
            m_sentinel.setPrev(getSentinel());
        }

        Node* tail() { return static_cast<Node*>(m_sentinel.prev()); } //prev of sentinel is tail
        const Node* tail() const { return static_cast<Node*>(m_sentinel.prev()); }

        Node* head() { return static_cast<Node*>(m_sentinel.next()); } //next of sentinel is head
        const Node* head() const { return static_cast<Node*>(m_sentinel.next()); }
    };

}