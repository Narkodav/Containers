#pragma once
#include "Utilities/ByteArray.h"
#include "Lists/Lists.h"
#include <memory>
#include <stdexcept>

template <typename T>
class MemoryPool
{
public:
    static inline const size_t Byte = 1;
    static inline const size_t KiloByte = 1024;
    static inline const size_t MegaByte = KiloByte * KiloByte;
    static inline const size_t GigaByte = KiloByte * MegaByte;
    static inline const size_t TeraByte = KiloByte * GigaByte;

    //these are just for completeness sake
    static inline const size_t PetaByte = KiloByte * TeraByte;
    static inline const size_t ExaByte = KiloByte * PetaByte;
    static inline const size_t ZettaByte = KiloByte * ExaByte;
    static inline const size_t YottaByte = KiloByte * ZettaByte;
    static inline const size_t BrontoByte = KiloByte * YottaByte;

    static constexpr size_t MAX_ALLOCATION_SIZE = 128 * MegaByte;

    struct Block {
        size_t offset;
        size_t size;
    };

    class Allocation
    {
    private:
        ListDoubleSidedTailed<Block>::Node* m_memoryBlock = nullptr;
        T* m_data = nullptr;
        MemoryPool* m_owner = nullptr;

        Allocation(ListDoubleSidedTailed<Block>::Node* memoryBlock, T* data, MemoryPool* owner) :
            m_memoryBlock(memoryBlock), m_data(data), m_owner(owner) {
        };

    public:
        Allocation() = default;

        ~Allocation()
        {
            if (m_owner != nullptr)
                m_owner->deallocate(m_memoryBlock, m_data);
        }

        Allocation(Allocation&& other) noexcept :
            m_memoryBlock(other.m_memoryBlock),
            m_data(other.m_data),
            m_owner(other.m_owner)
        {
            other.m_memoryBlock = nullptr;
            other.m_data = nullptr;
            other.m_owner = nullptr;
        }

        Allocation& operator=(Allocation&& other) noexcept
        {
            if (this != &other)
            {
                // Clean up current resources
                if (m_owner != nullptr)
                {
                    m_owner->deallocate(m_memoryBlock, m_data);
                }

                // Take ownership of other's resources
                m_memoryBlock = other.m_memoryBlock;
                m_data = other.m_data;
                m_owner = other.m_owner;

                // Null out other's pointers
                other.m_memoryBlock = nullptr;
                other.m_data = nullptr;
                other.m_owner = nullptr;
            }
            return *this;
        }

        Allocation(const Allocation&) noexcept = delete;
        Allocation& operator=(const Allocation&) noexcept = delete;

        T& operator*() { return *m_data; }
        T* operator->() { return m_data; }

        const T& operator*() const { return *m_data; }
        const T* operator->() const { return m_data; }

        explicit operator T& () { return *m_data; }
        explicit operator const T& () const { return *m_data; }

        void deallocate()
        {
            if (m_owner != nullptr)
                m_owner->deallocate(m_memoryBlock, m_data);
        }

        bool isAllocated() const { return m_owner != nullptr; };

        friend class MemoryPool;
    };

    class ArrayAllocation : public Allocation
    {
    private:
        using Allocation::m_data;
        using Allocation::m_memoryBlock;
        using Allocation::m_owner;
        using Allocation::operator*;
        using Allocation::operator->;
        using Allocation::operator T&;
        using Allocation::operator const T&;

    public:
        using Allocation::Allocation;
        using Allocation::deallocate;
        using Allocation::isAllocated;

        T& operator[](size_t index) {
            return m_data[index];
        }

        const T& operator[](size_t index) const {
            return m_data[index];
        }

        T* data() { return m_data; };
        const T* data() const { return m_data; };

        friend class MemoryPool;
    };

    struct FragmentationMetrics
    {
        size_t totalSize;
        size_t freeBlocks;
        size_t freeSize;
        size_t usedBlocks;
        size_t usedSize;
        size_t largestFreeBlock;
        size_t largestUsedBlock;
        double averageFreeBlockSize;
        double averageUsedBlockSize;
        size_t fragmentationRatio;
    };

