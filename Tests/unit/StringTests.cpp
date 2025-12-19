#include "../TestFramework.h"
#include "../../LinearStorage/String.h"
#include <string>

__TEST__(StringDefaultConstruction) {
    Containers::String str;
    __ASSERT_EQ__(0, str.size());
    __ASSERT__(str.empty());
    __ASSERT_EQ__('\0', str[0]);
}

__TEST__(StringLiteralConstruction) {
    Containers::String str("hello");
    __ASSERT_EQ__(5, str.size());
    __ASSERT__(!str.empty());
    __ASSERT_EQ__('h', str[0]);
    __ASSERT_EQ__('o', str[4]);
}

__TEST__(StringArrayConstruction) {
    const char arr[] = "world";
    Containers::String str(arr);
    __ASSERT_EQ__(5, str.size());
    __ASSERT_EQ__("world", str);
}

__TEST__(StringStdStringConstruction) {
    std::string stdStr = "test";
    Containers::String str(stdStr);
    __ASSERT_EQ__(4, str.size());
    __ASSERT_EQ__("test", str);
}

__TEST__(StringSizeValueConstruction) {
    Containers::String str(3, 'a');
    __ASSERT_EQ__(3, str.size());
    __ASSERT_EQ__('a', str[0]);
    __ASSERT_EQ__('a', str[1]);
    __ASSERT_EQ__('a', str[2]);
}

__TEST__(StringCopyConstructor) {
    Containers::String str1("original");
    Containers::String str2(str1);
    
    __ASSERT_EQ__(str1.size(), str2.size());
    __ASSERT_EQ__("original", str2);
}

__TEST__(StringMoveConstructor) {
    Containers::String str1("moveme");
    Containers::String str2(std::move(str1));
    
    __ASSERT_EQ__(6, str2.size());
    __ASSERT_EQ__("moveme", str2);
}

__TEST__(StringCopyAssignment) {
    Containers::String str1("source");
    Containers::String str2;
    
    str2 = str1;
    
    __ASSERT_EQ__(str1.size(), str2.size());
    __ASSERT_EQ__("source", str2);
}

__TEST__(StringMoveAssignment) {
    Containers::String str1("moveassign");
    Containers::String str2;
    
    str2 = std::move(str1);
    
    __ASSERT_EQ__(10, str2.size());
    __ASSERT_EQ__("moveassign", str2);
}

__TEST__(StringLiteralAssignment) {
    Containers::String str;
    str = "assigned";
    
    __ASSERT_EQ__(8, str.size());
    __ASSERT_EQ__("assigned", str);
}

__TEST__(StringStdStringAssignment) {
    Containers::String str;
    std::string stdStr = "stdassign";
    str = stdStr;
    
    __ASSERT_EQ__(9, str.size());
    __ASSERT_EQ__("stdassign", str);
}

__TEST__(StringElementAccess) {
    Containers::String str("access");
    
    __ASSERT_EQ__('a', str[0]);
    __ASSERT_EQ__('c', str[1]);
    __ASSERT_EQ__('s', str[5]);
    
    str[1] = 'X';
    __ASSERT_EQ__('X', str[1]);
}

__TEST__(StringAtMethod) {
    Containers::String str("bounds");
    
    __ASSERT_EQ__('b', str.at(0));
    __ASSERT_EQ__('s', str.at(5));
}

__TEST__(StringFrontBack) {
    Containers::String str("frontback");
    
    __ASSERT_EQ__('f', str.front());
    __ASSERT_EQ__('k', str.back());
    
    str.front() = 'F';
    str.back() = 'K';
    
    __ASSERT_EQ__('F', str[0]);
    __ASSERT_EQ__('K', str[8]);
}

__TEST__(StringDataAndCStr) {
    Containers::String str("data");
    
    const char* cstr = str.cStr();
    char* data = str.data();

    __ASSERT__(strcmp("data", cstr) == 0);
    __ASSERT__(strcmp("data", data) == 0);
    
    data[0] = 'D';
    __ASSERT_EQ__('D', str[0]);
}

__TEST__(StringCapacityOperations) {
    Containers::String str;
    
    str.reserve(100);
    __ASSERT__(str.capacity() >= 100);
    
    str = "small";
    str.shrinkToFit();
    __ASSERT__(str.capacity() >= str.size());
}

