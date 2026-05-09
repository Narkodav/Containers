#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include "Utilities/Concepts.h"
#include "PointerStorage/Vector.h"
#include "Lists/BidirectionalList.h"

// allocators that can accept any type castable to uintptr_t as a pool pointer
namespace Memory::ExternalMetadataAllocators {

    class BuddyAllocatorBase {
    private:
        static constexpr size_t s_minAlign = sizeof(uintptr_t);

        uintptr_t m_data = 0;
        size_t    m_size = 0;
        size_t    m_minBlock = 0;
        size_t    m_maxLevel = 0;

        Containers::Vector<Containers::BidirectionalList<uintptr_t>> m_freeLists = {};
        Containers::Vector<uint8_t> m_allocLevel = {};   // per min block: 0=free, otherwise level+1

    public:
        BuddyAllocatorBase() = default;
        ~BuddyAllocatorBase() = default;

        BuddyAllocatorBase(const BuddyAllocatorBase&) = delete;
        BuddyAllocatorBase& operator=(const BuddyAllocatorBase&) = delete;

        BuddyAllocatorBase(BuddyAllocatorBase&&) = default;
        BuddyAllocatorBase& operator=(BuddyAllocatorBase&&) = default;

        BuddyAllocatorBase(uintptr_t memory, size_t heapSize, size_t minBlock = s_minAlign) {
            assign(memory, heapSize, minBlock);
        }

        void assign(uintptr_t memory, size_t heapSize, size_t minBlock = s_minAlign) {
            if (heapSize < 4096)
                throw std::runtime_error("Invalid pool");

            minBlock = std::max(minBlock, s_minAlign);
            minBlock = alignPow2(minBlock);
            if (minBlock > heapSize)
                throw std::runtime_error("Heap size smaller than minimum allocation block size");

            size_t minBlocks = heapSize / minBlock;
            auto levels = log2Floor(minBlocks) + 1;

            size_t allocBytes = minBlocks * sizeof(uint8_t);

            m_data = memory;
            m_size = heapSize;
            m_minBlock = minBlock;
            m_maxLevel = levels - 1;

            m_freeLists.reserve(levels);
            m_freeLists.resize(levels);
            m_allocLevel.reserve(allocBytes);
            m_allocLevel.resize(allocBytes);

            std::memset(m_allocLevel.data(), 0, m_allocLevel.size());

            m_freeLists[0].pushFront(memory);
            return;
        }

        static size_t alignHeapSize(size_t heapSize, size_t minBlockSize = s_minAlign) {
            if (heapSize < 4096)
                throw std::runtime_error("Invalid pool");

            minBlockSize = std::max(minBlockSize, s_minAlign);
            minBlockSize = alignPow2(minBlockSize);

            size_t minBlocks = heapSize / minBlockSize;
            auto levels = log2Floor(minBlocks) + 1;
            heapSize = std::pow(2, levels) * minBlockSize;
            return heapSize;
        }

        size_t getUsableSize() const { return m_size; }

        uintptr_t allocate(size_t size) {
            return allocateImpl(size);
        }

        void deallocate(uintptr_t ptr) {
            deallocateImpl(ptr);
        }

    private:
        static size_t alignPow2(size_t x) {
            if constexpr (sizeof(size_t) == 8)
            {
                x--; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; x |= x >> 32; return x + 1;
            }
            else if constexpr (sizeof(size_t) == 4)
            {
                x--; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; return x + 1;
            }
        }
        static size_t align8(size_t x) { return (x + 7) & ~static_cast<size_t>(7); }
        static size_t log2Floor(size_t x) { size_t r = 0; while (x >>= 1) r++; return r; }

        size_t levelFor(size_t block) const {
            return log2Floor(m_size / block);
        }

        size_t minIndex(uintptr_t p) const {
            return (p - m_data) / m_minBlock;
        }

        uintptr_t getBuddy(uintptr_t p, size_t level) const {
            uintptr_t offset = p - m_data;
            uintptr_t size = m_size >> level;
            return m_data + (offset ^ size);
        }

        auto inFreeList(size_t lvl, uintptr_t p) {
            auto& list = m_freeLists[lvl];
            for (auto it = list.begin(); it != list.end(); ++it)
                if (*it == p) return it;
            return list.end();
        }

        uintptr_t allocateImpl(size_t size) {
            if (!size || size > m_size) return std::numeric_limits<size_t>::max();

            size = std::max(size, m_minBlock);
            size = alignPow2(size);
            //size = ((size - 1) / m_minBlock + 1) * m_minBlock; //allign to N * m_minBlock


            size_t target = levelFor(size);
            size_t lvl = target;
            while (true) {
                if (!m_freeLists[lvl].empty()) break;
                if (lvl == 0) return std::numeric_limits<size_t>::max();
                --lvl;
            }

            uintptr_t block = m_freeLists[lvl].front();
            m_freeLists[lvl].popFront();

            while (lvl < target) {
                ++lvl;
                size_t half = m_size >> lvl;
                uintptr_t buddy = block + half;
                m_freeLists[lvl].pushFront(buddy);
            }

            m_allocLevel[minIndex(block)] = uint8_t(target + 1);
            return block;
        }

