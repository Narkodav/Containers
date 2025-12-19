#include "../TestFramework.h"
#include "../../LinearStorage/Array.h"
#include <string>

__TEST__(ArrayDefaultConstruction) {
    Containers::Array<int, 5> arr;
    __ASSERT_EQ__(5, arr.size());
    __ASSERT__(!arr.empty());
}

__TEST__(ArrayZeroSizeArray) {
    Containers::Array<int, 0> arr;
    __ASSERT_EQ__(0, arr.size());
    __ASSERT__(arr.empty());
}

__TEST__(ArrayInitializerListConstruction) {
    Containers::Array<int, 4> arr{1, 2, 3, 4};
    __ASSERT_EQ__(4, arr.size());
    __ASSERT_EQ__(1, arr[0]);
    __ASSERT_EQ__(2, arr[1]);
    __ASSERT_EQ__(3, arr[2]);
    __ASSERT_EQ__(4, arr[3]);
}

__TEST__(ArrayPartialInitializerList) {
    Containers::Array<int, 5> arr{10, 20};
    __ASSERT_EQ__(5, arr.size());
    __ASSERT_EQ__(10, arr[0]);
    __ASSERT_EQ__(20, arr[1]);
}

__TEST__(ArrayCArrayConstruction) {
    int data[3] = {100, 200, 300};
    Containers::Array<int, 3> arr(data);
    __ASSERT_EQ__(3, arr.size());
    __ASSERT_EQ__(100, arr[0]);
    __ASSERT_EQ__(200, arr[1]);
    __ASSERT_EQ__(300, arr[2]);
}

__TEST__(ArrayCopyConstructor) {
    Containers::Array<int, 3> arr1{1, 2, 3};
    Containers::Array<int, 3> arr2(arr1);
    
    __ASSERT_EQ__(arr1.size(), arr2.size());
    for (size_t i = 0; i < arr1.size(); ++i) {
        __ASSERT_EQ__(arr1[i], arr2[i]);
    }
}

__TEST__(ArrayMoveConstructor) {
    Containers::Array<std::string, 2> arr1{"hello", "world"};
    Containers::Array<std::string, 2> arr2(std::move(arr1));
    
    __ASSERT_EQ__(2, arr2.size());
    __ASSERT_EQ__("hello", arr2[0]);
    __ASSERT_EQ__("world", arr2[1]);
}

__TEST__(ArrayCopyAssignment) {
    Containers::Array<int, 3> arr1{10, 20, 30};
    Containers::Array<int, 3> arr2;
    
    arr2 = arr1;
    
    __ASSERT_EQ__(arr1.size(), arr2.size());
    for (size_t i = 0; i < arr1.size(); ++i) {
        __ASSERT_EQ__(arr1[i], arr2[i]);
    }
}

__TEST__(ArrayMoveAssignment) {
    Containers::Array<std::string, 2> arr1{"test", "data"};
    Containers::Array<std::string, 2> arr2;
    
    arr2 = std::move(arr1);
    
    __ASSERT_EQ__(2, arr2.size());
    __ASSERT_EQ__("test", arr2[0]);
    __ASSERT_EQ__("data", arr2[1]);
}

__TEST__(ArrayElementAccess) {
    Containers::Array<int, 4> arr{5, 10, 15, 20};
    
    __ASSERT_EQ__(5, arr[0]);
    __ASSERT_EQ__(10, arr[1]);
    __ASSERT_EQ__(15, arr[2]);
    __ASSERT_EQ__(20, arr[3]);
    
    arr[1] = 99;
    __ASSERT_EQ__(99, arr[1]);
}

__TEST__(ArrayAtMethod) {
    Containers::Array<int, 3> arr{1, 2, 3};
    
    __ASSERT_EQ__(1, arr.at(0));
    __ASSERT_EQ__(2, arr.at(1));
    __ASSERT_EQ__(3, arr.at(2));
}

__TEST__(ArrayFrontBack) {
    Containers::Array<int, 4> arr{100, 200, 300, 400};
    
    __ASSERT_EQ__(100, arr.front());
    __ASSERT_EQ__(400, arr.back());
    
    arr.front() = 999;
    arr.back() = 888;
    
    __ASSERT_EQ__(999, arr[0]);
    __ASSERT_EQ__(888, arr[3]);
}

__TEST__(ArrayDataPointer) {
    Containers::Array<int, 3> arr{7, 8, 9};
    
    int* ptr = arr.data();
    __ASSERT_EQ__(7, ptr[0]);
    __ASSERT_EQ__(8, ptr[1]);
    __ASSERT_EQ__(9, ptr[2]);
    
    ptr[1] = 77;
    __ASSERT_EQ__(77, arr[1]);
}

__TEST__(ArrayIterators) {
    Containers::Array<int, 4> arr{1, 2, 3, 4};
    
    int expected = 1;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        __ASSERT_EQ__(expected++, *it);
    }
}

__TEST__(ArrayConstIterators) {
    const Containers::Array<int, 3> arr{10, 20, 30};
    
    int expected = 10;
    for (auto it = arr.cbegin(); it != arr.cend(); ++it) {
        __ASSERT_EQ__(expected, *it);
        expected += 10;
    }
}

__TEST__(ArrayRangeBasedLoop) {
    Containers::Array<int, 3> arr{5, 15, 25};
    
    int expected = 5;
    for (const auto& value : arr) {
        __ASSERT_EQ__(expected, value);
        expected += 10;
    }
}

__TEST__(ArrayModifyThroughIterator) {
    Containers::Array<int, 3> arr{1, 2, 3};
    
    for (auto& value : arr) {
        value *= 2;
    }
    
    __ASSERT_EQ__(2, arr[0]);
    __ASSERT_EQ__(4, arr[1]);
    __ASSERT_EQ__(6, arr[2]);
}

__TEST__(ArrayStringType) {
    Containers::Array<std::string, 2> arr{"first", "second"};
    
    __ASSERT_EQ__(2, arr.size());
    __ASSERT_EQ__("first", arr[0]);
    __ASSERT_EQ__("second", arr[1]);
    
    arr[0] = "modified";
    __ASSERT_EQ__("modified", arr[0]);
}