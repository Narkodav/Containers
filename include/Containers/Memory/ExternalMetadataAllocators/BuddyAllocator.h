#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include "Containers/Utilities/Concepts.h"
#include "Containers/PointerStorage/Vector.h"
#include "Containers/Lists/BidirectionalList.h"

// allocators that can accept any type castable to uintptr_t as a pool pointer
namespace Memory::ExternalMetadataAllocators {

    namespace Detail {
        template<std::integral T>
        inline size_t countTrailingZeroesFallback(T word) {
            word |= 1 << (sizeof(T) * 8 - 1);
            size_t tz = 0;
            while ((word & 1) == 0) {
                word >>= 1;
                ++tz;
            }
            return tz;
        }

        template<std::integral T>
        inline size_t countTrailingZeroes(T word) {
            return countTrailingZeroesFallback(word);
        }

        template<>
        inline size_t countTrailingZeroes<uint64_t>(uint64_t word) {
        #if defined(__GNUG__) || defined(__clang__)
            return __builtin_ctzll(word);
        #elif defined(_MSC_VER)
            unsigned long tz;
            _BitScanForward64(&tz, word);
            return tz;
        #else
            return countTrailingZeroesFallback(word);
        #endif
        }

        template<>
        inline size_t countTrailingZeroes<uint32_t>(uint32_t word) {
        #if defined(__GNUG__) || defined(__clang__)
            return __builtin_ctz(word);
        #elif defined(_MSC_VER)
            unsigned long tz;
            _BitScanForward(&tz, word);
            return tz;
        #else
            return countTrailingZeroesFallback(word);
        #endif
        }

        template<std::integral T>
        struct BroadcastPattern {};

        template<>
        struct BroadcastPattern<uint64_t> {
            static inline const uint64_t s_pattern = 0x5555555555555555ull;
        };

        template<>
        struct BroadcastPattern<uint32_t> {
            static inline const uint32_t s_pattern = 0x55555555u;
        };

        // UB at x = 0
        inline size_t alignNextPow2(size_t x) {
            x--;
            x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
            if constexpr (sizeof(size_t) == 8) {
                x |= x >> 32;
            }
            return x + 1;
        }
        inline size_t log2Floor(size_t x) { size_t r = 0; while (x >>= 1) r++; return r; }
    }

    class BuddyBlockAllocator {
    public:

        template<std::integral Limb>
        class Tree {
        public:
            enum class NodeState : uint8_t {
                Free = 0,
                Used = 1,
                Split = 2,
                Unused = 3,
            };

        private:
            Containers::Vector<Limb> m_tree;

        public:
            Tree() = default;
            ~Tree() = default;

            Tree(const Tree&) = default;
            Tree& operator=(const Tree&) = default;

            Tree(Tree&&) = default;
            Tree& operator=(Tree&&) = default;

            void init(size_t levelCount) {
                size_t bitCount = getNodeLevelOffset(levelCount + 1) * 2; // two bits per block
                size_t byteCount = (bitCount - 1) / 8 + 1;
                size_t limbCount = (byteCount - 1) / sizeof(Limb) + 1;
                m_tree.resize(limbCount, std::numeric_limits<Limb>::max());
                m_tree[0] = (m_tree[0] >> 2) << 2;
            }

            NodeState get(size_t lvl, size_t index) const {
                uint8_t state = 0;
                auto [bitIndex, limbIndex] = getInternalIndex(lvl, index);
                state = (m_tree[limbIndex] >> bitIndex) & 0b11;
                return static_cast<NodeState>(state);
            }

            void set(size_t lvl, size_t index, NodeState state) {
                auto [bitIndex, limbIndex] = getInternalIndex(lvl, index);
                auto slot = static_cast<Limb>(0b11) << bitIndex;
                auto mask = static_cast<Limb>(state) << bitIndex;
                m_tree[limbIndex] = (m_tree[limbIndex] & ~slot) | mask;
            }

            size_t findFreeAtLevel(size_t lvl) const {
                auto [startBitIndex, startLimbIndex] = getInternalIndex(lvl, 0);
                auto [endBitIndex, limbEnd] = getInternalIndex(lvl + 1, 0);
                size_t startNodeIndex = startBitIndex >> 1;
                size_t endNodeIndex = endBitIndex >> 1;
                size_t limbIndex = startLimbIndex;
                for(;limbIndex < limbEnd; ++limbIndex) {
                    auto index = findPattern(m_tree[limbIndex] >> startBitIndex, static_cast<uint8_t>(NodeState::Free));
                    if(index < sizeof(Limb) * 4 - startNodeIndex) return limbIndex * sizeof(Limb) * 4 + index + startNodeIndex - getNodeLevelOffset(lvl);
                    startNodeIndex = 0;
                    startBitIndex = 0;
                }
                auto index = findPattern(m_tree[limbIndex] >> startBitIndex, static_cast<uint8_t>(NodeState::Free));
                if(index < endNodeIndex - startNodeIndex) return limbIndex * sizeof(Limb) * 4 + index + startNodeIndex - getNodeLevelOffset(lvl);
                return std::numeric_limits<size_t>::max();
            }

