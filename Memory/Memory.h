#pragma once
#include "Utilities/ByteArray.h"
#include "Lists/Lists.h"
#include <memory>
#include <stdexcept>

namespace Containers {
    class Memory
    {
    public:
        static inline const size_t Byte = 1;
        static inline const size_t KiloByte = Byte * 1024;
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

        template<typename T>
        class Allocation
        {
        private:
            ListOneSided<Block>::Node* m_memoryBlock = nullptr;
            T* m_data = nullptr;
            Memory* m_owner = nullptr;

            Allocation(ListOneSided<Block>::Node* memoryBlock, T* data, Memory* owner) :
                m_memoryBlock(memoryBlock), m_data(data), m_owner(owner) {
            };

        public:
            Allocation() = default;

            ~Allocation()
            {
                deallocate();
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
                m_owner = nullptr;
            }

            bool isAllocated() const { return m_owner != nullptr; };

            friend class Memory;
        };

        template<typename T>
        class ArrayAllocation : public Allocation<T>
        {
        private:
            using Allocation<T>::m_data;
            using Allocation<T>::m_memoryBlock;
            using Allocation<T>::m_owner;
            using Allocation<T>::operator*;
            using Allocation<T>::operator->;
            using Allocation<T>::operator T&;
            using Allocation<T>::operator const T&;

        public:
            using Allocation<T>::Allocation;
            using Allocation<T>::deallocate;
            using Allocation<T>::isAllocated;

            T& operator[](size_t index) {
                return m_data[index];
            }

            const T& operator[](size_t index) const {
                return m_data[index];
            }

            T* data() { return m_data; };
            const T* data() const { return m_data; };

            friend class Memory;
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
            double fragmentationRatio;
        };

    private:
        ByteArray m_data;
        ListDoubleSidedTailed<Block> m_freeBlocks;
        ListOneSided<Block> m_usedBlocks;
        size_t m_size = 0;
        size_t m_freeSize = 0;
        size_t m_freeBlockAmount = 0;
        size_t m_usedBlockAmount = 0;

    public:

        Memory(size_t size) : m_data(size), m_size(size), m_freeSize(size), m_freeBlockAmount(1), m_usedBlockAmount(0)
        {
            m_freeBlocks.insertFront(Block{ 0, size });
        }

        ~Memory()
        {
            m_data.destroy();
        }

        Memory(const Memory&) noexcept = delete;
        Memory& operator=(const Memory&) noexcept = delete;

        Memory(Memory&&) noexcept = delete;
        Memory& operator=(Memory&&) noexcept = delete;

        //Memory(Memory&& other) noexcept
        //{
        //    m_data = std::move(other.m_data);
        //    m_freeBlocks = std::exchange(other.m_freeBlocks, ListDoubleSidedTailed<Block>());
        //    m_usedBlocks = std::exchange(other.m_usedBlocks, ListDoubleSidedTailed<Block>());
        //    m_size = std::exchange(other.m_size, 0);
        //    m_freeSize = std::exchange(other.m_freeSize, 0);
        //    m_freeBlockAmount = std::exchange(other.m_freeBlockAmount, 0);
        //    m_usedBlockAmount = std::exchange(other.m_usedBlockAmount, 0);
        //}

        //Memory& operator=(Memory&& other) noexcept
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
        template<typename T, typename... Args>
        Allocation<T> allocateFirstFit(Args&&... args)
        {
            constexpr size_t typeSize = sizeof(T);
            constexpr size_t typeAlign = alignof(T);

            if (typeSize == 0) {
                throw std::invalid_argument("Cannot allocate zero bytes");
            }

            if (typeSize > m_size) {
                throw std::runtime_error("Type size is bigger than memory size");
            }

            if (typeSize > MAX_ALLOCATION_SIZE) {
                throw std::runtime_error("Type size exceeds maximum allocation size");
            }

            auto block = allocateBlockFirstFit(typeSize, typeAlign);
            if (block != nullptr)
            {
                m_data.emplace<T>(block->m_data.offset, std::forward<Args>(args)...);
                return Allocation<T>(block, m_data.get<T>(block->m_data.offset), this);
            }
            throw std::runtime_error("No suitable memory block found");
        }

        template<typename T, typename... Args>
        ArrayAllocation<T> allocateArrayFirstFit(size_t count, Args&&... args)
        {
            constexpr size_t typeSize = sizeof(T);
            constexpr size_t typeAlign = alignof(T);
            size_t totalSize = count * typeSize;

            if (count == 0) {
                throw std::invalid_argument("Cannot allocate array of size 0");
            }

            // Check for multiplication overflow
            if (count > std::numeric_limits<size_t>::max() / typeSize) {
                throw std::runtime_error("Total size exceeds size_t max value");
            }

            if (totalSize > MAX_ALLOCATION_SIZE) {
                throw std::runtime_error("Total size exceeds maximum allocation size");
            }

            auto block = allocateBlockFirstFit(totalSize, typeAlign);
            if (block != nullptr)
            {
                m_data.emplaceRange<T>(count, block->m_data.offset, std::forward<Args>(args)...);
                return ArrayAllocation<T>(block, m_data.get<T>(block->m_data.offset), this);
            }
            throw std::runtime_error("No suitable memory block found");
        }

