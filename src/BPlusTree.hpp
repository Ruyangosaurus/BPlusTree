#ifndef BPLUS_TREE_CLASS_DEFINED
#define BPLUS_TREE_CLASS_DEFINED

#include "Constraints.hpp"
#include <stdexcept>
#include <array>
#include <variant>
#include <string>
#include <algorithm>
#include "BPlusNode.hpp"
#include "DefaultAllocator.hpp"
namespace csaur{
    template<OrderedKey Key, 
             Storable Mapped, 
             std::size_t N, 
             SingleElementAllocator ValueAlloc = DefaultAllocator<Mapped>,
             SingleElementAllocator NodeAlloc = DefaultAllocator<BPlusNode<Key, Mapped, N>>
            >
    class BPlusTree{
    public:
        using key_type = Key;
        using mapped_type = Mapped;
        using value_type = mapped_type;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using size_type = std::size_t;
        using value_allocator = ValueAlloc;
        using node_type = BPlusNode<Key, Mapped, N>;
        using node_allocator = NodeAlloc;
    private:

        /// @brief Gets two adjacent node pointers and merges them into the lower one. Assumes the input nodes are adjacent.
        /// @param lower_node A pointer to a node
        /// @param higher_node A pointer to a node
        template <bool is_internal>
        void merge(node_type* lower_node, node_type* higher_node);

        /// @brief Allocates a node and copies the contents of another into it.
        /// @param source A pointer to the node to be copied.
        /// @return A new allocated node copy.
        node_type* copy (const node_type* source, const node_type* prev, bool has_min, bool has_max);

        friend node_type;
        #ifdef DEBUGGING_B_PLUS_TREE
            friend class ::BPlusTest;
        #endif

    private:
        size_type m_size; 
        node_type* m_root,* m_min,* m_max;
        NodeAlloc m_node_alloc;
        value_allocator m_val_alloc;
    public:
        /// @brief Creates an empty BPlusTree.
        BPlusTree();

        /// @brief Copies a BPlusTree.
        /// @param other Another BPlusTree
        BPlusTree(const BPlusTree& other);

        /// @brief Moves a BPlusTree.
        /// @param other Another BPlusTree
        BPlusTree(BPlusTree&& other);

        /// @brief Destructs a BPlusTree
        ~BPlusTree();

        /// @brief Copies a BPlusTree.
        /// @param other Another BPlusTree
        BPlusTree& operator= (const BPlusTree& other);

        /// @brief Moves a BPlusTree.
        /// @param other Another BPlusTree
        BPlusTree& operator= (BPlusTree&& other);

        /// @brief Returns the number of keys and mapped values in the tree.
        size_type size() const;

        /// @brief Checks if the tree is empty
        bool is_empty() const;

        /// @brief Inserts a new key and its associated data into the tree.
        /// @param key The key to insert.
        /// @param data The data associated with the key.
        /// @note There can only be one associated value per key.
        void insert (const key_type&, const mapped_type&);
        void insert (key_type&&, mapped_type&&) noexcept;

        /// @brief Inserts a new key and its associated data into the tree.
        /// @param key The key to insert.
        /// @param args Arguments used to construct the data associated with the key.
        /// @note There can only be one associated value per key.
        /// @note At failure, does not change contents.
        template<typename ... Args> requires std::constructible_from<Mapped, Args...>
        void emplace (key_type&&, Args&& ...) noexcept;

        /// @brief Inserts a new key and its associated data into the tree.
        /// @param key The key to insert.
        /// @param args Arguments used to construct the data associated with the key.
        /// @note Replaces value if key exists.
        /// @note At failure, does not change contents.
        template<typename ... Args> requires std::constructible_from<Mapped, Args...>
        void try_emplace (key_type&&, Args&& ...) noexcept;

        /// @brief Erases a key and its associated data from the tree if found.
        /// @param key The key to erase.
        /// @return Whether or not the key was found.
        bool erase (const key_type&);

        /// @brief Removes all data if present.
        void erase_all();

        /// @brief Returns the minimal key in the tree
        /// @exception std::out_of_range if the container is empty. 
        const key_type& min_key() const;
        /// @brief Returns the maximal key in the tree
        /// @exception std::out_of_range if the container is empty. 
        const key_type& max_key() const;

        /// @brief Returns a reference to the mapped value associated with the key.
        /// @param key A key to compare to.
        /// @return A reference to the mapped value associated with the key.
        /// @exception std::out_of_range if the container does not have an element with the specified key. 
        mapped_type& at(const key_type& key);
        const mapped_type& at(const key_type& key) const;

