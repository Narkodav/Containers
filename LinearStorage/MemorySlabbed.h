#pragma once
#include "Maps/UnorderedMap.h"
#include "Vector.h"

class MemorySlabbed
{
public:
	struct TypeData
	{
		size_t size;
		size_t alignment;

		struct Hasher
		{
			bool operator()(const TypeData& d) const
			{
				return std::hash<size_t>()(d.size) ^ std::hash<size_t>()(d.alignment);
			}
		};
	};

    struct Block {
        size_t offset;
        size_t size;
    };

    struct Slab {
        ByteArray m_data;
        ListDoubleSidedTailed<Block> m_freeBlocks;
        ListDoubleSidedTailed<Block> m_usedBlocks;
        size_t m_size = 0;
        size_t m_freeSize = 0;
        size_t m_freeBlockAmount = 0;
        size_t m_usedBlockAmount = 0;
    };

	static inline const size_t SLAB_SIZE = 4096; //in objects, not in bytes, actual slab size is SLAB_SIZE * typeSize

    template<typename T>
    class Allocation
    {
    private:
        Slab* m_slab = nullptr;
        ListDoubleSidedTailed<Block>::Node* m_memoryBlock = nullptr;
        T* m_data = nullptr;
        Memory* m_owner = nullptr;

        Allocation(Slab* slab, ListDoubleSidedTailed<Block>::Node* memoryBlock, T* data, Memory* owner) :
            m_slab(slab), m_memoryBlock(memoryBlock), m_data(data), m_owner(owner) {
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

private:
	UnorderedMap<TypeData, Vector<Slab>, TypeData::Hasher> m_slabMap;

public:
	
	MemorySlabbed()
	{

	}

	~MemorySlabbed()
	{
        for (auto& slabs : m_slabMap)
            for (auto& slab : slabs.getValue())
                slab.m_data.destroy();
	}

	template<typename T, typename... Args>
	Allocation<T> allocateFirstFit(Args&&... args)
	{
		if (sizeof(T) == 0) {
			throw std::invalid_argument("Cannot allocate zero bytes");
		}

        TypeData typeData = { sizeof(T), alignof(T)};

        auto slabs = m_slabMap.find(typeData);

        if (slabs == m_slabMap.end())
            slabs = m_slabMap.insert(typeData, Vector<Slab>());
        else
        {
            for (auto& slab : slabs->getValue())
            {
                auto block = allocateBlockFirstFit(typeData.size, typeData.alignment,
                    slab.m_freeBlocks, slab.m_usedBlocks, slab.m_freeSize, slab.m_freeBlockAmount,
                    slab.m_usedBlockAmount);

                if (block != nullptr)
                {
                    slab.m_data.emplace<T>(block->m_data.offset, std::forward<Args>(args)...);
                    return Allocation<T>(&slab, block, slab.m_data.get<T>(block->m_data.offset), this);
                }
            }
        }
        slabs->getValue().pushBack(Slab{ ByteArray(SLAB_SIZE * sizeof(T)),
            ListDoubleSidedTailed<Block>{},
            ListDoubleSidedTailed<Block>{},
            SLAB_SIZE * sizeof(T), SLAB_SIZE * sizeof(T), 1, 0 });
        auto& slab = slabs->getValue()[0];
        slab.m_freeBlocks.insertFront(Block{ 0, SLAB_SIZE * sizeof(T) });

        auto block = allocateBlockFirstFit(typeData.size, typeData.alignment,
            slab.m_freeBlocks, slab.m_usedBlocks, slab.m_freeSize, slab.m_freeBlockAmount,
            slab.m_usedBlockAmount);
        slab.m_data.emplace<T>(block->m_data.offset, std::forward<Args>(args)...);
        return Allocation<T>(&slab, block, slab.m_data.get<T>(block->m_data.offset), this);
	}

private:

    template <typename T>
    void deallocate(ListDoubleSidedTailed<Block>::Node* memoryBlock, T* data, Slab* slab)
    {
        Block block = memoryBlock->m_data;
        slab->m_freeSize += block.size;
        slab->m_usedBlocks.deleteNode(memoryBlock);
        --slab->m_usedBlockAmount;
        ByteArray::erase(data);
        mergeBlocks(block, slab->m_freeBlocks, slab->m_freeBlockAmount);
    }

    void mergeBlocks(Block& memoryBlock, ListDoubleSidedTailed<Block>& freeBlocks, size_t& freeBlockAmount)
    {
        ListDoubleSidedTailed<Block>::Node* current;
        for (current = freeBlocks.getFront();
            current != nullptr; current = freeBlocks.iterateNext(current))
            if (current->m_data.offset > memoryBlock.offset)
            {
                freeBlocks.insertPrevious(current, memoryBlock);
                current = current->m_previous;
                ++freeBlockAmount;
                break;
            }
        if (current == nullptr)
        {
            freeBlocks.insertBack(memoryBlock);
            current = freeBlocks.getBack();
            ++freeBlockAmount;
        }
        else
        {
            ListDoubleSidedTailed<Block>::Node* next = current->m_next;

            if (next != nullptr && memoryBlock.offset + memoryBlock.size == next->m_data.offset) {
                memoryBlock.size += next->m_data.size;
                freeBlocks.deleteNode(next);
                --freeBlockAmount;
            }
        }
        ListDoubleSidedTailed<Block>::Node* previous = freeBlocks.iteratePrevious(current);

        if (previous != nullptr && previous->m_data.offset + previous->m_data.size == memoryBlock.offset) {
            previous->m_data.size += memoryBlock.size;
            freeBlocks.deleteNode(current);
            --freeBlockAmount;
        }
    }

    ListDoubleSidedTailed<Block>::Node* allocateBlockFirstFit(size_t size, size_t alignment,
        ListDoubleSidedTailed<Block>& freeBlocks, ListDoubleSidedTailed<Block>& usedBlocks,
        size_t& freeSize, size_t& freeBlockAmount, size_t& usedBlockAmount)
    {
        for (ListDoubleSidedTailed<Block>::Node* current = freeBlocks.getFront();
            current != nullptr; current = freeBlocks.iterateNext(current))
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
                    ++freeBlockAmount;
                    freeBlocks.insertPrevious(current,
                        Block{ current->m_data.offset, alignedOffset - current->m_data.offset });
                }

                current->m_data.offset = alignedOffset + size;
                current->m_data.size -= size + alignmentPadding;
                if (current->m_data.size == 0) {
                    freeBlocks.deleteNode(current);
                    --freeBlockAmount;
                }
                usedBlocks.insertFront(allocatedBlock);
                ++usedBlockAmount;
                freeSize -= size + alignmentPadding;

                return usedBlocks.getFront();
            }
        }
        return nullptr;
    }
};