            static size_t getCountAtLevel(size_t lvl) {
                return 1 << lvl;
            }

        private:
            std::pair<size_t, size_t> getInternalIndex(size_t lvl, size_t index) const {
                size_t nodeIndex = getNodeLevelOffset(lvl) + index;
                size_t globalBitIndex = nodeIndex * 2;
                size_t bitIndex = globalBitIndex % (sizeof(Limb) * 8);
                size_t limbIndex = globalBitIndex / (sizeof(Limb) * 8);
                return { bitIndex, limbIndex };
            }

            static size_t findPattern(Limb limb, uint8_t pattern) {
                Limb broadcast = Detail::BroadcastPattern<Limb>::s_pattern * pattern;

                Limb y = limb ^ broadcast;

                Limb lo = y & Detail::BroadcastPattern<Limb>::s_pattern;
                Limb hi = (y >> 1) & Detail::BroadcastPattern<Limb>::s_pattern;

                Limb matches = ~(lo | hi) & Detail::BroadcastPattern<Limb>::s_pattern;

                if (!matches) return std::numeric_limits<size_t>::max();

                return Detail::countTrailingZeroes(matches) >> 1;
            }

            static size_t getNodeLevelOffset(size_t level) {
                return (size_t{1} << level) - 1;
            }
        };

    private:
        size_t m_blockCount;
        size_t m_levelCount;
        Containers::Vector<uint64_t> m_freeNodeCounts = {};
        Tree<uint64_t> m_blockStates;

    public:

        BuddyBlockAllocator() = default;
        ~BuddyBlockAllocator() = default;

        BuddyBlockAllocator(const BuddyBlockAllocator&) = default;
        BuddyBlockAllocator& operator=(const BuddyBlockAllocator&) = default;

        BuddyBlockAllocator(BuddyBlockAllocator&&) = default;
        BuddyBlockAllocator& operator=(BuddyBlockAllocator&&) = default;

        BuddyBlockAllocator(size_t blockCount) {
            assign(blockCount);
        }

        void assign(size_t blockCount) {
            if (!blockCount) throw std::runtime_error("Cannot assign 0 blocks");
            m_blockCount = Detail::alignNextPow2(blockCount);
            m_levelCount = Detail::log2Floor(m_blockCount) + 1;

            size_t freeListsBitCount = 0;
            for(size_t i = 0; i < m_levelCount; ++i) freeListsBitCount += 1 << i;

           //m_freeLists.resize((freeListsBitCount - 1) / (sizeof(uint64_t) * 8) + 1, 0);
            m_freeNodeCounts.resize(m_levelCount, 0);
            m_blockStates.init(m_levelCount);
            m_freeNodeCounts[0] = 1;
        }

        size_t allocate(size_t blockCount) {
            if (!blockCount || blockCount > m_blockCount) return std::numeric_limits<size_t>::max();

            blockCount = Detail::alignNextPow2(blockCount);

            size_t targetLevel = m_levelCount - Detail::log2Floor(blockCount) - 1;
            size_t lvl = targetLevel;
            for(;;--lvl) {
                if (m_freeNodeCounts[lvl] != 0) break;
                if (lvl == 0) return std::numeric_limits<size_t>::max();
            }

            size_t i = m_blockStates.findFreeAtLevel(lvl);
            if(i == std::numeric_limits<size_t>::max()) return std::numeric_limits<size_t>::max();

            size_t block = i * (m_blockCount >> lvl);
            --m_freeNodeCounts[lvl];

            if(lvl == targetLevel) {
                m_blockStates.set(lvl, i, Tree<uint64_t>::NodeState::Used);
            }
            else {
                m_blockStates.set(lvl, i, Tree<uint64_t>::NodeState::Split);
                for(++lvl, i <<= 1; lvl < targetLevel; ++lvl, i <<= 1) {
                    ++m_freeNodeCounts[lvl];
                    m_blockStates.set(lvl, i, Tree<uint64_t>::NodeState::Split);
                    m_blockStates.set(lvl, i ^ 1, Tree<uint64_t>::NodeState::Free);
                }
                ++m_freeNodeCounts[lvl];
                m_blockStates.set(lvl, i, Tree<uint64_t>::NodeState::Used);
                m_blockStates.set(lvl, i ^ 1, Tree<uint64_t>::NodeState::Free);
            }
            return block;
        }

        void deallocate(size_t block) {
            if (block == std::numeric_limits<size_t>::max()) throw std::runtime_error("Deallocating an invalid block");

            size_t lvl = minLevelFromBlock(block);
            size_t i;
            while(true) {                
                i = block / (m_blockCount >> lvl);
                if(m_blockStates.get(lvl, i) == Tree<uint64_t>::NodeState::Used) break;
                ++lvl;
                if(lvl >= m_levelCount) throw std::runtime_error("Deallocating an invalid block");
            }

            while (lvl > 0) {
                size_t buddy = i ^ 1;
                if (m_blockStates.get(lvl, buddy) != Tree<uint64_t>::NodeState::Free)
                    break;
                m_blockStates.set(lvl, i, Tree<uint64_t>::NodeState::Unused);
                m_blockStates.set(lvl, buddy, Tree<uint64_t>::NodeState::Unused);
                --m_freeNodeCounts[lvl];
                --lvl;
                i >>= 1;
            }

            ++m_freeNodeCounts[lvl];
            m_blockStates.set(lvl, i, Tree<uint64_t>::NodeState::Free);
        }

