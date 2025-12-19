#include "../TestFramework.h"
#include "../../LinearStorage/Vector.h"
#include "../../LinearStorage/String.h"
#include "../../Utilities/Concepts.h"

using namespace Containers;

// Tracking allocator to monitor allocation behavior
template<typename T>
class TrackingAllocator {
public:
    static inline size_t allocations = 0;
    static inline size_t deallocations = 0;
    static inline size_t constructions = 0;
    static inline size_t destructions = 0;

    static void reset() {
        allocations = deallocations = constructions = destructions = 0;
    }

    T* allocate(size_t count) {
        allocations++;
        return static_cast<T*>(::operator new(count * sizeof(T)));
    }

    void deallocate(T* ptr, size_t count) {
        deallocations++;
        ::operator delete(ptr);
    }

    template<typename... Args>
    void construct(T* ptr, Args&&... args) {
        constructions++;
        ::new(ptr) T(std::forward<Args>(args)...);
    }

    void destroy(T* ptr) {
        destructions++;
        ptr->~T();
    }
};

__TEST__(vector_custom_allocator_basic) {
    TrackingAllocator<int>::reset();
    {
        Vector<int, TrackingAllocator<int>> v;
        v.pushBack(42);
        v.pushBack(24);
        __ASSERT_EQ__(2, v.size());
        __ASSERT_EQ__(42, v[0]);
        __ASSERT_EQ__(24, v[1]);
    }
    __ASSERT__(TrackingAllocator<int>::allocations > 0);
    __ASSERT__(TrackingAllocator<int>::constructions >= 2);
    __ASSERT_EQ__(TrackingAllocator<int>::constructions, TrackingAllocator<int>::destructions);
}

__TEST__(vector_trivial_allocator) {
    Vector<int, TrivialAllocator<int>> v;
    v.pushBack(1);
    v.pushBack(2);
    v.pushBack(3);
    __ASSERT_EQ__(3, v.size());
    __ASSERT_EQ__(1, v[0]);
    __ASSERT_EQ__(2, v[1]);
    __ASSERT_EQ__(3, v[2]);
}

__TEST__(vector_class_allocator) {
    Vector<std::string, ClassAllocator<std::string>> v;
    v.pushBack("hello");
    v.pushBack("world");
    __ASSERT_EQ__(2, v.size());
    __ASSERT_EQ__("hello", v[0]);
    __ASSERT_EQ__("world", v[1]);
}

__TEST__(vector_allocator_growth) {
    TrackingAllocator<int>::reset();
    {
        Vector<int, TrackingAllocator<int>, 2> v; // Small initial capacity
        for (int i = 0; i < 10; ++i) {
            v.pushBack(i);
        }
        __ASSERT_EQ__(10, v.size());
    }
    // Should have multiple allocations due to growth
    __ASSERT__(TrackingAllocator<int>::allocations > 1);
    __ASSERT_EQ__(TrackingAllocator<int>::constructions, TrackingAllocator<int>::destructions);
}

__TEST__(string_custom_allocator_basic) {
    TrackingAllocator<char>::reset();
    {
        StringBase<char, TrackingAllocator<char>> s("hello");
        __ASSERT_EQ__(5, s.size());
        __ASSERT_EQ__("hello", s);
    }
    __ASSERT__(TrackingAllocator<char>::allocations > 0);
    __ASSERT__(TrackingAllocator<char>::constructions >= 5);
    __ASSERT_EQ__(TrackingAllocator<char>::constructions, TrackingAllocator<char>::destructions);
}

__TEST__(string_trivial_allocator) {
    StringBase<char, TrivialAllocator<char>> s("test");
    s += " string";
    __ASSERT_EQ__(11, s.size());
    __ASSERT_EQ__("test string", s);
}

__TEST__(string_class_allocator) {
    StringBase<char, ClassAllocator<char>> s;
    s += "custom";
    s += " allocator";
    __ASSERT_EQ__(16, s.size());
    __ASSERT_EQ__("custom allocator", s);
}

__TEST__(allocator_concept_compliance) {
    static_assert(AllocatorConcept<TrackingAllocator<int>, int>);
    static_assert(AllocatorConcept<TrivialAllocator<int>, int>);
    static_assert(AllocatorConcept<ClassAllocator<std::string>, std::string>);
    __ASSERT__(true); // Compilation success means concept works
}