    static inline const size_t typeSize = sizeof(T);
    static inline const size_t typeAlign = alignof(T);

private:
    ByteArray m_data;
    ListDoubleSidedTailed<Block> m_freeBlocks;
    ListDoubleSidedTailed<Block> m_usedBlocks;
    size_t m_size = 0;
    size_t m_freeSize = 0;
    size_t m_freeBlockAmount = 0;
    size_t m_usedBlockAmount = 0;

public:

    MemoryPool(size_t size) : m_data(size * typeSize), m_size(size * typeSize),
        m_freeBlockAmount(1), m_usedBlockAmount(0)
    {
        static_assert(typeSize > 0, "Type size must be greater than 0");
        static_assert(typeAlign > 0, "Type alignment must be greater than 0");
        static_assert(typeSize < MAX_ALLOCATION_SIZE, "Type size must be less than or equal to MAX_ALLOCATION_SIZE");

        if (typeSize > m_size) {
            throw std::runtime_error("Size of type is larger than the memory pool");
        }

        m_freeBlocks.insertFront(Block{ 0, m_size });
    }

    ~MemoryPool()
    {
        m_data.destroy();
    }

    MemoryPool(const MemoryPool&) noexcept = delete;
    MemoryPool& operator=(const MemoryPool&) noexcept = delete;

    MemoryPool(MemoryPool&&) noexcept = delete;
    MemoryPool& operator=(MemoryPool&&) noexcept = delete;

    //MemoryPool(MemoryPool&& other) noexcept
    //{
    //    m_data = std::move(other.m_data);
    //    m_freeBlocks = std::exchange(other.m_freeBlocks, ListDoubleSidedTailed<Block>());
    //    m_usedBlocks = std::exchange(other.m_usedBlocks, ListDoubleSidedTailed<Block>());
    //    m_size = std::exchange(other.m_size, 0);
    //    m_freeSize = std::exchange(other.m_freeSize, 0);
    //    m_freeBlockAmount = std::exchange(other.m_freeBlockAmount, 0);
    //    m_usedBlockAmount = std::exchange(other.m_usedBlockAmount, 0);
    //}

    //MemoryPool& operator=(MemoryPool&& other) noexcept
    //{
    //    if (this == &other)
    //        return *this;
    //    m_data.destroy();
    //    m_data = std::move(other.m_data);
    //    m_freeBlocks = std::exchange(other.m_freeBlocks, ListDoubleSidedTailed<Block>());
    //    m_usedBlocks = std::exchange(other.m_usedBlocks, ListDoubleSidedTailed<Block>());
    //    m_size = std::exchange(other.m_size, 0);
    //    m_freeSize = std::exchange(other.m_freeSize, 0);
    //    m_freeBlockAmount = std::exchange(other.m_freeBlockAmount, 0);
    //    m_usedBlockAmount = std::exchange(other.m_usedBlockAmount, 0);
    //    return *this;
    //}

    //faster than best fit, can lead to fragmentation, use when a lot of memory is available and speed is a concern
    template<typename... Args>
    Allocation allocateFirstFit(Args&&... args)
    {
        auto block = allocateBlockFirstFit(typeSize, typeAlign);
        if (block != nullptr)
        {
            m_data.emplace<T>(block->m_data.offset, std::forward<Args>(args)...);
            return Allocation(block, m_data.get<T>(block->m_data.offset), this);
        }
        return Allocation(nullptr, nullptr, this);
    }

    template<typename... Args>
    ArrayAllocation allocateArrayFirstFit(size_t count, Args&&... args)
    {        
        size_t totalSize = count * typeSize;

        if (totalSize > MAX_ALLOCATION_SIZE) {
            throw std::bad_alloc();
        }

        auto block = allocateBlockFirstFit(totalSize, typeAlign);
        if (block != nullptr)
        {
            m_data.emplaceRange<T>(count, block->m_data.offset, std::forward<Args>(args)...);
            return ArrayAllocation(block, m_data.get<T>(block->m_data.offset), this);
        }
        return ArrayAllocation(nullptr, nullptr, this);
    }