        template<typename T, typename... Args>
        Allocation<T> allocateBestFit(Args&&... args)
        {
            constexpr size_t typeSize = sizeof(T);
            constexpr size_t typeAlign = alignof(T);

            if (typeSize == 0) {
                throw std::invalid_argument("Cannot allocate zero bytes");
            }

            if (typeSize > m_size) {
                throw std::runtime_error("Type size is bigger than memory size");
            }

            if (typeSize > MAX_ALLOCATION_SIZE) {
                throw std::runtime_error("Type size exceeds maximum allocation size");
            }

            auto block = allocateBlockBestFit(typeSize, typeAlign);
            if (block != nullptr)
            {
                m_data.emplace<T>(block->m_data.offset, std::forward<Args>(args)...);
                return Allocation<T>(block, m_data.get<T>(block->m_data.offset), this);
            }
            throw std::runtime_error("No suitable memory block found");
        }

        template<typename T, typename... Args>
        ArrayAllocation<T> allocateArrayBestFit(size_t count, Args&&... args)
        {
            constexpr size_t typeSize = sizeof(T);
            constexpr size_t typeAlign = alignof(T);
            size_t totalSize = count * typeSize;

            if (count == 0) {
                throw std::invalid_argument("Cannot allocate array of size 0");
            }

            // Check for multiplication overflow
            if (count > std::numeric_limits<size_t>::max() / typeSize) {
                throw std::runtime_error("Total size exceeds size_t max value");
            }

            if (totalSize > MAX_ALLOCATION_SIZE) {
                throw std::runtime_error("Total size exceeds maximum allocation size");
            }

            auto block = allocateBlockBestFit(totalSize, typeAlign);
            if (block != nullptr)
            {
                m_data.emplaceRange<T>(count, block->m_data.offset, std::forward<Args>(args)...);
                return ArrayAllocation<T>(block, m_data.get<T>(block->m_data.offset), this);
            }
            throw std::runtime_error("No suitable memory block found");
        }

        size_t getSize() const { return m_size; }
        size_t getFree() const { return m_freeSize; }
        size_t getFreeBlockAmount() const { return m_freeBlockAmount; }
        size_t getUsedBlockAmount() const { return m_usedBlockAmount; }
        size_t getUsed() const { return m_size - m_freeSize; }

        template<typename T = uint8_t>
        T* data() { return m_data.data<T>(); };

        template<typename T = uint8_t>
        const T* data() const { return m_data.data<T>(); };

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
                block = m_usedBlocks.iterateNext(block)) {
                metrics.largestUsedBlock = std::max(
                    metrics.largestUsedBlock,
                    block->m_data.size
                );
                metrics.averageUsedBlockSize += block->m_data.size;
            }
            metrics.averageUsedBlockSize /= metrics.usedBlocks;
            if (m_freeSize != 0)
                metrics.fragmentationRatio = static_cast<double>(metrics.freeBlocks) * m_size /
                (static_cast<double>(m_freeSize) * static_cast<double>(m_freeSize) * 2) * 100;
            else metrics.fragmentationRatio = 0;

            return metrics;
        }

    private:
        template <typename T>
        void deallocate(ListOneSided<Block>::Node* memoryBlock, T* data)
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
            ListDoubleSidedTailed<Block>::Node* current;
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
                ListDoubleSidedTailed<Block>::Node* next = current->m_next;

                if (next != nullptr && memoryBlock.offset + memoryBlock.size == next->m_data.offset) {
                    memoryBlock.size += next->m_data.size;
                    m_freeBlocks.deleteNode(next);
                    --m_freeBlockAmount;
                }
            }
            ListDoubleSidedTailed<Block>::Node* previous = m_freeBlocks.iteratePrevious(current);

            if (previous != nullptr && previous->m_data.offset + previous->m_data.size == memoryBlock.offset) {
                previous->m_data.size += memoryBlock.size;
                m_freeBlocks.deleteNode(current);
                --m_freeBlockAmount;
            }
        }

        ListOneSided<Block>::Node* allocateBlockFirstFit(size_t size, size_t alignment)
        {
            for (ListDoubleSidedTailed<Block>::Node* current = m_freeBlocks.getFront();
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

        ListOneSided<Block>::Node* allocateBlockBestFit(size_t size, size_t alignment)
        {
            size_t bestSize = std::numeric_limits<size_t>::max();
            size_t bestAlignedOffset = 0;
            size_t bestAlignmentPadding = 0;
            ListDoubleSidedTailed<Block>::Node* best = nullptr;

            for (ListDoubleSidedTailed<Block>::Node* current = m_freeBlocks.getFront();
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

        template <typename T>
        friend class Allocation;

        template <typename T>
        friend class ArrayAllocation;
    };

}