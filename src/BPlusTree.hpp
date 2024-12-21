#ifndef BPLUS_TREE_CLASS_DEFINED
#define BPLUS_TREE_CLASS_DEFINED

#include "Constraints.hpp"
#include <stdexcept>
#include <array>
#include <variant>
#include <string>
#include <algorithm>

template<OrderedKey Key, Storable Mapped, std::size_t N>
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

private:
    class BPlusNode{
    public:
        std::size_t m_key_counter;
        std::array<key_type, N> m_keys;
        std::variant<std::array<pointer, N>, std::array<BPlusNode*, N>> m_data; // either Mapped* in leafs or BPlusNode* in internal nodes
        BPlusNode* m_next, * m_prev;
    public:

        /// @brief Creates an empty node
        /// @param is_leaf A flag for creating either a leaf (if true) or an internal node (if false)
        BPlusNode(bool);

        /// @brief Searches for the data associated with a given key in the node's subtree.
        /// @param key The key to search for.
        /// @return A pointer to the mapped data associated with the key or nullptr if there isn't one.
        pointer search (const key_type&) const;

        /// @brief Inserts a new key and its associated data into the node's subtree.
        /// @param key The key to insert.
        /// @param data The data associated with the key.
        /// @return A pointer to the newly created node if the node was full and split. Otherwise, returns `nullptr` if the insertion is successful and the node is not full.
        BPlusNode* insert (key_type&&, mapped_type&&);

        /// @brief Deletes the data associated with the given key from the node. 
        /// @param key The key to erase.
        /// @return `true` if the deletion was successful, `false` otherwise.
        bool erase(const key_type&);

        /// @brief Removes all data if present.
        void erase_all();

        /// @brief Finds the index of the smallest key in the node that is strictly greater than the given key.
        /// @param key The key to find the smallest bigger key for.
        /// @return The index of the smallest key larger than the given `key` in the `m_keys` array.
        /// @note Time Complexity: O(log x), where x is the number of keys currently stored in the node.
        /// @note Space Complexity: O(1).
        size_type find_smallest_bigger_key_index(const key_type& key) const;

        /// @brief Handles underflows that occur after deletion in an internal node. This method is responsible for
        ///        deciding whether to merge nodes or redistribute keys between siblings, and it propagates the underflow
        ///        signal to the parent if necessary.
        /// @param underflow_index The index of the underflowing subnode.
        template<bool is_internal>
        void handle_underflow(size_type underflow_index);
    };

    /// @brief Gets two adjacent node pointers and merges them into the lower one. Assumes the input nodes are adjacent.
    /// @param lower_node A pointer to a node
    /// @param higher_node A pointer to a node
    template <bool is_internal>
    static void merge(BPlusNode* lower_node, BPlusNode* higher_node);

    /// @brief Allocates a node and copies the contents of another into it.
    /// @param source A pointer to the node to be copied.
    /// @return A new allocated node copy.
    BPlusNode* copy (const BPlusNode* source, const BPlusNode* prev, bool has_min, bool has_max);

    #ifdef DEBUGGING_B_PLUS_TREE
        friend class BPlusTest;
    #endif

private:
    size_type m_size; 
    BPlusNode* m_root,* m_min,* m_max;
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
    void insert (key_type&&, mapped_type&&);

    /// @brief Erases a key and its associated data from the tree if found.
    /// @param key The key to erase.
    /// @return Whether or not the key was found.
    bool erase (const key_type&);

    /// @brief Removes all data if present.
    void erase_all();

    /// @brief Returns a reference to the mapped value associated with the key.
    /// @param key A key to compare to.
    /// @return A reference to the mapped value associated with the key.
    /// @exception std::out_of_range if the container does not have an element with the specified key. 
    mapped_type& at(const key_type& key);
    const mapped_type& at(const key_type& key) const;
};

#include "BPlusNode.tpp"

//////////////////////////////////////////////////////////////////////////////////
//                      BPlusTree CTOR, DTOR, & =TOR Block                      //
//////////////////////////////////////////////////////////////////////////////////

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::BPlusTree()
{
    m_size = 0;
    m_root = new BPlusNode(true);
    m_min = m_root;
    m_max = m_root;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::BPlusTree(const BPlusTree &other)
{
    m_size = other.m_size;
    m_root = copy(other.m_root, nullptr, true, true);
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::BPlusTree(BPlusTree &&other)
{
    m_size = other.m_size;
    m_root = other.m_root;
    m_min = other.m_min;
    m_max = other.m_max;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::~BPlusTree()
{
    m_root->erase_all();
    delete m_root;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>& BPlusTree<Key, Mapped, N>::operator=(const BPlusTree &other)
{
    m_root->erase_all();
    delete m_root;
    m_size = other.m_size;
    m_root = copy(other.m_root, nullptr, true, true);
    return *this;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>& BPlusTree<Key, Mapped, N>::operator=(BPlusTree &&other)
{
    m_root->erase_all();
    delete m_root;
    m_size = other.m_size;
    m_root = other.m_root;
    m_min = other.m_min;
    m_max = other.m_max;
    return *this;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::size_type BPlusTree<Key, Mapped, N>::size() const
{
    return m_size;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
bool BPlusTree<Key, Mapped, N>::is_empty() const
{
    return m_size == 0;
}

//////////////////////////////////////////////////////////////////////////////////
//                           BPlusTree CRUD API Block                           //
//////////////////////////////////////////////////////////////////////////////////

template <OrderedKey Key, Storable Mapped, std::size_t N>
void BPlusTree<Key, Mapped, N>::insert(const key_type& key, const mapped_type& mapped)
{
    insert(key_type(key), mapped_type(mapped));
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
void BPlusTree<Key, Mapped, N>::insert(key_type&& key, mapped_type&& mapped)
{
    auto new_node = m_root->insert(std::move(key), std::move(mapped));
    if (new_node){
        BPlusNode* new_root = new BPlusNode (false);
        new_root->m_key_counter = 2;
        new_root->m_keys[0] = m_root->m_keys[0];
        new_root->m_keys[1] = new_node->m_keys[0];
        std::get<1>(new_root->m_data)[0] = m_root;
        std::get<1>(new_root->m_data)[1] = new_node;
        m_root = new_root;
    }
    ++m_size;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
bool BPlusTree<Key, Mapped, N>::erase(const key_type& key)
{
    bool out = m_root->erase(key);
    if (m_root->m_key_counter == 1 && m_root->m_data.index() == 1){
        auto temp = std::get<1>(m_root->m_data)[0];
        std::get<1>(m_root->m_data)[0] = nullptr;
        delete m_root;
        m_root = temp;
    }
    --m_size;
    return out;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
void BPlusTree<Key, Mapped, N>::erase_all()
{
    m_root->erase_all();
    m_size = 0;
    m_root->m_data = std::array<pointer, N> ();
    m_min = m_root;
    m_max = m_root;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::mapped_type& BPlusTree<Key, Mapped, N>::at(const key_type &key)
{
    pointer search_result = m_root->search(key);
    if (search_result){
        return *search_result;
    }
    throw std::out_of_range("BPlusTree::at: No value associated with the key was found.\n");
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
const BPlusTree<Key, Mapped, N>::mapped_type& BPlusTree<Key, Mapped, N>::at(const key_type &key) const
{
    pointer search_result = m_root->search(key);
    if (search_result){
        return *search_result;
    }
    throw std::out_of_range("BPlusTree::at: No value associated with the key was found.\n");
}

#endif
