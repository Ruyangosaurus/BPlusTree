#ifndef BANK_ALLOCATOR_CLASS_DEFINED
#define BANK_ALLOCATOR_CLASS_DEFINED

#include <cstring>
#include <bitset>
namespace csaur{
    template<class T, size_t M>
    class BankAllocator{
        struct AllocatedBlock;

        struct AllocatedObject{
            T m_node;
            size_t m_num;
        };

        struct AllocatedBlock{
            AllocatedObject m_objects [M];
            std::bitset<M> m_flags;
            AllocatedBlock* m_next;
            AllocatedBlock* m_prev;
        };

        AllocatedBlock* m_root;

    public:
        using value_type = T;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;

        BankAllocator();
        ~BankAllocator();
        BankAllocator(const BankAllocator&) = delete;
        BankAllocator& operator=(const BankAllocator&) = delete;

        pointer allocate();
        void deallocate(pointer);
    };

    template <class T, size_t M>
    BankAllocator<T, M>::BankAllocator()
    {
        m_root = new AllocatedBlock();
        for (size_t i = 0; i < M; ++i){
            m_root->m_objects[i].m_num = i;
        }
        m_root->m_next = m_root;
        m_root->m_prev = m_root;
    }

    template <class T, size_t M>
    BankAllocator<T, M>::~BankAllocator()
    {
        while (m_root->m_next != m_root){
            AllocatedBlock* cur = m_root->m_next;
            cur->m_prev->m_next = cur->m_next;
            cur->m_next->m_prev = cur->m_prev;
            delete cur;
        }
        delete m_root;
    }

    template <class T, size_t M>
    typename BankAllocator<T, M>::pointer BankAllocator<T, M>::allocate()
    {
        AllocatedBlock* cur;
        if(!m_root->m_flags.all()){
            cur = m_root;
        }
        else{
            cur = m_root->m_next;
            while(cur->m_flags.all()){
                cur = cur->m_next;
                if(cur == m_root){
                    cur = new AllocatedBlock();
                    for (size_t i = 0; i < M; ++i){
                        cur->m_objects[i].m_num = i;
                    }
                    cur->m_next = m_root->m_next;
                    cur->m_prev = m_root;
                    m_root->m_next->m_prev = cur;
                    m_root->m_next = cur;
                    break;
                }
            }
        }
        unsigned i = 0;
        for(; cur->m_flags.test(i); ++i){}
        cur->m_flags.set(i);
        return (pointer)(&(cur->m_objects[i]));
    }

    template <class T, std::size_t M>
    void BankAllocator<T, M>::deallocate(pointer node)
    {
        if (node == nullptr) return;
        AllocatedObject* obj = (AllocatedObject*)node;
        AllocatedBlock* cur = (AllocatedBlock*)(obj - obj->m_num);
        unsigned i = obj->m_num;
        cur->m_flags.reset(i);
        if (cur->m_flags.none() && cur != m_root){
            cur->m_prev->m_next = cur->m_next;
            cur->m_next->m_prev = cur->m_prev;
            delete cur;
        }
    }
};
#endif