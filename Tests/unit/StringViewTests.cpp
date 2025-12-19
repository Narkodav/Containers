#include "../TestFramework.h"
#include "../../LinearStorage/StringView.h"
#include "../../LinearStorage/String.h"

__TEST__(StringViewDefaultConstruction) {
    const char* cstr = "test";
    Containers::StringView sv(cstr, 4);
    
    __ASSERT_EQ__(4, sv.size());
    __ASSERT_EQ__('t', sv[0]);
    __ASSERT_EQ__('t', sv[3]);
}

__TEST__(StringViewCStringConstruction) {
    const char* cstr = "hello";
    Containers::StringView sv(cstr);
    
    __ASSERT_EQ__(5, sv.size());
    __ASSERT_EQ__("hello", sv);
}

__TEST__(StringViewFromString) {
    Containers::String str("fromstring");
    Containers::StringView sv(str);
    
    __ASSERT_EQ__(10, sv.size());
    __ASSERT_EQ__("fromstring", sv);
}

__TEST__(StringViewElementAccess) {
    const char* cstr = "access";
    Containers::StringView sv(cstr);
    
    __ASSERT_EQ__('a', sv[0]);
    __ASSERT_EQ__('c', sv[1]);
    __ASSERT_EQ__('s', sv[5]);
}

__TEST__(StringViewAtMethod) {
    const char* cstr = "bounds";
    Containers::StringView sv(cstr);
    
    __ASSERT_EQ__('b', sv.at(0));
    __ASSERT_EQ__('s', sv.at(5));
}

__TEST__(StringViewFrontBack) {
    const char* cstr = "frontback";
    Containers::StringView sv(cstr);
    
    __ASSERT_EQ__('f', sv.front());
    __ASSERT_EQ__('k', sv.back());
}

__TEST__(StringViewDataAccess) {
    const char* original = "convert";
    Containers::StringView sv(original);
    
    __ASSERT_EQ__(original, sv.data());
    
    // Test explicit data access
    const char* dataPtr = sv.data();
    __ASSERT_EQ__(original, dataPtr);
}

__TEST__(StringViewIterators) {
    const char* cstr = "iterate";
    Containers::StringView sv(cstr);
    
    char expected = 'i';
    for (auto it = sv.begin(); it != sv.end(); ++it) {
        __ASSERT_EQ__(expected, *it);
        if (expected == 'i') expected = 't';
        else if (expected == 't') expected = 'e';
        else if (expected == 'e') expected = 'r';
        else if (expected == 'r') expected = 'a';
        else if (expected == 'a') expected = 't';
        else expected = 'e';
    }
}

__TEST__(StringViewConstIterators) {
    const char* cstr = "const";
    Containers::StringView sv(cstr);
    
    size_t count = 0;
    for (auto it = sv.cbegin(); it != sv.cend(); ++it) {
        count++;
    }
    
    __ASSERT_EQ__(5, count);
}

__TEST__(StringViewRangeBasedLoop) {
    const char* cstr = "range";
    Containers::StringView sv(cstr);
    
    size_t count = 0;
    for (const auto& ch : sv) {
        count++;
    }
    
    __ASSERT_EQ__(5, count);
}

__TEST__(StringViewFromModifiedString) {
    Containers::String str("original");
    Containers::StringView sv(str);
    
    __ASSERT_EQ__("original", sv);
    
    // Modify the string
    str[0] = 'O';
    
    // StringView should reflect the change
    __ASSERT_EQ__('O', sv[0]);
}

__TEST__(StringViewEmptyString) {
    const char* empty = "";
    Containers::StringView sv(empty);
    
    __ASSERT_EQ__(0, sv.size());
}

__TEST__(StringViewSingleChar) {
    const char* single = "x";
    Containers::StringView sv(single);
    
    __ASSERT_EQ__(1, sv.size());
    __ASSERT_EQ__('x', sv[0]);
    __ASSERT_EQ__('x', sv.front());
    __ASSERT_EQ__('x', sv.back());
}

__TEST__(StringViewLongString) {
    const char* longStr = "this is a very long string for testing purposes";
    Containers::StringView sv(longStr);
    
    __ASSERT_EQ__(47, sv.size());
    __ASSERT_EQ__('t', sv.front());
    __ASSERT_EQ__('s', sv.back());
}

__TEST__(StringViewCopyConstructor) {
    const char* cstr = "copy";
    Containers::StringView sv1(cstr);
    Containers::StringView sv2(sv1);
    
    __ASSERT_EQ__(sv1.size(), sv2.size());
    __ASSERT_EQ__(sv1.cStr(), sv2.cStr());
}

__TEST__(StringViewCopyAssignment) {
    const char* cstr1 = "first";
    const char* cstr2 = "second";
    
    Containers::StringView sv1(cstr1);
    Containers::StringView sv2(cstr2);
    
    sv1 = sv2;
    
    __ASSERT_EQ__(sv2.size(), sv1.size());
    __ASSERT_EQ__(sv2.cStr(), sv1.cStr());
}

__TEST__(StringViewMoveConstructor) {
    const char* cstr = "move";
    Containers::StringView sv1(cstr);
    Containers::StringView sv2(std::move(sv1));
    
    __ASSERT_EQ__(4, sv2.size());
    __ASSERT_EQ__("move", sv2.cStr());
}

__TEST__(StringViewMoveAssignment) {
    const char* cstr1 = "move1";
    const char* cstr2 = "move2";
    
    Containers::StringView sv1(cstr1);
    Containers::StringView sv2(cstr2);
    
    sv1 = std::move(sv2);
    
    __ASSERT_EQ__(5, sv1.size());
    __ASSERT_EQ__("move2", sv1.cStr());
}

__TEST__(StringViewWithNullTerminator) {
    const char cstr[] = {'h', 'e', 'l', 'l', 'o', '\0', 'x', 'x'};
    Containers::StringView sv(cstr, 5); // Explicitly specify size
    
    __ASSERT_EQ__(5, sv.size());
    __ASSERT_EQ__('h', sv[0]);
    __ASSERT_EQ__('o', sv[4]);
}

__TEST__(StringViewIteratorDistance) {
    const char* cstr = "distance";
    Containers::StringView sv(cstr);
    
    auto distance = sv.end() - sv.begin();
    __ASSERT_EQ__(8, distance);
    __ASSERT_EQ__(sv.size(), static_cast<size_t>(distance));
}