        void deallocateImpl(uintptr_t ptr) {
            if (!ptr) return;

            size_t idx = minIndex(ptr);
            uint8_t lvlp = m_allocLevel[idx];
            if (!lvlp) throw std::runtime_error("Double free");

            int level = lvlp - 1;
            m_allocLevel[idx] = 0;

            uintptr_t block = ptr;

            while (level > 0) {
                uintptr_t buddy = getBuddy(block, level);
                auto it = inFreeList(level, buddy);
                if (it == m_freeLists[level].end()) break;
                m_freeLists[level].erase(it);
                block = std::min(block, buddy);
                level--;
            }
            m_freeLists[level].pushFront(block);
        }
    };


    class BuddyBlockAllocator {
    private:
        size_t m_blockCount;
        size_t m_levelCount;
        Containers::Vector<uint64_t> m_freeListSizes = {};
        Containers::Vector<uint64_t> m_freeLists = {};
        Containers::Vector<uint8_t> m_allocLevel = {};   // per block: std::numeric_limits<uint8_t>::max()=free, otherwise level

    public:

        BuddyBlockAllocator() = default;
        ~BuddyBlockAllocator() = default;

        BuddyBlockAllocator(const BuddyBlockAllocator&) = default;
        BuddyBlockAllocator& operator=(const BuddyBlockAllocator&) = default;

        BuddyAllBuddyBlockAllocatorocatorBase(BuddyBlockAllocator&&) = default;
        BuddyBlockAllocator& operator=(BuddyBlockAllocator&&) = default;

        BuddyBlockAllocator(size_t blockCount) {
            assign(blockCount);
        }

        void assign(size_t blockCount) {
            m_blockCount = alignNextPow2(blockCount);
            m_levelCount = log2Floor(m_blockCount) + 1;

            size_t freeListsBitCount = 0;
            for(size_t i = 0; i < m_levelCount; ++i) freeListsBitCount += pow(2, i);

            m_freeLists.resize((freeListsBitCount - 1) / (sizeof(uint64_t) * 8) + 1, 0);
            m_freeListSizes.resize(m_levelCount, 0);
            m_allocLevel.resize(m_blockCount, std::numeric_limits<uint8_t>::max());

            m_freeLists[0] = 1;
            m_freeListSizes[0] = 1;
            return;
        }

        size_t allocate(size_t blockCount) {
            if (!blockCount || blockCount > m_blockCount) return std::numeric_limits<size_t>::max();

            blockCount = alignNextPow2(blockCount);

            size_t targetLevel = m_levelCount - log2Floor(blockCount);
            size_t lvl = targetLevel;
            for(;;--lvl) {
                if (m_freeListSizes[lvl] != 0) break;
                if (lvl == 0) return std::numeric_limits<size_t>::max();
            }

            size_t block = m_freeLists[lvl].front();
            m_freeLists[lvl].popFront();

            while (lvl < targetLevel) {
                size_t buddy = getBuddy(block, ++lvl);
                m_freeLists[lvl].pushFront(buddy);
            }

            m_allocLevel[block] = uint8_t(targetLevel);
            return block;
        }

        void deallocate(size_t block) {
            if (block == std::numeric_limits<size_t>::max()) return;

            uint8_t level = m_allocLevel[block];
            if (level == std::numeric_limits<uint8_t>::max()) throw std::runtime_error("Double free");
            m_allocLevel[block] = std::numeric_limits<uint8_t>::max();

            while (level > 0) {
                size_t buddy = getBuddy(block, level);
                auto it = inFreeList(level, buddy);
                if (it == m_freeLists[level].end()) break;
                m_freeLists[level].erase(it);
                block = std::min(block, buddy);
                level--;
            }
            m_freeLists[level].pushFront(block);
        }

    private:

        bool getStateAt(size_t level, size_t index) {
            size_t indexGlobal = 1 << level + index;
            
        }

        size_t popBlock(size_t level) {
            size_t startIndex = 1 << level;

        }

        static size_t alignNextPow2(size_t x) {
            x--;
            x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
            if constexpr (sizeof(size_t) == 8) {
                x |= x >> 32;
            }
            return x + 1;
        }
        static size_t align8(size_t x) { return (x + 7) & ~static_cast<size_t>(7); }
        static size_t log2Floor(size_t x) { size_t r = 0; while (x >>= 1) r++; return r; }

        size_t getBuddy(size_t block, size_t level) const {
            size_t size = m_blockCount >> level;
            return block ^ size;
        }

        auto inFreeList(size_t lvl, size_t block) {
            auto& list = m_freeLists[lvl];
            for (auto it = list.begin(); it != list.end(); ++it)
                if (*it == block) return it;
            return list.end();
        }

    }

}