__TEST__(StringResize) {
    Containers::String str("resize");
    
    str.resize(3);
    __ASSERT_EQ__(3, str.size());
    __ASSERT_EQ__("res", str);
    
    str.resize(6);
    __ASSERT_EQ__(6, str.size());
}

__TEST__(StringClear) {
    Containers::String str("clear");
    
    str.clear();
    __ASSERT_EQ__(0, str.size());
    __ASSERT__(str.empty());
}

__TEST__(StringPushPopBack) {
    Containers::String str("push");
    
    str.pushBack('!');
    __ASSERT_EQ__(5, str.size());
    __ASSERT_EQ__('!', str.back());
    
    str.popBack();
    __ASSERT_EQ__(4, str.size());
    __ASSERT_EQ__('h', str.back());
}

__TEST__(StringAppendString) {
    Containers::String str1("hello");
    Containers::String str2(" world");
    
    str1.append(str2);
    
    __ASSERT_EQ__(11, str1.size());
    __ASSERT_EQ__("hello world", str1);
}

__TEST__(StringAppendCString) {
    Containers::String str("append");
    
    str.append(" test");
    
    __ASSERT_EQ__(11, str.size());
    __ASSERT_EQ__("append test", str);
}

__TEST__(StringAppendCharArray) {
    Containers::String str("append");
    const char arr[] = " array";
    
    str.append(arr);
    
    __ASSERT_EQ__(12, str.size());
    __ASSERT_EQ__("append array", str);
}

__TEST__(StringAppendChars) {
    Containers::String str("repeat");
    
    str.append(3, '!');
    
    __ASSERT_EQ__(9, str.size());
    __ASSERT_EQ__("repeat!!!", str);
}

__TEST__(StringAppendSubstring) {
    Containers::String str1("base");
    Containers::String str2("extraction");
    
    str1.append(str2, 2, 4); // "trac"
    
    __ASSERT_EQ__(8, str1.size());
    __ASSERT_EQ__("basetrac", str1);
}

__TEST__(StringIterators) {
    Containers::String str("iterate");
    
    char expected = 'i';
    for (auto it = str.begin(); it != str.end(); ++it) {
        __ASSERT_EQ__(expected, *it);
        if (expected == 'i') expected = 't';
        else if (expected == 't') expected = 'e';
        else if (expected == 'e') expected = 'r';
        else if (expected == 'r') expected = 'a';
        else if (expected == 'a') expected = 't';
        else expected = 'e';
    }
}

__TEST__(StringConstIterators) {
    const Containers::String str("const");
    
    size_t count = 0;
    for (auto it = str.cbegin(); it != str.cend(); ++it) {
        count++;
    }
    
    __ASSERT_EQ__(5, count);
}

__TEST__(StringRangeBasedLoop) {
    Containers::String str("range");
    
    size_t count = 0;
    for (const auto& ch : str) {
        count++;
    }
    
    __ASSERT_EQ__(5, count);
}

__TEST__(StringModifyThroughIterator) {
    Containers::String str("modify");
    
    for (auto& ch : str) {
        if (ch == 'm') ch = 'M';
        if (ch == 'y') ch = 'Y';
    }
    
    __ASSERT_EQ__("ModifY", str);
}

__TEST__(StringInsertChar) {
    Containers::String str("insert");
    
    str.insert(str.begin() + 3, 'X');
    
    __ASSERT_EQ__(7, str.size());
    __ASSERT_EQ__("insXert", str);
}

__TEST__(StringEraseChar) {
    Containers::String str("erase");
    
    str.erase(str.begin() + 1);
    
    __ASSERT_EQ__(4, str.size());
    __ASSERT_EQ__("ease", str);
}

__TEST__(StringEraseRange) {
    Containers::String str("eraserange");
    
    str.erase(str.begin() + 2, str.begin() + 5);
    
    __ASSERT_EQ__(7, str.size());
    __ASSERT_EQ__("errange", str);
}

//__TEST__(StringOutputStream) {
//    Containers::String str("output");
//    
//    std::ostringstream oss;
//    oss << str;
//    
//    __ASSERT_EQ__("output", oss.str());
//}