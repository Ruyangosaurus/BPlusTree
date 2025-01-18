#include "BPlusTest.hpp"

const char* contains_tester_aux (std::function<int (int)>lambda){
    csaur::BPlusTree<int,int,3> tree;
    for (int i = 0; i < BPlusTest::num_inserted; ++i){
        tree.insert(lambda(i),lambda(i));
        for (int j = 0; j <= i; ++j){
            _ASSERT(tree.contains(lambda(j)));
        }
        for (int j = i + 1;j < BPlusTest::num_inserted;++j){
            _ASSERT(!tree.contains(lambda(j)));
        }
    }

    for (int i = 0; i < BPlusTest::num_inserted; ++i){
        tree.erase(lambda(i));
        for (int j = 0 ;j < i+1;++j){
            _ASSERT(!tree.contains(lambda(j)));
        }
        for (int j = i + 1; j < BPlusTest::num_inserted; ++j){
            _ASSERT(tree.contains(lambda(j)));
        }
    }
    return passed;
};

void BPlusTest::test_observers()
{
    tester("1. size in insertion", [&](){
        csaur::BPlusTree<int,int,3> tree;
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
        csaur::BPlusTree<int,int,3> tree;
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
        csaur::BPlusTree<int,int,3> tree;
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
        csaur::BPlusTree<int,int,3> tree;
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
        csaur::BPlusTree<int,int,3> tree;
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
        csaur::BPlusTree<int,int,3> tree;
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

    tester("7. contains rising order", []{return contains_tester_aux([](int x){return x;});});

    tester("8. contains falling order", []{return contains_tester_aux([](int x){return BPlusTest::num_inserted - 1 - x;});});

    tester("9. contains zigzag", []{return contains_tester_aux([](int x){return (x%2)?x/2:BPlusTest::num_inserted - 1 - x/2;
                                                            });});
    tester("10. contains steps of seven",[]{return contains_tester_aux([](int x){return (x*7)%BPlusTest::num_inserted;});}); // 7 and BPlusTest::num_inserted are coprime
}

