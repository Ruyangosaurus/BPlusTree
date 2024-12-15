#include "BPlusTestNode.hpp"
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
    std::cout << "BPlusLeafNode TESTS:\n";

    tester("1. empty leaf", []{
        BPlusLeafTestInterface<4,int,int> even_leaf;
        BPlusLeafTestInterface<5,int,int> odd_leaf;
        _ASSERT(even_leaf.get_key_num() == 0);
        _ASSERT(odd_leaf.get_key_num() == 0);
        _ASSERT(even_leaf.get_next() == nullptr);
        _ASSERT(odd_leaf.get_next() == nullptr);
        _ASSERT(even_leaf.get_prev() == nullptr);
        _ASSERT(odd_leaf.get_prev() == nullptr);
        _ASSERT(even_leaf.search(0) == nullptr);
        _ASSERT(odd_leaf.search(0) == nullptr);
        _ASSERT(even_leaf.erase(0) == false);
        _ASSERT(odd_leaf.erase(0) == false);
        even_leaf.erase_all();
        odd_leaf.erase_all();
        _CONCLUDE;
    });

    tester("2. basic insertion", []{
        BPlusLeafTestInterface<4,int,int> even_leaf;
        BPlusLeafTestInterface<5,int,int> odd_leaf;
        even_leaf.insert(0,0);
        even_leaf.insert(10,10);
        even_leaf.insert(30,30);
        even_leaf.insert(20,20);
        odd_leaf.insert(20,20);
        odd_leaf.insert(40,40);
        odd_leaf.insert(30,30);
        odd_leaf.insert(0,0);
        odd_leaf.insert(10,10);
        
        _ASSERT(even_leaf.get_key_num() == 4);
        _ASSERT(odd_leaf.get_key_num() == 5);
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
        _ASSERT(even_leaf.get_key_num() == 0);
        _ASSERT(odd_leaf.get_key_num() == 0);
        _CONCLUDE;
    });

    tester("3. insertion above split", []{
        BPlusLeafTestInterface<4,int,int> even_leaf;
        BPlusLeafTestInterface<5,int,int> odd_leaf;
        even_leaf.insert(10,10);
        even_leaf.insert(20,20);
        even_leaf.insert(40,40);
        even_leaf.insert(0,0);

        BPlusLeafNode<4, int, int>* new_even_leaf = dynamic_cast<BPlusLeafNode<4, int, int>*>(even_leaf.insert(30,30));
        _ASSERT(even_leaf.is_split_from_me(new_even_leaf));
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

        BPlusLeafNode<5, int, int>* new_odd_leaf = dynamic_cast<BPlusLeafNode<5, int, int>*>(odd_leaf.insert(40,40));
        _ASSERT(odd_leaf.is_split_from_me(new_odd_leaf));
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
        BPlusLeafTestInterface<4,int,int> even_leaf;
        BPlusLeafTestInterface<5,int,int> odd_leaf;
        even_leaf.insert(30,30);
        even_leaf.insert(20,20);
        even_leaf.insert(40,40);
        even_leaf.insert(0,0);

        BPlusLeafNode<4, int, int>* new_even_leaf = dynamic_cast<BPlusLeafNode<4, int, int>*>(even_leaf.insert(10,10));
        _ASSERT(even_leaf.is_split_from_me(new_even_leaf));
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

        BPlusLeafNode<5, int, int>* new_odd_leaf = dynamic_cast<BPlusLeafNode<5, int, int>*>(odd_leaf.insert(10,10));
        _ASSERT(odd_leaf.is_split_from_me(new_odd_leaf));
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
        BPlusLeafTestInterface<4,int,int> even_leaf;
        BPlusLeafTestInterface<5,int,int> odd_leaf;
        even_leaf.insert(30,30);
        even_leaf.insert(10,10);
        even_leaf.insert(40,40);
        even_leaf.insert(0,0);

        BPlusLeafNode<4, int, int>* new_even_leaf = dynamic_cast<BPlusLeafNode<4, int, int>*>(even_leaf.insert(20,20));
        _ASSERT(even_leaf.is_split_from_me(new_even_leaf));
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

        BPlusLeafNode<5, int, int>* new_odd_leaf = dynamic_cast<BPlusLeafNode<5, int, int>*>(odd_leaf.insert(20,20));
        _ASSERT(odd_leaf.is_split_from_me(new_odd_leaf));
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

        new_odd_leaf = dynamic_cast<BPlusLeafNode<5, int, int>*>(odd_leaf.insert(30,30));
        _ASSERT(odd_leaf.is_split_from_me(new_odd_leaf));
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
        BPlusLeafTestInterface<4,int,int> even_leaf;
        BPlusLeafTestInterface<5,int,int> odd_leaf;
        even_leaf.insert(0,0);
        odd_leaf.insert(10,10);
        
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(10)) == 10);

        even_leaf.insert(0,20);
        odd_leaf.insert(10,30);
        _ASSERT(even_leaf.get_key_num() == 1);
        _ASSERT(odd_leaf.get_key_num() == 1);
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(odd_leaf.search(10)) == 10);

        even_leaf.erase_all();
        odd_leaf.erase_all();
        _ASSERT(even_leaf.get_key_num() == 0);
        _ASSERT(odd_leaf.get_key_num() == 0);
        _CONCLUDE;
    });

    tester("7. deletion", []{
        BPlusLeafTestInterface<4,int,int> even_leaf;
        BPlusLeafTestInterface<5,int,int> odd_leaf;
        even_leaf.insert(0,0);
        even_leaf.insert(10,10);
        even_leaf.insert(30,30);
        even_leaf.insert(20,20);
        odd_leaf.insert(20,20);
        odd_leaf.insert(40,40);
        odd_leaf.insert(30,30);
        odd_leaf.insert(0,0);
        odd_leaf.insert(10,10);
        
        _ASSERT(even_leaf.get_key_num() == 4);
        _ASSERT(odd_leaf.get_key_num() == 5);
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
        _ASSERT(even_leaf.get_key_num() == 3);
        _ASSERT(odd_leaf.get_key_num() == 4);
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
        _ASSERT(even_leaf.get_key_num() == 2);
        _ASSERT(odd_leaf.get_key_num() == 3);
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
        _ASSERT(even_leaf.get_key_num() == 1);
        _ASSERT(odd_leaf.get_key_num() == 2);
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
        _ASSERT(even_leaf.get_key_num() == 0);
        _ASSERT(odd_leaf.get_key_num() == 1);
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
        _ASSERT(even_leaf.get_key_num() == 0);
        _ASSERT(odd_leaf.get_key_num() == 0);
        _CONCLUDE;
    });

    std::cout << "BPlusInternalNode TESTS:\n";
    
    tester("8. search pt. 1", []{
        BPlusLeafTestInterface<4,int,int> even_leaf_1;
        even_leaf_1.insert(0,0);
        even_leaf_1.insert(10,10);
        even_leaf_1.insert(30,30);
        even_leaf_1.insert(20,20);
        BPlusLeafTestInterface<4,int,int> even_leaf_2;
        even_leaf_2.insert(40,40);
        even_leaf_2.insert(50,50);
        even_leaf_2.insert(60,60);
        even_leaf_2.insert(70,70);
        BPlusInternalNodeTestInterface<4,int,int> even_node (&even_leaf_1);
        even_node.push_child(&even_leaf_2);

        _ASSERT(even_node.get_key_num() == 1);
        _ASSERT(even_node.get_key(0) == 40);
        _ASSERT(even_node.min_key() == 0);
        _ASSERT(even_node.max_key() == 70);
        _ASSERT(even_node.max_key() == 70);
        _ASSERT(even_node.get_child(0) == &even_leaf_1);
        _ASSERT(even_node.get_child(1) == &even_leaf_2);
        _ASSERT(*(even_leaf.search(0)) == 0);
        _ASSERT(*(even_leaf.search(10)) == 10);
        _ASSERT(*(even_leaf.search(20)) == 20);
        _ASSERT(*(even_leaf.search(30)) == 30);
        _ASSERT(*(even_leaf.search(40)) == 40);
        _ASSERT(*(even_leaf.search(50)) == 50);
        _ASSERT(*(even_leaf.search(60)) == 60);
        _ASSERT(*(even_leaf.search(70)) == 70);
        even_leaf_1.erase_all();
        even_leaf_2.erase_all();
        _CONCLUDE;
    });

    return 0;
}