    template<typename... Args>
    Allocation allocateBestFit(Args&&... args)
    {
        auto block = allocateBlockBestFit(typeSize, typeAlign);
        if (block != nullptr)
        {
            m_data.emplace<T>(block->m_data.offset, std::forward<Args>(args)...);
            return Allocation(block, m_data.get<T>(block->m_data.offset), this);
        }
        return Allocation(nullptr, nullptr, this);
    }

    template<typename... Args>
    ArrayAllocation allocateArrayBestFit(size_t count, Args&&... args)
    {
        size_t totalSize = count * typeSize;

        if (totalSize > MAX_ALLOCATION_SIZE) {
            throw std::bad_alloc();
        }

        auto block = allocateBlockBestFit(totalSize, typeAlign);
        if (block != nullptr)
        {
            m_data.emplaceRange<T>(count, block->m_data.offset, std::forward<Args>(args)...);
            return ArrayAllocation(block, m_data.get<T>(block->m_data.offset), this);
        }
        return ArrayAllocation(nullptr, nullptr, this);
    }

    size_t getSize() const { return m_size; }
    size_t getFree() const { return m_freeSize; }
    size_t getFreeBlockAmount() const { return m_freeBlockAmount; }
    size_t getUsedBlockAmount() const { return m_usedBlockAmount; }
    size_t getUsed() const { return m_size - m_freeSize; }

    template<typename U = T>
    U* data() { return m_data.data<U>(); };

    template<typename U = T>
    const U* data() const { return m_data.data<U>(); };

    FragmentationMetrics getFragmentationMetrics() const {
        FragmentationMetrics metrics{};
        metrics.totalSize = m_size;
        metrics.freeBlocks = m_freeBlockAmount;
        metrics.usedBlocks = m_usedBlockAmount;
        metrics.largestFreeBlock = 0;
        metrics.largestUsedBlock = 0;
        metrics.averageFreeBlockSize = 0;
        metrics.averageUsedBlockSize = 0;
        metrics.freeSize = m_freeSize;
        metrics.usedSize = m_size - m_freeSize;

        // Collect metrics from free blocks
        for (auto* block = m_freeBlocks.getFront();
            block != nullptr;
            block = m_freeBlocks.iterateNext(block)) {
            metrics.largestFreeBlock = std::max(
                metrics.largestFreeBlock,
                block->m_data.size
            );
            metrics.averageFreeBlockSize += block->m_data.size;
        }
        metrics.averageFreeBlockSize /= metrics.freeBlocks;

        // Collect metrics from used blocks
        for (auto* block = m_usedBlocks.getFront();
            block != nullptr;
            block = m_freeBlocks.iterateNext(block)) {
            metrics.largestUsedBlock = std::max(
                metrics.largestUsedBlock,
                block->m_data.size
            );
            metrics.averageUsedBlockSize += block->m_data.size;
        }
        metrics.averageUsedBlockSize /= metrics.usedBlocks;

        metrics.fragmentationRatio = static_cast<double>(metrics.freeBlocks) /
            (static_cast<double>(metrics.freeSize) / m_size);

        return metrics;
    }

private:

    void deallocate(ListDoubleSidedTailed<Block>::Node* memoryBlock, T* data)
    {
        Block block = memoryBlock->m_data;
        m_freeSize += block.size;
        m_usedBlocks.deleteNode(memoryBlock);
        --m_usedBlockAmount;
        m_data.erase<T>(block.size / sizeof(T), block.offset);
        mergeBlocks(block);
    }

    void mergeBlocks(Block& memoryBlock)
    {
        typename ListDoubleSidedTailed<Block>::Node* current;
        for (current = m_freeBlocks.getFront();
            current != nullptr; current = m_freeBlocks.iterateNext(current))
            if (current->m_data.offset > memoryBlock.offset)
            {
                m_freeBlocks.insertPrevious(current, memoryBlock);
                current = current->m_previous;
                ++m_freeBlockAmount;
                break;
            }
        if (current == nullptr)
        {
            m_freeBlocks.insertBack(memoryBlock);
            current = m_freeBlocks.getBack();
            ++m_freeBlockAmount;
        }
        else
        {
            typename ListDoubleSidedTailed<Block>::Node* next = current->m_next;

            if (next != nullptr && memoryBlock.offset + memoryBlock.size == next->m_data.offset) {
                memoryBlock.size += next->m_data.size;
                m_freeBlocks.deleteNode(next);
                --m_freeBlockAmount;
            }
        }
        typename ListDoubleSidedTailed<Block>::Node* previous = m_freeBlocks.iteratePrevious(current);

        if (previous != nullptr && previous->m_data.offset + previous->m_data.size == memoryBlock.offset) {
            previous->m_data.size += memoryBlock.size;
            m_freeBlocks.deleteNode(current);
            --m_freeBlockAmount;
        }
    }

