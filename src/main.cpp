#include "../include/LinearStorage/EnumArray.h"
#include "../include/LinearStorage/Span.h"

#include <span>
#include <memory>
#include <list>
#include <array>
#include <iostream>

consteval Containers::Array<int, 3> asdf(Containers::Span<const int> span)
{
    constexpr Containers::Array<int, 3> arr = {1, 2, 3};
    return arr;
}

// template<Containers::LinearContainerType Container>
// void foo2(const Container& arr) {};

template <typename T>
class CustomDeleter
{
public:
    int data = 0;
    void operator()(T *ptr) noexcept
    {
        std::cout << "Custom deleting pointer " << data << std::endl;
        delete ptr;
    }
};

template <typename T>
class CustomDeleter<T[]>
{
public:
    int data = 0;
    void operator()(T *ptr) noexcept
    {
        std::cout << "Custom deleting pointer " << data << std::endl;
        delete[] ptr;
    }
};

constexpr void foo(std::span<int> span) {

};

class Class
{
public:
    int value = 42;

    Class() { std::cout << "Class constructed" << std::endl; }
    ~Class() { std::cout << "Class destructed" << std::endl; }
};

int main()
{
    // Containers::DirectAccessAllocators::BuddyAllocatorBase alloc;
    // auto metadataSize = Containers::DirectAccessAllocators::BuddyAllocatorBase::computeMetadataSize(1024 * 128);
    // void* heap = malloc(1024 * 128 + metadataSize);
    // alloc.assign(heap, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(heap) + metadataSize), 1024 * 128);

    // uint8_t** pointers = alloc.allocate<uint8_t*>(100);
    // for (int i = 0; i < 100; i++) {
    //     pointers[i] = alloc.allocate<uint8_t>(100);
    // }

    // for (int i = 0; i < 100; i++) {
    //     alloc.deallocate(pointers[i]);
    // }
    // alloc.deallocate(pointers);

    // free(heap);

    enum class MyEnum : uint8_t
    {
        A,
        B,
        C,
        D,
        E,
        F,
        G,

        Count
    };

    Containers::EnumArray<int, MyEnum, MyEnum::Count> enumArray = {10, 20, 30, 40, 50};

    enumArray[MyEnum::A] = 3;

    // std::cout << "\n" << *ptr << std::endl;
    return 0;
}