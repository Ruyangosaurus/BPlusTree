#include "BPLusTest.hpp"

void BPlusTest::tester(const char* const name, std::function<const char* const ()> func) {
    std::cout << name << func();
}

void BPlusTest::operator()()
{
    std::cout << "TESTING INTERNAL NODE STRUCTURE\n";
    test_nodes();
}