    ListDoubleSidedTailed<Block>::Node* allocateBlockFirstFit(const size_t& size, const size_t& alignment)
    {
        for (typename ListDoubleSidedTailed<Block>::Node* current = m_freeBlocks.getFront();
            current != nullptr; current = m_freeBlocks.iterateNext(current))
        {
            if (current->m_data.offset > std::numeric_limits<size_t>::max() - alignment) {
                continue;
            }

            size_t alignedOffset = (current->m_data.offset + alignment - 1) & ~(alignment - 1);
            size_t alignmentPadding = alignedOffset - current->m_data.offset;

            if (size <= std::numeric_limits<size_t>::max() - alignmentPadding &&
                current->m_data.size >= size + alignmentPadding) {
                Block allocatedBlock = Block{ alignedOffset, size };

                if (alignmentPadding > 0)
                {
                    ++m_freeBlockAmount;
                    m_freeBlocks.insertPrevious(current,
                        Block{ current->m_data.offset, alignedOffset - current->m_data.offset });
                }

                current->m_data.offset = alignedOffset + size;
                current->m_data.size -= size + alignmentPadding;
                if (current->m_data.size == 0) {
                    m_freeBlocks.deleteNode(current);
                    --m_freeBlockAmount;
                }
                m_usedBlocks.insertFront(allocatedBlock);
                ++m_usedBlockAmount;
                m_freeSize -= size + alignmentPadding;

                return m_usedBlocks.getFront();
            }
        }
        return nullptr;
    }

    ListDoubleSidedTailed<Block>::Node* allocateBlockBestFit(const size_t& size, const size_t& alignment)
    {
        size_t bestSize = std::numeric_limits<size_t>::max();
        size_t bestAlignedOffset = 0;
        size_t bestAlignmentPadding = 0;
        typename ListDoubleSidedTailed<Block>::Node* best = nullptr;

        for (typename ListDoubleSidedTailed<Block>::Node* current = m_freeBlocks.getFront();
            current != nullptr; current = m_freeBlocks.iterateNext(current))
        {
            if (current->m_data.offset > std::numeric_limits<size_t>::max() - alignment) {
                continue;
            }

            size_t alignedOffset = (current->m_data.offset + alignment - 1) & ~(alignment - 1);
            size_t alignmentPadding = alignedOffset - current->m_data.offset;

            if (size <= std::numeric_limits<size_t>::max() - alignmentPadding &&
                current->m_data.size >= size + alignmentPadding && bestSize > current->m_data.size) {
                bestSize = current->m_data.size;
                best = current;
                bestAlignedOffset = alignedOffset;
                bestAlignmentPadding = alignmentPadding;
            }
        }

        if (best != nullptr)
        {
            Block allocatedBlock = Block{ bestAlignedOffset, size };

            if (bestAlignmentPadding > 0)
            {
                ++m_freeBlockAmount;
                m_freeBlocks.insertPrevious(best,
                    Block{ best->m_data.offset, bestAlignedOffset - best->m_data.offset });
            }

            best->m_data.offset = bestAlignedOffset + size;
            best->m_data.size -= size + bestAlignmentPadding;
            if (best->m_data.size == 0) {
                m_freeBlocks.deleteNode(best);
                --m_freeBlockAmount;
            }
            m_usedBlocks.insertFront(allocatedBlock);
            ++m_usedBlockAmount;
            m_freeSize -= size + bestAlignmentPadding;
            return m_usedBlocks.getFront();
        }

        return nullptr;
    }

    friend class Allocation;
    friend class ArrayAllocation;
};

