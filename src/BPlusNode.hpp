#ifndef BPLUS_NODE_CLASS_DEFINED
#define BPLUS_NODE_CLASS_DEFINED

#include "Constraints.hpp"
#include <stdexcept>
#include <array>
#include <memory>
#include <variant>

namespace csaur{
    template<OrderedKey, Storable, std::size_t, SingleElementAllocator, SingleElementAllocator>
    class BPlusTree;

    template<OrderedKey Key, Storable Mapped, std::size_t N>
    class BPlusNode{
        template<OrderedKey, Storable, std::size_t, SingleElementAllocator, SingleElementAllocator>
        friend class csaur::BPlusTree;
        #ifdef DEBUGGING_B_PLUS_TREE
            friend class ::BPlusTest;
        #endif
    public:
        using key_type = Key;
        using mapped_type = Mapped;
        using value_type = mapped_type;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using size_type = std::size_t;

        /// @brief Creates an empty node
        /// @param is_leaf A flag for creating either a leaf (if true) or an internal node (if false)
        BPlusNode(bool is_leaf = false);
    private:
        std::size_t m_key_counter;
        std::array<key_type, N> m_keys;
        std::variant<std::array<pointer, N>, std::array<BPlusNode*, N>> m_data; // either Mapped* in leafs or BPlusNode* in internal nodes
        BPlusNode* m_next, * m_prev;

        /// @brief Searches for the data associated with a given key in the node's subtree.
        /// @param key The key to search for.
        /// @return A pointer to the mapped data associated with the key or nullptr if there isn't one.
        pointer search (const key_type&) const;

        /// @brief Inserts a new key and its associated data into the node's subtree.
        /// @param key The key to insert.
        /// @param args Arguments used to construct the data associated with the key.
        /// @return A pointer to the newly created node if the node was full and split. Otherwise, returns `nullptr` if the insertion is successful and the node is not full.
        template<SingleElementAllocator ValAlloc, SingleElementAllocator NodeAlloc, typename ... Args> requires std::constructible_from<Mapped, Args...>
        BPlusNode* emplace (BPlusTree<Key, Mapped, N, ValAlloc, NodeAlloc>&, key_type&&, Args&& ...);

        /// @brief Deletes the data associated with the given key from the node. 
        /// @param key The key to erase.
        /// @return `true` if the deletion was successful, `false` otherwise.
        template<SingleElementAllocator ValAlloc, SingleElementAllocator NodeAlloc>
        bool erase(BPlusTree<Key, Mapped, N, ValAlloc, NodeAlloc>&, const key_type&);

        /// @brief Removes all data if present.
        template<SingleElementAllocator ValAlloc, SingleElementAllocator NodeAlloc>
        void erase_all(BPlusTree<Key, Mapped, N, ValAlloc, NodeAlloc>&);

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
        template<SingleElementAllocator ValAlloc, SingleElementAllocator NodeAlloc, bool is_internal>
        void handle_underflow(BPlusTree<Key, Mapped, N, ValAlloc, NodeAlloc>&, size_type underflow_index);
    };
    #include "BPlusNode.tpp"
};

#endif