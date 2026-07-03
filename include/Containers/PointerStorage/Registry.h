// #pragma once
// #include <memory>
// #include <stdexcept>

// #include "PointerStorage/InitializerList.h"
// #include "PointerStorage/PointerContainers.h"
// #include "PointerStorage/ContainerInterfaces.h"
// #include "PointerStorage/Span.h"

// #include "PointerStorage/Vector.h"
// #include "Sets/UnorderedSet.h"
// #include "utilities/Optional.h"

// namespace Containers
// {
//     class Registry;

//     class RegistryId {
//         friend class Registry;
//     private:
//         uint32_t m_index;
//         uint32_t m_generation;
//         explicit Id(index, generation) : m_index(index), m_generation(generation) {}
//         RegistryId& operator++() { ++m_generation; return *this; }
//     public:
//         operator uint32_t() const { return m_index; }

//         uint32_t id() const { return m_index; }
//         uint32_t generation() const { return m_generation; }
//     };

//     template <typename T, TypedAllocatorType<T> Alloc = TypedAllocator<T>, TypedAllocatorType<RegistryId> AllocStack = TypedAllocator<RegistryId>,
//         LifetimeManagerType<T> LifeT = LifetimeManager<T>, LifetimeManagerType<Optional<T, LifeT>> Life = LifetimeManager<Optional<T, LifeT>>, 
//         LifetimeManagerType<RegistryId> LifeStack = LifetimeManager<RegistryId>,
//         size_t s_initialCapacity = 16, float s_growthFactor = 1.618f, size_t s_initialCapacityId = 16, float s_growthFactorId = 1.618f>
//     class Registry {
//     private:
//         struct Slot {
            
//         }

//         using Container = Containers::Vector<Optional<T, Life>, Alloc, Life, s_initialCapacity, s_growthFactor>;
//         using FreeIds = Containers::Stack<T, AllocStack, LifeStack, s_initialCapacityId, s_growthFactorId>;



//         Container m_data;
//         FreeIds m_freeIds;

//     public:

//         Registry() = default;

//         template<typename... Args>
//         Id emplace(Args...&& args) {
//             if(m_freeIds.empty()) {
//                 m_data.pushBack(std::forward<Args>(args)...);
//                 return Id(m_data.size() - 1, 0);
//             }
//             auto id = m_freeIds.extract();
//             m_data[id].emplace(std::forward<Args>(args)...);
//             return ++id;
//         }

//         Registry& remove(Id id) {
            
//             if(it != m_freeIds.end()) throw std::runtime_error("Trying to remove a nonexistent object");
//             if(id >= m_data.size()) throw std::runtime_error("Trying to remove with an invalid Id");
//             m_freeIds.insert(id);
//             return *this;
//         }

        

//     }


// }