        /// @brief Checks if there is a value mapped to the key in this tree.
        /// @param key A key to compare to.
        /// @return Whether there is a value mapped to the key in this tree.
        bool contains(const key_type& key) const;
    };

    //////////////////////////////////////////////////////////////////////////////////
    //                      BPlusTree CTOR, DTOR, & =TOR Block                      //
    //////////////////////////////////////////////////////////////////////////////////

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::BPlusTree()
    {
        m_size = 0;
        m_root = std::construct_at(m_node_alloc.allocate(), true);
        m_min = m_root;
        m_max = m_root;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::BPlusTree(const BPlusTree &other)
    {
        m_size = other.m_size;
        m_root = copy(other.m_root, nullptr, true, true);
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::BPlusTree(BPlusTree &&other)
    {
        m_size = other.m_size;
        m_root = other.m_root;
        m_min = other.m_min;
        m_max = other.m_max;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::~BPlusTree()
    {
        m_root->erase_all(*this);
        std::destroy_at(m_root);
        m_node_alloc.deallocate(m_root);
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>& BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::operator=(const BPlusTree &other)
    {
        m_root->erase_all(*this);
        std::destroy_at(m_root);
        m_node_alloc.deallocate(m_root);
        m_size = other.m_size;
        m_root = copy(other.m_root, nullptr, true, true);
        return *this;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>& BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::operator=(BPlusTree &&other)
    {
        m_root->erase_all(*this);
        std::destroy_at(m_root);
        m_node_alloc.deallocate(m_root);
        m_size = other.m_size;
        m_root = other.m_root;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::size_type BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::size() const
    {
        return m_size;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    bool BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::is_empty() const
    {
        return m_size == 0;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    const BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::key_type &BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::min_key() const
    {
        if (is_empty()){
            throw std::out_of_range("BPlusTree::min_key: An empty tree has no minimal key.\n");
        }
        return m_min->m_keys[0];
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    const BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::key_type &BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::max_key() const
    {
        if (is_empty()){
            throw std::out_of_range("BPlusTree::min_key: An empty tree has no minimal key.\n");
        }
        return m_max->m_keys[m_max->m_key_counter - 1];
    }

    //////////////////////////////////////////////////////////////////////////////////
    //                           BPlusTree CRUD API Block                           //
    //////////////////////////////////////////////////////////////////////////////////

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    void BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::insert(const key_type& key, const mapped_type& mapped)
    {
        emplace(key_type(key), key_type(mapped));
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    void BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::insert(key_type&& key, mapped_type&& mapped) noexcept
    {
        emplace(std::move(key), std::move(mapped));
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc> 
    template <typename ... Args> requires std::constructible_from<Mapped, Args...>
    void BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::emplace(key_type && key, Args &&... args) noexcept
    {
        std::variant<pointer, node_type*> new_node = m_root->template emplace<ValueAlloc, NodeAlloc, false, Args...>(*this, std::move(key), std::move(args...));
        if (std::get<1>(new_node)){
            node_type* new_root;
            try{
                new_root = std::construct_at(m_node_alloc.allocate(), false);
            }
            catch(std::bad_alloc& e){
                while(new_node.index() != 0){
                    auto temp = std::get<1>(new_node);
                    if (temp->m_data.index() == 0){
                        new_node = std::get<1>(temp->m_data)[0];
                    }
                    else{
                        new_node = std::get<0>(temp->m_data)[0];
                    }
                    std::destroy_at(temp);
                    m_node_alloc.deallocate(temp);
                }
                std::destroy_at(std::get<0>(new_node));
                m_val_alloc.deallocate(std::get<0>(new_node));
                return;
            }
            node_type::restabilize(m_root, std::get<1>(new_node));
            new_root->m_key_counter = 2;
            new_root->m_keys[0] = m_root->m_keys[0];
            new_root->m_keys[1] = std::get<1>(new_node)->m_keys[0];
            std::get<1>(new_root->m_data)[0] = m_root;
            std::get<1>(new_root->m_data)[1] = std::get<1>(new_node);
            m_root = new_root;
        }
        ++m_size;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc> 
    template <typename ... Args> requires std::constructible_from<Mapped, Args...>
    void BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::try_emplace(key_type && key, Args &&... args) noexcept
    {
        std::variant<pointer, node_type*> new_node = m_root->template emplace<ValueAlloc, NodeAlloc, true, Args...>(*this, std::move(key), std::move(args...));
        if (std::get<1>(new_node)){
            node_type* new_root;
            try{
                new_root = std::construct_at(m_node_alloc.allocate(), false);
            }
            catch(std::bad_alloc& e){
                while(new_node.index() != 0){
                    auto temp = std::get<1>(new_node);
                    if (temp->m_data.index() == 0){
                        new_node = std::get<1>(temp->m_data)[0];
                    }
                    else{
                        new_node = std::get<0>(temp->m_data)[0];
                    }
                    std::destroy_at(temp);
                    m_node_alloc.deallocate(temp);
                }
                std::destroy_at(std::get<0>(new_node));
                m_val_alloc.deallocate(std::get<0>(new_node));
                return;
            }
            node_type::restabilize(m_root, std::get<1>(new_node));
            new_root->m_key_counter = 2;
            new_root->m_keys[0] = m_root->m_keys[0];
            new_root->m_keys[1] = std::get<1>(new_node)->m_keys[0];
            std::get<1>(new_root->m_data)[0] = m_root;
            std::get<1>(new_root->m_data)[1] = std::get<1>(new_node);
            m_root = new_root;
        }
        ++m_size;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    bool BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::erase(const key_type &key)
    {
        bool out = m_root->erase(*this, key);
        if (m_root->m_key_counter == 1 && m_root->m_data.index() == 1){
            auto temp = std::get<1>(m_root->m_data)[0];
            std::get<1>(m_root->m_data)[0] = nullptr;
            std::destroy_at(m_root);
            m_node_alloc.deallocate(m_root);
            m_root = temp;
        }
        --m_size;
        return out;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    void BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::erase_all()
    {
        m_root->erase_all(*this);
        m_size = 0;
        m_root->m_data = std::array<pointer, N> ();
        m_min = m_root;
        m_max = m_root;
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::mapped_type& BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::at(const key_type &key)
    {
        pointer search_result = m_root->search(key);
        if (search_result){
            return *search_result;
        }
        throw std::out_of_range("BPlusTree::at: No value associated with the key was found.\n");
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    const BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::mapped_type& BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::at(const key_type &key) const
    {
        pointer search_result = m_root->search(key);
        if (search_result){
            return *search_result;
        }
        throw std::out_of_range("BPlusTree::at: No value associated with the key was found.\n");
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    bool BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::contains(const key_type &key) const
    {
        return nullptr != m_root->search(key);
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    template <bool is_internal>
    void BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::merge(typename BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::node_type* lower_node, typename BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::node_type* higher_node){
        for (std::size_t i = 0; i < higher_node->m_key_counter; ++i) {
            lower_node->m_keys[lower_node->m_key_counter + i] = higher_node->m_keys[i];
            std::get<is_internal>(lower_node->m_data)[lower_node->m_key_counter + i] = std::get<is_internal>(higher_node->m_data)[i];
        }

        lower_node->m_key_counter += higher_node->m_key_counter;

        if (higher_node->m_next){
            higher_node->m_next->m_prev = lower_node;
        }
        lower_node->m_next = higher_node->m_next;

        std::destroy_at(higher_node);
        m_node_alloc.deallocate(higher_node);
    }

    template <OrderedKey Key, Storable Mapped, std::size_t N, SingleElementAllocator ValueAlloc, SingleElementAllocator NodeAlloc>
    typename BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::node_type *BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::copy(const typename BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::node_type *source, const typename BPlusTree<Key, Mapped, N, ValueAlloc, NodeAlloc>::node_type *prev, bool has_min, bool has_max)
    {
        node_type* out = std::construct_at(m_node_alloc.allocate(), source->m_data.index());
        out->m_prev = prev;
        if (prev){
            prev->m_next = out;
        }
        out->m_key_counter = source->m_key_counter;
        for (size_type i = 0; i < out->m_key_counter; ++i){
            out->m_keys[i] = source->m_keys[i];
        }
        if (source->m_data.index() == 0){
            for (size_type i = 0; i < out->m_key_counter; ++i){
                std::get<0>(out->m_data)[i] = new value_type (std::get<0>(source->m_data)[i]);
            }
            if (has_min){
                m_min = out;
            }
            if (has_max){
                m_max = out;
            }
        }
        else{
            size_type i = 0;
            std::get<1>(out->m_data)[i] = copy (std::get<1>(source->m_data)[i], nullptr, has_min, false);
            for (++i; i < out->m_key_counter - 1; ++i){
                std::get<1>(out->m_data)[i] = copy (std::get<1>(source->m_data)[i], std::get<1>(out->m_data)[i - 1], false, false);
            }
            std::get<1>(out->m_data)[i] = copy (std::get<1>(source->m_data)[i], std::get<1>(out->m_data)[i - 1], false, has_max);
        }
        return out;
    }
};
#endif
