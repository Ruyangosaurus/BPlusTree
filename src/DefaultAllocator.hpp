#ifndef DEFAULT_ALLOCATOR_CLASS_DEFINED
#define DEFAULT_ALLOCATOR_CLASS_DEFINED
#include <cstdlib>
namespace csaur{
    template<class T>
    class DefaultAllocator{
    public:
        using value_type = T;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;

        DefaultAllocator(){}
        DefaultAllocator(const DefaultAllocator&) = delete;
        DefaultAllocator& operator=(const DefaultAllocator&) = delete;

        pointer allocate();
        pointer allocate(size_t);
        void deallocate(pointer);
    };

    template <class T>
    typename DefaultAllocator<T>::pointer DefaultAllocator<T>::allocate()
    {
        return (DefaultAllocator<T>::pointer)std::malloc(sizeof(value_type));
    }

    template <class T>
    typename DefaultAllocator<T>::pointer DefaultAllocator<T>::allocate(size_t n)
    {
        return (DefaultAllocator<T>::pointer)std::malloc(n*sizeof(value_type));
    }

    template <class T>
    void DefaultAllocator<T>::deallocate(pointer node)
    {
        std::free(node);
    }
};
#endif