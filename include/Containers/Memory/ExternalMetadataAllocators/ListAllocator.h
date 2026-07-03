#pragma once
#include <cstdint>
#include <string>
#include <stdexcept>

#include "Containers/Utilities/Concepts.h"
#include "Containers/Lists/BidirectionalList.h"
#include "Containers/Maps/UnorderedMap.h"

// allocators that can accept any type castable to uintptr_t as a pool pointer
namespace Memory::ExternalMetadataAllocators {

	class ListAllocatorNaiveBase {
	private:
		struct MemoryRegion {
			enum class Type : uint8_t {
				Free,
				Allocated
			};
			Type type;
			uintptr_t start;
			size_t size;
		};

		uintptr_t m_data;
		size_t m_size;
		size_t m_freeSize;

		Containers::BidirectionalList<MemoryRegion> m_regions;
		
	public:
		ListAllocatorNaiveBase() = default;
		~ListAllocatorNaiveBase() = default;

		ListAllocatorNaiveBase(const ListAllocatorNaiveBase&) = delete;
		ListAllocatorNaiveBase& operator=(const ListAllocatorNaiveBase&) = delete;

		ListAllocatorNaiveBase(ListAllocatorNaiveBase&&) = default;
		ListAllocatorNaiveBase& operator=(ListAllocatorNaiveBase&&) = default;

		void assign(uintptr_t data, size_t size) {
			if (size <= sizeof(MemoryRegion))
				throw std::runtime_error("Size must be more than sizeof(MemoryRegion) bytes");
			m_data = data;
			m_size = size;
			m_freeSize = m_size;
			m_regions.pushBack(MemoryRegion{MemoryRegion::Type::Free, m_data, m_size});
		}

        uintptr_t allocate(size_t size) {
            return allocateImpl(size);
        }

        void deallocate(uintptr_t ptr) {
            deallocateImpl(ptr);
        }

	private:

		uintptr_t allocateImpl(size_t size) {
			auto it = m_regions.begin();
			while(true) {				
				if(it->type == MemoryRegion::Type::Free && it->size >= size) break;
				++it;
				if(it == m_regions.end()) return std::numeric_limits<uintptr_t>::max();
			}
			
			if(it->size == size) {
				it->type = MemoryRegion::Type::Allocated;
				return it->start;
			}

			auto allocated = m_regions.insert(it);

			allocated->size = size;
			allocated->start = it->start;
			allocated->type = MemoryRegion::Type::Allocated;

			it->size -= size;
			it->start += size;

			return allocated->start;
		}

		void deallocateImpl(uintptr_t ptr) {
			auto it = m_regions.begin();
			while(true) {				
				if(it->start == ptr) break;
				++it;
				if(it == m_regions.end()) return;
			}
			auto prev = it.prev();
			auto next = it.next();

			if(prev != m_regions.end() && prev->type == MemoryRegion::Type::Free) {
				it->start = prev->start;
				it->size += prev->size;
				m_regions.erase(prev);
			}
			if(next != m_regions.end() && next->type == MemoryRegion::Type::Free) {
				it->size += next->size;
				m_regions.erase(next);
			}
			it->type = MemoryRegion::Type::Free;
		}
	};

	class ListAllocatorBase {
	private:
		struct MemoryRegion {
			enum class Type : uint8_t {
				Free,
				Allocated
			};
			Type type;
			uintptr_t start;
			size_t size;

			using Iterator = Containers::BidirectionalList<typename Containers::BidirectionalList<MemoryRegion>::Iterator>::Iterator;

			Iterator storageInterator;
		};

		uintptr_t m_data;
		size_t m_size;
		size_t m_freeSize;

		Containers::BidirectionalList<MemoryRegion> m_regions;
		Containers::BidirectionalList<typename Containers::BidirectionalList<MemoryRegion>::Iterator> m_freeRegions;
		Containers::BidirectionalList<typename Containers::BidirectionalList<MemoryRegion>::Iterator> m_allocatedRegions;
		Containers::UnorderedMap<uintptr_t, typename Containers::BidirectionalList<MemoryRegion>::Iterator> m_pointerToRegionMap;
		
	public:
		ListAllocatorBase() = default;
		~ListAllocatorBase() = default;

		ListAllocatorBase(const ListAllocatorBase&) = delete;
		ListAllocatorBase& operator=(const ListAllocatorBase&) = delete;

		ListAllocatorBase(ListAllocatorBase&&) = default;
		ListAllocatorBase& operator=(ListAllocatorBase&&) = default;

		void assign(uintptr_t data, size_t size) {
			if (size <= sizeof(MemoryRegion))
				throw std::runtime_error("Size must be more than sizeof(MemoryRegion) bytes");
			m_data = data;
			m_size = size;
			m_freeSize = m_size;
			m_regions.pushBack(MemoryRegion::Type::Free, m_data, m_size, m_freeRegions.end());
			m_freeRegions.pushBack(m_regions.begin());
			m_regions.back().storageInterator = m_freeRegions.begin();
		}

        uintptr_t allocate(size_t size) {
            return allocateImpl(size);
        }

        void deallocate(uintptr_t ptr) {
            deallocateImpl(ptr);
        }

	private:

		uintptr_t allocateImpl(size_t size) {
			for(auto it = m_freeRegions.begin(); it != m_freeRegions.end(); ++it) {
				if((*it)->size > size) {
					auto current = *it;
					size_t offset = current->size - size;
					auto allocation = m_regions.insert(current);
					allocation->type = MemoryRegion::Type::Allocated;
					allocation->size = size;
					allocation->start = current->start;
					allocation->storageInterator = m_allocatedRegions.insert(m_allocatedRegions.end(), allocation);
					m_pointerToRegionMap.insert(allocation->start, allocation);

					current->size = offset;
					current->start += offset;
					m_freeSize -= size;
					return allocation->start;
				}
				else if((*it)->size == size) {
					auto current = *it;
					m_freeRegions.erase(it);
					current->storageInterator = m_allocatedRegions.insert(m_allocatedRegions.end(), current);
					current->type = MemoryRegion::Type::Allocated;
					m_pointerToRegionMap.insert(current->start, current);
					m_freeSize -= size;
					return current->start;
				}
			}
			return std::numeric_limits<uintptr_t>::max();
		}

		void deallocateImpl(uintptr_t ptr) {
			auto mapIt = m_pointerToRegionMap.find(ptr);
			if(mapIt == m_pointerToRegionMap.end()) throw std::runtime_error("Trying to deallocate invalid pointer");
			auto allocation = mapIt->second;
			m_pointerToRegionMap.erase(mapIt);
			
			auto next = allocation.next();
			auto prev = allocation.prev();
			m_allocatedRegions.erase(allocation->storageInterator);
			m_freeSize += allocation->size;
			
			if(next != m_regions.end() && next->type == MemoryRegion::Type::Free) {
				if(prev != m_regions.end() && prev->type == MemoryRegion::Type::Free) {
					prev->size += allocation->size + next->size;
					m_freeRegions.erase(next->storageInterator);
					m_regions.erase(next);					
					m_regions.erase(allocation);
				}
				else {
					next->start -= allocation->size;
					next->size += allocation->size;
					m_regions.erase(allocation);
				}
			}
			else if(prev != m_regions.end() && prev->type == MemoryRegion::Type::Free) {
				prev->size += allocation->size;
				m_regions.erase(allocation);
			}
			else {
				allocation->storageInterator = m_freeRegions.insert(m_freeRegions.end(), allocation);
				allocation->type = MemoryRegion::Type::Free;
			}			
		}
	};

}

