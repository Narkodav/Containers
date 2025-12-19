#include "../TestFramework.h"
#include "../../LinearStorage/Span.h"
#include "../../LinearStorage/Vector.h"
#include <vector>

__TEST__(SpanDefaultConstruction) {
    Containers::Span<int, 5> span;
    __ASSERT_EQ__(5, span.size());
    __ASSERT__(span.empty()); // empty() checks if data is nullptr
}

__TEST__(SpanDynamicDefaultConstruction) {
    Containers::Span<int> span;
    __ASSERT_EQ__(0, span.size());
    __ASSERT__(span.empty());
}

__TEST__(SpanFromRawPointer) {
    int data[4] = {10, 20, 30, 40};
    Containers::Span<int, 4> span(data);
    
    __ASSERT_EQ__(4, span.size());
    __ASSERT_EQ__(10, span[0]);
    __ASSERT_EQ__(20, span[1]);
    __ASSERT_EQ__(30, span[2]);
    __ASSERT_EQ__(40, span[3]);
}

__TEST__(SpanDynamicFromRawPointer) {
    int data[3] = {100, 200, 300};
    Containers::Span<int> span(data, 3);
    
    __ASSERT_EQ__(3, span.size());
    __ASSERT_EQ__(100, span[0]);
    __ASSERT_EQ__(200, span[1]);
    __ASSERT_EQ__(300, span[2]);
}

__TEST__(SpanFromArray) {
    Containers::Array<int, 3> arr{1, 2, 3};
    Containers::Span<int, 3> span(arr);
    
    __ASSERT_EQ__(3, span.size());
    __ASSERT_EQ__(1, span[0]);
    __ASSERT_EQ__(2, span[1]);
    __ASSERT_EQ__(3, span[2]);
    
    // Modify through span
    span[1] = 99;
    __ASSERT_EQ__(99, arr[1]);
}

__TEST__(SpanFromVector) {
    Containers::Vector<int> vec;
    vec.pushBack(5);
    vec.pushBack(10);
    vec.pushBack(15);
    
    Containers::Span<int> span(vec);
    
    __ASSERT_EQ__(3, span.size());
    __ASSERT_EQ__(5, span[0]);
    __ASSERT_EQ__(10, span[1]);
    __ASSERT_EQ__(15, span[2]);
    
    // Modify through span
    span[0] = 555;
    __ASSERT_EQ__(555, vec[0]);
}

__TEST__(SpanFromVectorWithSize) {
    Containers::Vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.pushBack(i);
    }
    
    Containers::Span<int> span(vec, 5); // Only first 5 elements
    
    __ASSERT_EQ__(5, span.size());
    for (int i = 0; i < 5; ++i) {
        __ASSERT_EQ__(i, span[i]);
    }
}

__TEST__(SpanFromIterators) {
    Containers::Vector<int> vec;
    vec.pushBack(11);
    vec.pushBack(22);
    vec.pushBack(33);
    
    Containers::Span<int> span(vec.begin(), vec.end());
    
    __ASSERT_EQ__(3, span.size());
    __ASSERT_EQ__(11, span[0]);
    __ASSERT_EQ__(22, span[1]);
    __ASSERT_EQ__(33, span[2]);
}

__TEST__(SpanCopyConstructor) {
    int data[3] = {1, 2, 3};
    Containers::Span<int, 3> span1(data);
    Containers::Span<int, 3> span2(span1);
    
    __ASSERT_EQ__(span1.size(), span2.size());
    __ASSERT_EQ__(span1.data(), span2.data()); // Same data pointer
    
    for (size_t i = 0; i < span1.size(); ++i) {
        __ASSERT_EQ__(span1[i], span2[i]);
    }
}

__TEST__(SpanAssignment) {
    int data1[2] = {10, 20};
    int data2[2] = {30, 40};
    
    Containers::Span<int, 2> span1(data1);
    Containers::Span<int, 2> span2(data2);
    
    span1 = span2;
    
    __ASSERT_EQ__(span2.data(), span1.data());
    __ASSERT_EQ__(30, span1[0]);
    __ASSERT_EQ__(40, span1[1]);
}

