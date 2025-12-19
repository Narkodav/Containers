#pragma once

// WARNING: This is a dangerous hack that breaks encapsulation
// Use only if absolutely necessary and you understand the risks
#define private public
#include <vector>
#undef private

template<typename T>
class HackyVector {
private:
    T* data_;
    size_t size_;
    size_t capacity_;

public:
    // Steal data from std::vector (DANGEROUS!)
    HackyVector(std::vector<T>&& vec) {
        // Access private members (implementation-dependent!)
        data_ = vec._Myfirst;      // MSVC implementation
        size_ = vec._Mylast - vec._Myfirst;
        capacity_ = vec._Myend - vec._Myfirst;
        
        // Prevent destructor from freeing memory
        vec._Myfirst = nullptr;
        vec._Mylast = nullptr;
        vec._Myend = nullptr;
    }
    
    ~HackyVector() {
        if (data_) {
            delete[] data_;
        }
    }
    
    T* release() {
        T* result = data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
        return result;
    }
    
    T& operator[](size_t index) { return data_[index]; }
    size_t size() const { return size_; }
};