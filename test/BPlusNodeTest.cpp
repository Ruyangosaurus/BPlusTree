#include "../src/BPlusNode.hpp"
#include <cassert>
#include <iostream>
#include <functional>

const char* const failed = " - FAILED\n";
const char* const passed = " - PASSED\n";

#define _ASSERT(smth) if (!(smth)) return failed
#define _CONCLUDE return passed

void tester(const char* const name, std::function<const char* const ()> func) {
    std::cout << name << func();
}

int main(){
    std::cout << "BPlusNode TESTS:\n";

    tester("1. empty leaf", []{
        BPlusNode<4,int,int> even_leaf (true);
        BPlusNode<5,int,int> odd_leaf (true);
        _ASSERT(even_leaf.m_key_counter == 0);
        _ASSERT(odd_leaf.m_key_counter == 0);
        _ASSERT(even_leaf.m_next == nullptr);
        _ASSERT(odd_leaf.m_next == nullptr);
        _ASSERT(even_leaf.m_prev == nullptr);
        _ASSERT(odd_leaf.m_prev == nullptr);
        _ASSERT(even_leaf.search(0) == nullptr);
        _ASSERT(odd_leaf.search(0) == nullptr);
        _ASSERT(even_leaf.erase(0) == false);
        _ASSERT(odd_leaf.erase(0) == false);
        even_leaf.erase_all();
        odd_leaf.erase_all();
        _CONCLUDE;
    });

    tester("2. basic insertion", []{
        BPlusNode<4,int,int> even_leaf (true);
        BPlusNode<5,int,int> odd_leaf (true);
        even_leaf.insert(0,0);
        even_leaf.insert(10,10);
        even_leaf.insert(30,30);
        even_leaf.insert(20,20);
        odd_leaf.insert(20,20);
        odd_leaf.insert(40,40);
        odd_leaf.insert(30,30);
        odd_leaf.insert(0,0);
        odd_leaf.insert(10,10);
        
        _ASSERT(even_leaf.m_key_counter == 4);
        _ASSERT(odd_leaf.m_key_counter == 5);
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(0)) == 0);
        _ASSERT(*(even_leaf.search(10)) == 10);
        _ASSERT(*(odd_leaf.search(10)) == 10);
        _ASSERT(*(even_leaf.search(20)) == 20);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(*(even_leaf.search(30)) == 30);
        _ASSERT(*(odd_leaf.search(30)) == 30);
        _ASSERT(*(odd_leaf.search(40)) == 40);
        even_leaf.erase_all();
        odd_leaf.erase_all();
        _ASSERT(even_leaf.m_key_counter == 0);
        _ASSERT(odd_leaf.m_key_counter == 0);
        _CONCLUDE;
    });

    tester("3. insertion above split", []{
        BPlusNode<4,int,int> even_leaf (true);
        BPlusNode<5,int,int> odd_leaf (true);
        even_leaf.insert(10,10);
        even_leaf.insert(20,20);
        even_leaf.insert(40,40);
        even_leaf.insert(0,0);

        BPlusNode<4, int, int>* new_even_leaf = even_leaf.insert(30,30);
        _ASSERT(even_leaf.m_next == new_even_leaf);
        _ASSERT(new_even_leaf->m_prev == &even_leaf);
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(even_leaf.search(10)) == 10);
        _ASSERT(((even_leaf.search(20) != nullptr) && (*(even_leaf.search(20)) == 20)) || (*(new_even_leaf->search(20)) == 20));
        _ASSERT(*(new_even_leaf->search(30)) == 30);
        _ASSERT(*(new_even_leaf->search(40)) == 40);
        even_leaf.erase_all();
        new_even_leaf->erase_all();
        delete new_even_leaf;

        odd_leaf.insert(10,10);
        odd_leaf.insert(20,20);
        odd_leaf.insert(30,30);
        odd_leaf.insert(50,50);
        odd_leaf.insert(0,0);

        BPlusNode<5, int, int>* new_odd_leaf = odd_leaf.insert(40,40);
        _ASSERT(odd_leaf.m_next == new_odd_leaf);
        _ASSERT(new_odd_leaf->m_prev == &odd_leaf);
        _ASSERT(*(odd_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(10)) == 10);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(*(new_odd_leaf->search(30)) == 30);
        _ASSERT(*(new_odd_leaf->search(40)) == 40);
        _ASSERT(*(new_odd_leaf->search(50)) == 50);
        odd_leaf.erase_all();
        new_odd_leaf->erase_all();
        delete new_odd_leaf;
        _CONCLUDE;
    });

    tester("4. insertion below split", []{
        BPlusNode<4,int,int> even_leaf (true);
        BPlusNode<5,int,int> odd_leaf (true);
        even_leaf.insert(30,30);
        even_leaf.insert(20,20);
        even_leaf.insert(40,40);
        even_leaf.insert(0,0);

        BPlusNode<4, int, int>* new_even_leaf = even_leaf.insert(10,10);
        _ASSERT(even_leaf.m_next == new_even_leaf);
        _ASSERT(new_even_leaf->m_prev == &even_leaf);
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(even_leaf.search(10)) == 10);
        _ASSERT(((even_leaf.search(20) != nullptr) && (*(even_leaf.search(20)) == 20)) || (*(new_even_leaf->search(20)) == 20));
        _ASSERT(*(new_even_leaf->search(30)) == 30);
        _ASSERT(*(new_even_leaf->search(40)) == 40);
        even_leaf.erase_all();
        new_even_leaf->erase_all();
        delete new_even_leaf;

        odd_leaf.insert(40,40);
        odd_leaf.insert(20,20);
        odd_leaf.insert(30,30);
        odd_leaf.insert(50,50);
        odd_leaf.insert(0,0);

        BPlusNode<5, int, int>* new_odd_leaf = odd_leaf.insert(10,10);
        _ASSERT(odd_leaf.m_next == new_odd_leaf);
        _ASSERT(new_odd_leaf->m_prev == &odd_leaf);
        _ASSERT(*(odd_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(10)) == 10);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(*(new_odd_leaf->search(30)) == 30);
        _ASSERT(*(new_odd_leaf->search(40)) == 40);
        _ASSERT(*(new_odd_leaf->search(50)) == 50);
        odd_leaf.erase_all();
        new_odd_leaf->erase_all();
        delete new_odd_leaf;
        _CONCLUDE;
    });

    tester("5. insertion near split", []{
        BPlusNode<4,int,int> even_leaf (true);
        BPlusNode<5,int,int> odd_leaf (true);
        even_leaf.insert(30,30);
        even_leaf.insert(10,10);
        even_leaf.insert(40,40);
        even_leaf.insert(0,0);

        BPlusNode<4, int, int>* new_even_leaf = even_leaf.insert(20,20);
        _ASSERT(even_leaf.m_next == new_even_leaf);
        _ASSERT(new_even_leaf->m_prev == &even_leaf);
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(even_leaf.search(10)) == 10);
        _ASSERT(((even_leaf.search(20) != nullptr) && (*(even_leaf.search(20)) == 20)) || (*(new_even_leaf->search(20)) == 20));
        _ASSERT(*(new_even_leaf->search(30)) == 30);
        _ASSERT(*(new_even_leaf->search(40)) == 40);
        even_leaf.erase_all();
        new_even_leaf->erase_all();
        delete new_even_leaf;

        odd_leaf.insert(40,40);
        odd_leaf.insert(10,10);
        odd_leaf.insert(30,30);
        odd_leaf.insert(50,50);
        odd_leaf.insert(0,0);

        BPlusNode<5, int, int>* new_odd_leaf = odd_leaf.insert(20,20);
        _ASSERT(odd_leaf.m_next == new_odd_leaf);
        _ASSERT(new_odd_leaf->m_prev == &odd_leaf);
        _ASSERT(*(odd_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(10)) == 10);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(*(new_odd_leaf->search(30)) == 30);
        _ASSERT(*(new_odd_leaf->search(40)) == 40);
        _ASSERT(*(new_odd_leaf->search(50)) == 50);
        odd_leaf.erase_all();
        new_odd_leaf->erase_all();
        delete new_odd_leaf;

        odd_leaf.insert(40,40);
        odd_leaf.insert(10,10);
        odd_leaf.insert(20,20);
        odd_leaf.insert(50,50);
        odd_leaf.insert(0,0);

        new_odd_leaf = odd_leaf.insert(30,30);
        _ASSERT(odd_leaf.m_next == new_odd_leaf);
        _ASSERT(new_odd_leaf->m_prev == &odd_leaf);
        _ASSERT(*(odd_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(10)) == 10);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(*(new_odd_leaf->search(30)) == 30);
        _ASSERT(*(new_odd_leaf->search(40)) == 40);
        _ASSERT(*(new_odd_leaf->search(50)) == 50);
        odd_leaf.erase_all();
        new_odd_leaf->erase_all();
        delete new_odd_leaf;
        _CONCLUDE;
    });

    tester("6. duplicate insertion", []{
        BPlusNode<4,int,int> even_leaf (true);
        BPlusNode<5,int,int> odd_leaf (true);
        even_leaf.insert(0,0);
        odd_leaf.insert(10,10);
        
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(10)) == 10);

        even_leaf.insert(0,20);
        odd_leaf.insert(10,30);
        _ASSERT(even_leaf.m_key_counter == 1);
        _ASSERT(odd_leaf.m_key_counter == 1);
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(10)) == 10);

        even_leaf.erase_all();
        odd_leaf.erase_all();
        _ASSERT(even_leaf.m_key_counter == 0);
        _ASSERT(odd_leaf.m_key_counter == 0);
        _CONCLUDE;
    });

    tester("7. deletion", []{
        BPlusNode<4,int,int> even_leaf (true);
        BPlusNode<5,int,int> odd_leaf (true);
        even_leaf.insert(0,0);
        even_leaf.insert(10,10);
        even_leaf.insert(30,30);
        even_leaf.insert(20,20);
        odd_leaf.insert(20,20);
        odd_leaf.insert(40,40);
        odd_leaf.insert(30,30);
        odd_leaf.insert(0,0);
        odd_leaf.insert(10,10);
        
        _ASSERT(even_leaf.m_key_counter == 4);
        _ASSERT(odd_leaf.m_key_counter == 5);
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(0)) == 0);
        _ASSERT(*(even_leaf.search(10)) == 10);
        _ASSERT(*(odd_leaf.search(10)) == 10);
        _ASSERT(*(even_leaf.search(20)) == 20);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(*(even_leaf.search(30)) == 30);
        _ASSERT(*(odd_leaf.search(30)) == 30);
        _ASSERT(*(odd_leaf.search(40)) == 40);

        even_leaf.erase(0);
        odd_leaf.erase(0);
        _ASSERT(even_leaf.m_key_counter == 3);
        _ASSERT(odd_leaf.m_key_counter == 4);
        _ASSERT(even_leaf.search(0) == nullptr);
        _ASSERT(odd_leaf.search(0) == nullptr);
        _ASSERT(*(even_leaf.search(10)) == 10);
        _ASSERT(*(odd_leaf.search(10)) == 10);
        _ASSERT(*(even_leaf.search(20)) == 20);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(*(even_leaf.search(30)) == 30);
        _ASSERT(*(odd_leaf.search(30)) == 30);
        _ASSERT(*(odd_leaf.search(40)) == 40);

        even_leaf.erase(10);
        odd_leaf.erase(10);
        _ASSERT(even_leaf.m_key_counter == 2);
        _ASSERT(odd_leaf.m_key_counter == 3);
        _ASSERT(even_leaf.search(0) == nullptr);
        _ASSERT(odd_leaf.search(0) == nullptr);
        _ASSERT(even_leaf.search(10) == nullptr);
        _ASSERT(odd_leaf.search(10) == nullptr);
        _ASSERT(*(even_leaf.search(20)) == 20);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(*(even_leaf.search(30)) == 30);
        _ASSERT(*(odd_leaf.search(30)) == 30);
        _ASSERT(*(odd_leaf.search(40)) == 40);

        even_leaf.erase(30);
        odd_leaf.erase(30);
        _ASSERT(even_leaf.m_key_counter == 1);
        _ASSERT(odd_leaf.m_key_counter == 2);
        _ASSERT(even_leaf.search(0) == nullptr);
        _ASSERT(odd_leaf.search(0) == nullptr);
        _ASSERT(even_leaf.search(10) == nullptr);
        _ASSERT(odd_leaf.search(10) == nullptr);
        _ASSERT(*(even_leaf.search(20)) == 20);
        _ASSERT(*(odd_leaf.search(20)) == 20);
        _ASSERT(even_leaf.search(30) == nullptr);
        _ASSERT(odd_leaf.search(30) == nullptr);
        _ASSERT(*(odd_leaf.search(40)) == 40);

        even_leaf.erase(20);
        odd_leaf.erase(20);
        _ASSERT(even_leaf.m_key_counter == 0);
        _ASSERT(odd_leaf.m_key_counter == 1);
        _ASSERT(even_leaf.search(0) == nullptr);
        _ASSERT(odd_leaf.search(0) == nullptr);
        _ASSERT(even_leaf.search(10) == nullptr);
        _ASSERT(odd_leaf.search(10) == nullptr);
        _ASSERT(even_leaf.search(20) == nullptr);
        _ASSERT(odd_leaf.search(20) == nullptr);
        _ASSERT(even_leaf.search(30) == nullptr);
        _ASSERT(odd_leaf.search(30) == nullptr);
        _ASSERT(*(odd_leaf.search(40)) == 40);

        even_leaf.erase_all();
        odd_leaf.erase_all();
        _ASSERT(even_leaf.m_key_counter == 0);
        _ASSERT(odd_leaf.m_key_counter == 0);
        _CONCLUDE;
    });

    std::cout << "BPlusInternalNode TESTS:\n";
    
    tester("8. search pt. 1", []{
        BPlusNode<4,int,int> even_leaf_1 (true);
        even_leaf_1.insert(0,0);
        even_leaf_1.insert(10,10);
        even_leaf_1.insert(30,30);
        even_leaf_1.insert(20,20);
        BPlusNode<4,int,int> even_leaf_2 (true);
        even_leaf_2.insert(40,40);
        even_leaf_2.insert(50,50);
        even_leaf_2.insert(60,60);
        even_leaf_2.insert(70,70);
        BPlusNode<4,int,int> even_node (false);
        even_node.m_key_counter = 2;
        std::get<1>(even_node.m_data)[0] = &even_leaf_1;
        std::get<1>(even_node.m_data)[1] = &even_leaf_2;
        even_node.m_keys[0] = even_leaf_1.m_keys[0];
        even_node.m_keys[1] = even_leaf_2.m_keys[0];

        _ASSERT(even_node.m_key_counter == 2);
        _ASSERT(even_node.m_keys[0] == 0);
        _ASSERT(even_node.m_keys[1] == 40);
        _ASSERT(std::get<1>(even_node.m_data)[0] == &even_leaf_1);
        _ASSERT(std::get<1>(even_node.m_data)[1] == &even_leaf_2);
        _ASSERT(*(even_node.search(0)) == 0);
        _ASSERT(*(even_node.search(10)) == 10);
        _ASSERT(*(even_node.search(20)) == 20);
        _ASSERT(*(even_node.search(30)) == 30);
        _ASSERT(*(even_node.search(40)) == 40);
        _ASSERT(*(even_node.search(50)) == 50);
        _ASSERT(*(even_node.search(60)) == 60);
        _ASSERT(*(even_node.search(70)) == 70);
        even_leaf_1.erase_all();
        even_leaf_2.erase_all();
        _CONCLUDE;
    });

    return 0;
}