__TEST__(SpanElementAccess) {
    int data[4] = {100, 200, 300, 400};
    Containers::Span<int, 4> span(data);
    
    __ASSERT_EQ__(100, span[0]);
    __ASSERT_EQ__(200, span[1]);
    __ASSERT_EQ__(300, span[2]);
    __ASSERT_EQ__(400, span[3]);
    
    span[2] = 999;
    __ASSERT_EQ__(999, data[2]); // Original data modified
}

__TEST__(SpanAtMethod) {
    int data[3] = {5, 10, 15};
    Containers::Span<int, 3> span(data);
    
    __ASSERT_EQ__(5, span.at(0));
    __ASSERT_EQ__(10, span.at(1));
    __ASSERT_EQ__(15, span.at(2));
}

__TEST__(SpanFrontBack) {
    int data[4] = {1, 2, 3, 4};
    Containers::Span<int, 4> span(data);
    
    __ASSERT_EQ__(1, span.front());
    __ASSERT_EQ__(4, span.back());
    
    span.front() = 99;
    span.back() = 88;
    
    __ASSERT_EQ__(99, data[0]);
    __ASSERT_EQ__(88, data[3]);
}

__TEST__(SpanDataPointer) {
    int data[3] = {7, 8, 9};
    Containers::Span<int, 3> span(data);
    
    __ASSERT_EQ__(data, span.data());
    
    int* ptr = span.data();
    ptr[1] = 777;
    __ASSERT_EQ__(777, span[1]);
}

__TEST__(SpanIterators) {
    int data[4] = {2, 4, 6, 8};
    Containers::Span<int, 4> span(data);
    
    int expected = 2;
    for (auto it = span.begin(); it != span.end(); ++it) {
        __ASSERT_EQ__(expected, *it);
        expected += 2;
    }
}

__TEST__(SpanConstIterators) {
    int data[3] = {10, 20, 30};
    const Containers::Span<int, 3> span(data);
    
    int expected = 10;
    for (auto it = span.cbegin(); it != span.cend(); ++it) {
        __ASSERT_EQ__(expected, *it);
        expected += 10;
    }
}

__TEST__(SpanRangeBasedLoop) {
    int data[3] = {3, 6, 9};
    Containers::Span<int, 3> span(data);
    
    int expected = 3;
    for (const auto& value : span) {
        __ASSERT_EQ__(expected, value);
        expected += 3;
    }
}

__TEST__(SpanModifyThroughIterator) {
    int data[3] = {1, 2, 3};
    Containers::Span<int, 3> span(data);
    
    for (auto& value : span) {
        value *= 10;
    }
    
    __ASSERT_EQ__(10, data[0]);
    __ASSERT_EQ__(20, data[1]);
    __ASSERT_EQ__(30, data[2]);
}

__TEST__(SpanClear) {
    int data[2] = {1, 2};
    Containers::Span<int, 2> span(data);
    
    __ASSERT__(!span.empty());
    
    span.clear();
    __ASSERT__(span.empty());
}

__TEST__(SpanAssignNewData) {
    int data1[2] = {1, 2};
    int data2[2] = {10, 20};
    
    Containers::Span<int, 2> span(data1);
    __ASSERT_EQ__(1, span[0]);
    
    span.assign(data2);
    __ASSERT_EQ__(10, span[0]);
    __ASSERT_EQ__(20, span[1]);
}

__TEST__(SpanDynamicAssignWithSize) {
    int data1[3] = {1, 2, 3};
    int data2[3] = {10, 20, 30};
    
    Containers::Span<int> span(data1, 3);
    __ASSERT_EQ__(3, span.size());
    __ASSERT_EQ__(1, span[0]);
    
    span.assign(data2, 2); // Only assign 2 elements
    __ASSERT_EQ__(2, span.size());
    __ASSERT_EQ__(10, span[0]);
    __ASSERT_EQ__(20, span[1]);
}

__TEST__(SpanDynamicClear) {
    int data[2] = {1, 2};
    Containers::Span<int> span(data, 2);

    __ASSERT_EQ__(2, span.size());
    __ASSERT__(!span.empty());
    
    span.clear();
    __ASSERT_EQ__(0, span.size());
    __ASSERT__(span.empty());
}