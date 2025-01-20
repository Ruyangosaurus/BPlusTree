#ifndef TESTING_HEADER_DEFINED
#define TESTING_HEADER_DEFINED
#define DEBUGGING_B_PLUS_TREE

class BPlusTest;

#include "../src/BPlusTree.hpp"
#include <cassert>
#include <iostream>
#include <functional>

const char* const failed = " - FAILED\n";
const char* const passed = " - PASSED\n";

#define _ASSERT(smth) if (!(smth)) return failed

template<class T>
class FourMaxAllocator{
    int flag;
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    FourMaxAllocator(){flag = 0;}
    FourMaxAllocator(const FourMaxAllocator&) = delete;
    FourMaxAllocator& operator=(const FourMaxAllocator&) = delete;

    pointer allocate();
    pointer allocate(size_t);
    void deallocate(pointer);
};

template <class T>
typename FourMaxAllocator<T>::pointer FourMaxAllocator<T>::allocate()
{
    if (flag == 4){throw std::bad_alloc();}
    ++flag;
    return (FourMaxAllocator<T>::pointer)std::malloc(sizeof(value_type));
}

template <class T>
void FourMaxAllocator<T>::deallocate(pointer node)
{
    --flag;
    std::free(node);
}

class BPlusTest{
    /// @brief Tests the correctness of behaviour of the nodes of the tree.
    void test_nodes();

    /// @brief Tests the correctness of behaviour of insert, erase, and at.
    void test_crud();

    /// @brief Tests the correctness of behaviour of size and is_empty
    void test_observers();

    /// @brief Auxiliary testing function, prints the name and the output of the function.
    /// @param name The name of the test.
    /// @param func The test itself, returns a c-string.
    static void tester(const char* const name, std::function<const char* const ()> func);
public:
    static constexpr int num_inserted = 24;
    
    /// @brief Runs all the tests
    void operator()();
};

#endif
