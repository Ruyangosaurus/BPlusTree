#include "BPlusTest.hpp"

void BPlusTest::test_crud(){
    tester("1. rising order", []{
        BPlusTree<int,int,3> tree;
        for (int i = 0; i < 24; ++i){
            tree.insert(i,i);
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at(j) == j);
            }
        }

        for (int i = 0; i < 24; ++i){
            tree.erase(i);
            for (int j = i + 1; j < 24; ++j){
                _ASSERT(tree.at(j) == j);
            }
        }
        return passed;
    });

    tester("2. falling order", []{
        BPlusTree<int,int,3> tree;
        for (int i = 23; i >= 0; --i){
            tree.insert(i,i);
            for (int j = 23; j >= i; --j){
                _ASSERT(tree.at(j) == j);
            }
        }

        for (int i = 23; i >= 0; --i){
            tree.erase(i);
            for (int j = i - 1; j >= 0; --j){
                _ASSERT(tree.at(j) == j);
            }
        }
        return passed;
    });

    tester("3. zigzag", []{
        int array [24] = {0,23,1,22,2,21,3,20,4,19,5,18,6,17,7,16,8,15,9,14,10,13,11,12};
        BPlusTree<int,int,3> tree;
        for (int i = 0; i < 24; ++i){
            tree.insert(array[i],array[i]);
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
        }

        for (int i = 0; i < 24; ++i){
            tree.erase(array[i]);
            for (int j = i + 1; j < 24; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
        }
        return passed;
    });

    tester("4. steps of seven", []{ // 7 and 24 are coprime
        BPlusTree<int,int,3> tree;
        for (int i = 0; i < 24; ++i){
            tree.insert((i*7)%24,i);
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at((j*7)%24) == j);
            }
        }

        for (int i = 0; i < 24; ++i){
            tree.erase((i*7)%24);
            for (int j = i + 1; j < 24; ++j){
                _ASSERT(tree.at((j*7)%24) == j);
            }
        }
        return passed;
    });
}
