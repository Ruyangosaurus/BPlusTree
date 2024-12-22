#ifndef TESTING_HEADER_DEFINED
#define TESTING_HEADER_DEFINED
#define DEBUGGING_B_PLUS_TREE
#include "../src/BPlusTree.hpp"
#include <cassert>
#include <iostream>
#include <functional>

const char* const failed = " - FAILED\n";
const char* const passed = " - PASSED\n";

#define _ASSERT(smth) if (!(smth)) return failed

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
