#include "BPlusTest.hpp"
#include <random>

const char* crd_tester_aux (std::function<int (int)>lambda){
    csaur::BPlusTree<int,int,3> tree;
    for (int i = 0; i < BPlusTest::num_inserted; ++i){
        tree.insert(lambda(i),lambda(i));
        for (int j = 0; j <= i; ++j){
            _ASSERT(tree.at(lambda(j))==lambda(j));
        }
        for (int j = i + 1;j < BPlusTest::num_inserted;++j){
            try{
                tree.at(lambda(j));
                return failed;
            }
            catch(const std::out_of_range& e){
                ;
            }
        }
    }

    for (int i = 0; i < BPlusTest::num_inserted; ++i){
        tree.erase(lambda(i));
        for (int j = 0 ;j < i+1;++j){
            try{
                tree.at(lambda(j));
                return failed;
            }
            catch(const std::out_of_range& e){
                ;
            }
        }
        for (int j = i + 1; j < BPlusTest::num_inserted; ++j){
            _ASSERT(tree.at(lambda(j))==lambda(j));
        }
    }
    return passed;
};

void BPlusTest::test_crud(){

    tester("1. rising order", []{return crd_tester_aux([](int x){return x;});});

    tester("2. falling order", []{return crd_tester_aux([](int x){return BPlusTest::num_inserted - 1 - x;});});

    tester("3. zigzag", []{return crd_tester_aux([](int x){return (x%2)?x/2:BPlusTest::num_inserted - 1 - x/2;
                                                            });});

    tester("4. steps of seven",[]{return crd_tester_aux([](int x){return (x*7)%BPlusTest::num_inserted;});}); // 7 and BPlusTest::num_inserted are coprime

    std::vector<int> array = {0,23,1,22,2,21,3,20,4,19,5,18,6,17,7,16,8,15,9,14,10,13,11,12};
    auto rm =std::default_random_engine(std::random_device{}());
    tester("5. random permutation", [&](){
        std::ranges::shuffle(array.begin(), array.end(),rm);
        csaur::BPlusTree<int,int,3> tree;
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(array[i],array[i]);
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
            for (int j = i + 1;j < BPlusTest::num_inserted;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.erase(array[i]);
            for (int j = 0 ;j < i+1;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
            for (int j = i + 1; j < BPlusTest::num_inserted; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
        }
        return passed;
    });

    tester("6. twice", [&](){
        std::ranges::shuffle(array.begin(), array.end(),rm);
        csaur::BPlusTree<int,int,3> tree;
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(array[i],array[i]);
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
            for (int j = i + 1;j < BPlusTest::num_inserted;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.erase(array[i]);
            for (int j = 0 ;j < i+1;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
            for (int j = i + 1; j < BPlusTest::num_inserted; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
        }

        std::ranges::shuffle(array.begin(), array.end(),rm);

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(array[i],array[i]);
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
            for (int j = i + 1;j < BPlusTest::num_inserted;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.erase(array[i]);
            for (int j = 0 ;j < i+1;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
            for (int j = i + 1; j < BPlusTest::num_inserted; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
        }
        return passed;
    });

    tester("7. update", [&](){
        std::ranges::shuffle(array.begin(), array.end(),rm);
        csaur::BPlusTree<int,int,3> tree;
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(array[i],array[i]);
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
            for (int j = i + 1;j < BPlusTest::num_inserted;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            try{
                tree.at(array[i]) = i;
            }
            catch(const std::out_of_range& e){
                return failed;
            }
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            try{
                _ASSERT(tree.at(array[i]) == i);
            }
            catch(const std::out_of_range& e){
                return failed;
            }
        }

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.erase(array[i]);
            for (int j = 0 ;j < i+1;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
            for (int j = i + 1; j < BPlusTest::num_inserted; ++j){
                _ASSERT(tree.at(array[j]) == j);
            }
        }

        return passed;
    });

    tester("8. erase all", [&](){
        std::ranges::shuffle(array.begin(), array.end(),rm);
        csaur::BPlusTree<int,int,3> tree;
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.insert(array[i],array[i]);
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
            for (int j = i + 1;j < BPlusTest::num_inserted;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
        }

        tree.erase_all();

        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            try{
                tree.at(array[i]);
                return failed;
            }
            catch(const std::out_of_range& e){
                ;
            }
        }

        return passed;
    });

    tester("9. emplace", [&](){
        std::ranges::shuffle(array.begin(), array.end(),rm);
        csaur::BPlusTree<int,int,3> tree;
        for (int i = 0; i < BPlusTest::num_inserted; ++i){
            tree.emplace<int>(int(array[i]),int(array[i]));
            for (int j = 0; j <= i; ++j){
                _ASSERT(tree.at(array[j]) == array[j]);
            }
            for (int j = i + 1;j < BPlusTest::num_inserted;++j){
                try{
                    tree.at(array[j]);
                    return failed;
                }
                catch(const std::out_of_range& e){
                    ;
                }
            }
        }

        tree.erase_all();
        return passed;
    });

    tester("10. value bad_alloc", [&](){
        csaur::BPlusTree<int,int,3, FourMaxAllocator<int>> tree;
        tree.insert(0,0);
        tree.insert(1,1);
        tree.insert(2,2);
        tree.insert(3,3);
        tree.insert(4,4);

        _ASSERT(tree.contains(0));
        _ASSERT(tree.contains(1));
        _ASSERT(tree.contains(2));
        _ASSERT(tree.contains(3));
        _ASSERT(!tree.contains(4));

        tree.erase_all();
        return passed;
    });

    tester("11. node bad_alloc", [&](){
        csaur::BPlusTree<int,int,3, csaur::DefaultAllocator<int>, FourMaxAllocator<csaur::BPlusNode<int,int,3>>> tree;
        tree.insert(0,0);
        tree.insert(1,1);
        tree.insert(2,2);
        tree.insert(3,3);
        tree.insert(4,4);
        tree.insert(5,5);
        tree.insert(6,6);
        tree.insert(7,7);

        _ASSERT(tree.contains(0));
        _ASSERT(tree.contains(1));
        _ASSERT(tree.contains(2));
        _ASSERT(tree.contains(3));
        _ASSERT(tree.contains(4));
        _ASSERT(tree.contains(5));
        _ASSERT(tree.contains(6));
        _ASSERT(!tree.contains(7));

        tree.erase_all();
        return passed;
    });

    tester("12. double insertions", [&](){
        csaur::BPlusTree<int,int,3> tree;
        tree.emplace(4,4);
        tree.try_emplace(5,5);
        tree.try_emplace(4,5);
        tree.emplace(5,4);

        _ASSERT(tree.at(4) == 5);
        _ASSERT(tree.at(5) == 5);

        tree.erase_all();
        return passed;
    });
}