        void debugPrint() const {
            std::cout << "BuddyBlockAllocator state\n";
            std::cout << "Total blocks: " << m_blockCount
                    << ", Levels: " << m_levelCount << "\n\n";

            for (size_t lvl = 0; lvl < m_levelCount; ++lvl) {
                size_t nodeCount = m_blockStates.getCountAtLevel(lvl);
                size_t blockSize = m_blockCount >> lvl;

                // Indentation to visualize tree structure
                size_t indent = (1ull << (m_levelCount - lvl - 1)) - 1;
                std::cout << "L" << lvl
                        << " (blockSize=" << std::setw(4) << blockSize << ") "
                        << "[free=" << m_freeNodeCounts[lvl] << "] : ";
                for (size_t s = 0; s < indent; ++s) std::cout << "  ";

                for (size_t i = 0; i < nodeCount; ++i) {
                    char c = '?';
                    switch (m_blockStates.get(lvl, i)) {
                        case Tree<uint64_t>::NodeState::Free:   c = 'F'; break;
                        case Tree<uint64_t>::NodeState::Used:   c = 'U'; break;
                        case Tree<uint64_t>::NodeState::Split:  c = 'S'; break;
                        case Tree<uint64_t>::NodeState::Unused: c = 'N'; break;
                    }

                    std::cout << c;

                    // spacing between siblings for readability
                    if (i + 1 < nodeCount) {
                        size_t spacing = (1ull << (m_levelCount - lvl)) - 1;
                        for (size_t s = 0; s < spacing; ++s) std::cout << ' ';
                    }
                }

                std::cout << "\n";
            }

            std::cout << std::endl;
        }

        bool isFree() const {
            return m_freeNodeCounts.empty() || m_freeNodeCounts[0] == 1 && m_blockStates.get(0, 0) == Tree<uint64_t>::NodeState::Free;
        }

    private:

        inline size_t minLevelFromBlock(size_t block)
        {
            if (block == 0)
                return 0;
            return (m_levelCount - 1) - Detail::countTrailingZeroes(block);
        }
    };

    class BuddyAllocatorBase {
    private:
        BuddyBlockAllocator m_impl;
        size_t m_minBlockSize;
        uintptr_t m_heap;
        size_t m_size;
        static inline size_t s_defaultMinBlockSize = 64;

    public:
        BuddyAllocatorBase() = default;
        ~BuddyAllocatorBase() noexcept = default;

        BuddyAllocatorBase(const BuddyAllocatorBase&) = delete;
        BuddyAllocatorBase& operator=(const BuddyAllocatorBase&) = delete;

        BuddyAllocatorBase(BuddyAllocatorBase&&) = default;
        BuddyAllocatorBase& operator=(BuddyAllocatorBase&&) = default;

        void assign(uintptr_t heap, size_t size, size_t minBlockSize = s_defaultMinBlockSize) {
            if(heap % minBlockSize != 0) throw std::runtime_error("Heap must be minBlockSize aligned");
            //if(Detail::alignNextPow2(minBlockSize) != minBlockSize) throw std::runtime_error("Minimal block size must be power of 2");
            size_t blockCount = size / minBlockSize;
            if(Detail::alignNextPow2(blockCount) != blockCount) throw std::runtime_error("Block count must be power of 2");
            m_impl.assign(blockCount);            
            m_minBlockSize = minBlockSize;
            m_heap = heap;
            m_size = size;
        }

        static size_t alignHeapSize(size_t size, size_t minBlockSize = s_defaultMinBlockSize) {
            if(size == 0) return minBlockSize;
            size_t blockCount = (size - 1) / minBlockSize + 1;
            blockCount = Detail::alignNextPow2(blockCount);
            return blockCount * minBlockSize;
        }

        uintptr_t allocate(size_t size) {
            if(size == 0) return 0;
            size_t blockCount = (size - 1) / m_minBlockSize + 1;
            size_t allocation = m_impl.allocate(blockCount);
            if(allocation == std::numeric_limits<size_t>::max()) return std::numeric_limits<uintptr_t>::max();
            return m_heap + allocation * m_minBlockSize;
        }

        void deallocate(uintptr_t ptr) {
            auto offset = ptr - m_heap;
            if(offset % m_minBlockSize != 0) throw std::runtime_error("Unaligned pointer");
            if(offset >= m_size) throw std::runtime_error("Invalid pointer");
            auto index = offset / m_minBlockSize;
            m_impl.deallocate(index);
        }

        void debugPrint() const { m_impl.debugPrint(); }
    };
}
