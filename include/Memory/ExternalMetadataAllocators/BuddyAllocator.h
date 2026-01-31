#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include "../../Utilities/Concepts.h"
#include "../../PointerStorage/Vector.h"
#include "../../Lists/BidirectionalList.h"

// allocators that can accept any type castable to uintptr_t as a pool pointer
namespace Memory::ExternalMetadataAllocators {

    class BuddyAllocatorBase {
    public:

    private:
        static constexpr size_t s_minAlign = sizeof(uintptr_t);

        uintptr_t m_data = 0;
        size_t    m_size = 0;
        size_t    m_minBlock = 0;
        size_t    m_maxLevel = 0;

        Containers::Vector<Containers::BidirectionalList<uintptr_t>> m_freeLists = {};
        Containers::Vector<uint8_t> m_allocLevel = {};   // per min block: 0=free, otherwise level+1
        size_t m_metadataSize = 0;

    public:
        BuddyAllocatorBase() = default;
        ~BuddyAllocatorBase() = default;

        BuddyAllocatorBase(const BuddyAllocatorBase&) = delete;
        BuddyAllocatorBase& operator=(const BuddyAllocatorBase&) = delete;

        BuddyAllocatorBase(BuddyAllocatorBase&&) = default;
        BuddyAllocatorBase& operator=(BuddyAllocatorBase&&) = default;

        void assign(uintptr_t memory, size_t heapSize, size_t minBlock = s_minAlign) {
            if (!memory || heapSize < 4096)
                throw std::runtime_error("Invalid pool");

            minBlock = std::max(minBlock, s_minAlign);
            minBlock = alignPow2(minBlock);

            size_t minBlocks = heapSize / minBlock;
            auto levels = log2Floor(minBlocks);

            size_t allocBytes = minBlocks * sizeof(uint8_t);

            m_data = reinterpret_cast<uintptr_t>(memory);
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

        static size_t computeMetadataSize(size_t totalSize, size_t minBlockSize = s_minAlign) {
            size_t minBlocks = totalSize / minBlockSize;
            int levels = (int)log2Floor(minBlocks) + 1;

            size_t freeListBytes = levels * sizeof(Containers::BidirectionalList<uintptr_t>);
            size_t allocBytes = minBlocks * sizeof(uint8_t);
            size_t meta = align8(freeListBytes + allocBytes);
            return meta;
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
            return (p ^ (m_size >> level));
        }

        auto inFreeList(size_t lvl, uintptr_t p) {
            auto& list = m_freeLists[lvl];
            for (auto it = list.begin(); it != list.end(); ++it)
                if (*it == p) return it;
            return list.end();
        }

        uintptr_t allocateImpl(size_t size) {
            if (!size || size > m_size) return 0;

            size = std::max(size, m_minBlock);
            size = alignPow2(size);

            size_t target = levelFor(size);

            for (size_t lvl = target; lvl != std::numeric_limits<size_t>::max(); --lvl) {
                if (m_freeLists[lvl].empty()) continue;

                auto& list = m_freeLists[lvl];
                uintptr_t block = *list.begin();
                list.popFront();

                while (lvl < target) {
                    lvl++;
                    size_t half = m_size >> lvl;
                    uintptr_t buddy = block + half;
                    m_freeLists[lvl].pushFront(buddy);
                }

                m_allocLevel[minIndex(block)] = uint8_t(target + 1);
                return block;
            }
            return 0;
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

}
