#include "BPlusTest.hpp"

void BPlusTest::test_observers()
{
    tester("1. size in insertion", [&](){
        BPlusTree<int,int,3> tree;
        _ASSERT(tree.size() == 0);
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(i,i);
            _ASSERT(tree.size() == (size_t)(i + 1));
        }

        tree.erase_all();
        _ASSERT(tree.size() == 0);
        return passed;
    });

    tester("2. size in deletion", [&](){
        BPlusTree<int,int,3> tree;
        _ASSERT(tree.size() == 0);
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(i,i);
            _ASSERT(tree.size() == (size_t)(i + 1));
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            _ASSERT(tree.size() == (size_t)(BPlusTest::num_inserted - i));
            tree.erase(i);
        }
        _ASSERT(tree.size() == 0);
        return passed;
    });

    tester("3. is_empty in insertion", [&](){
        BPlusTree<int,int,3> tree;
        _ASSERT(tree.is_empty() == true);
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(i,i);
            _ASSERT(tree.is_empty() == false);
        }

        tree.erase_all();
        _ASSERT(tree.is_empty() == true);
        return passed;
    });

    tester("4. is_empty in deletion", [&](){
        BPlusTree<int,int,3> tree;
        _ASSERT(tree.is_empty() == true);
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(i,i);
            _ASSERT(tree.is_empty() == false);
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            _ASSERT(tree.is_empty() == false);
            tree.erase(i);
        }
        _ASSERT(tree.is_empty() == true);
        return passed;
    });

    tester("5. extreme keys in insertion", [&](){
        BPlusTree<int,int,3> tree;
        _ASSERT(tree.is_empty() == true);
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(i,i);
            try{
                _ASSERT(tree.min_key() == 0);
                _ASSERT(tree.max_key() == i);
            }
            catch(const std::out_of_range& e){
                return failed;
            }
        }

        tree.erase_all();
        _ASSERT(tree.is_empty() == true);
        return passed;
    });

    tester("6. extreme keys in deletion", [&](){
        BPlusTree<int,int,3> tree;
        _ASSERT(tree.is_empty() == true);
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(i,i);
            try{
                _ASSERT(tree.min_key() == 0);
                _ASSERT(tree.max_key() == i);
            }
            catch(const std::out_of_range& e){
                return failed;
            }
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            try{
                _ASSERT(tree.min_key() == i);
                _ASSERT(tree.max_key() == BPlusTest::num_inserted - 1);
            }
            catch(const std::out_of_range& e){
                return failed;
            }
            tree.erase(i);
        }
        _ASSERT(tree.is_empty() == true);
        return passed;
    });
}

