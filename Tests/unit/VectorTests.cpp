#include "../TestFramework.h"
#include "../../LinearStorage/Vector.h"
#include <string>

__TEST__(VectorDefaultConstruction) {
    Containers::Vector<int> vec;
    __ASSERT_EQ__(0, vec.size());
    __ASSERT__(vec.empty());
}

__TEST__(VectorPushBack) {
    Containers::Vector<int> vec;
    vec.pushBack(42);
    vec.pushBack(24);
    
    __ASSERT_EQ__(2, vec.size());
    __ASSERT_EQ__(42, vec[0]);
    __ASSERT_EQ__(24, vec[1]);
}

__TEST__(VectorCapacityGrowth) {
    Containers::Vector<int> vec;
    size_t initialCapacity = vec.capacity();
    
    // Fill beyond initial capacity
    for (int i = 0; i < 20; ++i) {
        vec.pushBack(i);
    }
    
    __ASSERT__(vec.capacity() > initialCapacity);
    __ASSERT_EQ__(20, vec.size());
}

__TEST__(VectorCopyConstructor) {
    Containers::Vector<int> vec1;
    vec1.pushBack(1);
    vec1.pushBack(2);
    vec1.pushBack(3);
    
    Containers::Vector<int> vec2(vec1);
    
    __ASSERT_EQ__(vec1.size(), vec2.size());
    for (size_t i = 0; i < vec1.size(); ++i) {
        __ASSERT_EQ__(vec1[i], vec2[i]);
    }
}

__TEST__(VectorMoveConstructor) {
    Containers::Vector<int> vec1;
    vec1.pushBack(1);
    vec1.pushBack(2);
    
    size_t originalSize = vec1.size();
    Containers::Vector<int> vec2(std::move(vec1));
    
    __ASSERT_EQ__(originalSize, vec2.size());
    __ASSERT_EQ__(1, vec2[0]);
    __ASSERT_EQ__(2, vec2[1]);
}

__TEST__(VectorIterators) {
    Containers::Vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.pushBack(i);
    }
    
    int expected = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        __ASSERT_EQ__(expected++, *it);
    }
}

__TEST__(VectorRangeBasedLoop) {
    Containers::Vector<int> vec;
    for (int i = 0; i < 3; ++i) {
        vec.pushBack(i * 2);
    }
    
    int expected = 0;
    for (const auto& value : vec) {
        __ASSERT_EQ__(expected, value);
        expected += 2;
    }
}

__TEST__(VectorAssignmentOperator) {
    Containers::Vector<int> vec1;
    vec1.pushBack(10);
    vec1.pushBack(20);
    
    Containers::Vector<int> vec2;
    vec2 = vec1;
    
    __ASSERT_EQ__(vec1.size(), vec2.size());
    __ASSERT_EQ__(10, vec2[0]);
    __ASSERT_EQ__(20, vec2[1]);
}

__TEST__(VectorMoveAssignment) {
    Containers::Vector<int> vec1;
    vec1.pushBack(100);
    vec1.pushBack(200);
    
    Containers::Vector<int> vec2;
    vec2 = std::move(vec1);
    
    __ASSERT_EQ__(2, vec2.size());
    __ASSERT_EQ__(100, vec2[0]);
    __ASSERT_EQ__(200, vec2[1]);
}

__TEST__(VectorPopBack) {
    Containers::Vector<int> vec;
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);
    
    vec.popBack();
    __ASSERT_EQ__(2, vec.size());
    __ASSERT_EQ__(2, vec[1]);
    
    vec.popBack();
    __ASSERT_EQ__(1, vec.size());
    __ASSERT_EQ__(1, vec[0]);
}

__TEST__(VectorClear) {
    Containers::Vector<int> vec;
    vec.pushBack(1);
    vec.pushBack(2);
    
    vec.clear();
    __ASSERT_EQ__(0, vec.size());
    __ASSERT__(vec.empty());
}

__TEST__(VectorResize) {
    Containers::Vector<int> vec;
    vec.pushBack(1);
    vec.pushBack(2);
    
    vec.resize(5);
    __ASSERT_EQ__(5, vec.size());
    __ASSERT_EQ__(1, vec[0]);  // Existing elements preserved
    __ASSERT_EQ__(2, vec[1]);
    
    vec.resize(3);
    __ASSERT_EQ__(3, vec.size());
    __ASSERT_EQ__(1, vec[0]);  // Elements still preserved
    __ASSERT_EQ__(2, vec[1]);
}

__TEST__(VectorAt) {
    Containers::Vector<int> vec;
    vec.pushBack(10);
    vec.pushBack(20);
    
    __ASSERT_EQ__(10, vec.at(0));
    __ASSERT_EQ__(20, vec.at(1));
}

__TEST__(VectorFrontBack) {
    Containers::Vector<int> vec;
    vec.pushBack(5);
    vec.pushBack(15);
    vec.pushBack(25);
    
    __ASSERT_EQ__(5, vec.front());
    __ASSERT_EQ__(25, vec.back());
}

__TEST__(VectorReserve) {
    Containers::Vector<int> vec;
    vec.reserve(100);
    
    __ASSERT__(vec.capacity() >= 100);
    __ASSERT_EQ__(0, vec.size());
}

__TEST__(VectorShrinkToFit) {
    Containers::Vector<int> vec;
    vec.reserve(100);
    vec.pushBack(1);
    vec.pushBack(2);
    
    vec.shrinkToFit();
    __ASSERT_EQ__(vec.capacity(), vec.size());
}

__TEST__(VectorEmplaceBack) {
    Containers::Vector<std::string> vec;
    vec.emplaceBack("hello");
    vec.emplaceBack("world");
    
    __ASSERT_EQ__(2, vec.size());
    __ASSERT_EQ__("hello", vec[0]);
    __ASSERT_EQ__("world", vec[1]);
}

__TEST__(VectorInsert) {
    Containers::Vector<int> vec;
    vec.pushBack(1);
    vec.pushBack(3);
    
    vec.insert(vec.begin() + 1, 2);
    
    __ASSERT_EQ__(3, vec.size());
    __ASSERT_EQ__(1, vec[0]);
    __ASSERT_EQ__(2, vec[1]);
    __ASSERT_EQ__(3, vec[2]);
}

__TEST__(VectorErase) {
    Containers::Vector<int> vec;
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);
    vec.pushBack(4);
    
    vec.erase(vec.begin() + 1);
    
    __ASSERT_EQ__(3, vec.size());
    __ASSERT_EQ__(1, vec[0]);
    __ASSERT_EQ__(3, vec[1]);
    __ASSERT_EQ__(4, vec[2]);
}

__TEST__(VectorSwap) {
    Containers::Vector<int> vec1;
    vec1.pushBack(1);
    vec1.pushBack(2);
    
    Containers::Vector<int> vec2;
    vec2.pushBack(10);
    vec2.pushBack(20);
    vec2.pushBack(30);
    
    vec1.swap(vec2);
    
    __ASSERT_EQ__(3, vec1.size());
    __ASSERT_EQ__(10, vec1[0]);
    __ASSERT_EQ__(2, vec2.size());
    __ASSERT_EQ__(1, vec2[0]);
}

__TEST__(VectorEquality) {
    Containers::Vector<int> vec1;
    vec1.pushBack(1);
    vec1.pushBack(2);
    
    Containers::Vector<int> vec2;
    vec2.pushBack(1);
    vec2.pushBack(2);
    
    Containers::Vector<int> vec3;
    vec3.pushBack(1);
    vec3.pushBack(3);
    
    __ASSERT__(vec1 == vec2);
    __ASSERT__(!(vec1 == vec3));
}