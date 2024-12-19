#ifndef BPLUS_NODE_CLASS_DEFINED
#define BPLUS_NODE_CLASS_DEFINED

#include "Constraints.hpp"
#include <stdexcept>
#include <array>
#include <variant>

template<std::size_t N, OrderedKey Key, Storable Mapped>
class BPlusNode{
public:
    std::size_t m_key_counter;
    std::array<Key, N> m_keys;
    std::variant<std::array<Mapped*, N>, std::array<BPlusNode*, N>> m_data; // either Mapped* in leafs or BPlusNode* in internal nodes
    BPlusNode* m_next, * m_prev;
public:

    /// @brief Creates an empty node
    /// @param is_leaf A flag for creating either a leaf (if true) or an internal node (if false)
    BPlusNode(bool);

    /// @brief Searches for the data associated with a given key in the node's subtree.
    /// @param key The key to search for.
    /// @return A pointer to the mapped data associated with the key or nullptr if there isn't one.
    Mapped* search (const Key&) const;

    /// @brief Inserts a new key and its associated data into the node's subtree.
    /// @param key The key to insert.
    /// @param data The data associated with the key.
    /// @return A pointer to the newly created node if the node was full and split. Otherwise, returns `nullptr` if the insertion is successful and the node is not full.
    BPlusNode* insert (const Key&, const Mapped&);

    /// @brief Deletes the data associated with the given key from the node. 
    /// @param key The key to erase.
    /// @return `true` if the deletion was successful, `false` otherwise.
    bool erase(const Key&);

    /// @brief Removes all data if present.
    void erase_all();

    /// @brief Finds the index of the smallest key in the node that is strictly greater than the given key.
    /// @param key The key to find the smallest bigger key for.
    /// @return The index of the smallest key larger than the given `key` in the `m_keys` array.
    /// @note Time Complexity: O(log x), where x is the number of keys currently stored in the node.
    /// @note Space Complexity: O(1).
    std::size_t find_smallest_bigger_key_index(const Key& key) const;

    /// @brief Handles underflows that occur after deletion in an internal node. This method is responsible for
    ///        deciding whether to merge nodes or redistribute keys between siblings, and it propagates the underflow
    ///        signal to the parent if necessary.
    /// @param underflow_index The index of the underflowing subnode.
    template<bool is_internal>
    void handle_underflow(std::size_t underflow_index);
};

//////////////////////////////////////////////////////////////////////////////////
//                           Auxiliary Function Block                           //
//////////////////////////////////////////////////////////////////////////////////

/// @brief Gets two adjacent node pointers and merges them into the lower one. Assumes the input nodes are adjacent.
/// @param lower_node A pointer to a node
/// @param higher_node A pointer to a node
template <std::size_t N, OrderedKey Key, Storable Mapped, bool is_internal>
void merge(BPlusNode<N, Key, Mapped>* lower_node, BPlusNode<N, Key, Mapped>* higher_node){
    for (std::size_t i = 0; i < higher_node->m_key_counter; ++i) {
        lower_node->m_keys[lower_node->m_key_counter + i] = higher_node->m_keys[i];
        std::get<is_internal>(lower_node->m_data)[lower_node->m_key_counter + i] = std::get<is_internal>(higher_node->m_data)[i];
    }

    lower_node->m_key_counter += higher_node->m_key_counter + 1;

    if (higher_node->m_next){
        higher_node->m_next->m_prev = lower_node;
    }
    lower_node->m_next = higher_node->m_next;

    delete higher_node;
}

//////////////////////////////////////////////////////////////////////////////////
//                        BPlusNode Implementation Block                        //
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, OrderedKey Key, Storable Mapped>
BPlusNode<N, Key, Mapped>::BPlusNode(bool is_leaf)
{
    m_key_counter = 0;
    m_next = nullptr;
    m_prev = nullptr;
    if (is_leaf){
        m_data = std::array<Mapped*, N> ();
    }
    else{
        m_data = std::array<BPlusNode*, N> ();
    }
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
Mapped *BPlusNode<N, Key, Mapped>::search(const Key &key) const
{
    auto index = find_smallest_bigger_key_index(key);
    if (index == 0) return nullptr;
    else --index;
    if (m_data.index() == 0){
        // in leafs, d[i] corresponds to k[i]; in internal nodes, d[i] contains search keys which are all less than k[i]
        if (this->m_keys[index] != key) return nullptr; 
        return std::get<0>(m_data)[index];
    }
    return std::get<1>(m_data)[index]->search(key);
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
BPlusNode<N, Key, Mapped> *BPlusNode<N, Key, Mapped>::insert(const Key& key, const Mapped& mapped)
{
    std::variant<Mapped*, BPlusNode*> inserted_ptr;
    Key inserted_key;
    if (m_data.index() == 0){                           // if leaf
        if (search(key) != nullptr) return nullptr;
        inserted_ptr = new Mapped (mapped);
        inserted_key = Key (key);
    }
    else{                                               // if internal
        std::size_t working_index = this->find_smallest_bigger_key_index(key);
        working_index -= (working_index != 0);
        inserted_ptr = std::get<1>(this->m_data)[working_index]->insert(key, mapped);
        this->m_keys[working_index] = std::get<1>(this->m_data)[working_index]->m_keys[0];
        if (std::get<1>(inserted_ptr) == nullptr) return nullptr;
        inserted_key = std::get<1>(inserted_ptr)->m_keys[0];
    }

    BPlusNode* inserting_node = this, * out_ptr = nullptr;
    if (this->m_key_counter == N){ // split
        auto new_node = new BPlusNode(this->m_data.index() == 0);              // index == 0 <-> this is a leaf
        std::size_t mid = (inserted_key < this->m_keys[N/2]) ? N/2 : N - N/2;  // stabilizing the sizes
        inserting_node = (inserted_key < this->m_keys[N/2]) ? this : new_node; // choose the smaller node
        out_ptr = new_node;

        if (this->m_data.index() == 0){
            for (std::size_t i = mid; i < N; ++i) {
                std::get<0>(new_node->m_data)[i - mid] = std::get<0>(this->m_data)[i];
                new_node->m_keys[i - mid] = this->m_keys[i];
            }
        }
        else{
            for (std::size_t i = mid; i < N; ++i) {
                std::get<1>(new_node->m_data)[i - mid] = std::get<1>(this->m_data)[i];
                new_node->m_keys[i - mid] = this->m_keys[i];
            }
        }
        this->m_key_counter = mid;
        new_node->m_key_counter = N - mid;

        if (this->m_next){
           this->m_next->m_prev = new_node;
        }
        new_node->m_next = this->m_next;
        this->m_next = new_node;
        new_node->m_prev = this;
    }
    std::size_t i = inserting_node->m_key_counter;
    if (this->m_data.index() == 0){
        for (; (i > 0)&&(inserted_key < inserting_node->m_keys[i-1]); --i){ // move them all one step right to fit the new one
            std::get<0>(inserting_node->m_data)[i] = std::get<0>(inserting_node->m_data)[i-1];
        }
        std::get<0>(inserting_node->m_data)[i] = std::get<0>(inserted_ptr);
    }
    else{
        for (; (i > 0)&&(inserted_key < inserting_node->m_keys[i-1]); --i){ // move them all one step right to fit the new one
            std::get<1>(inserting_node->m_data)[i] = std::get<1>(inserting_node->m_data)[i-1];
        }
        std::get<1>(inserting_node->m_data)[i] = std::get<1>(inserted_ptr);
    }
    for (i = inserting_node->m_key_counter; (i > 0)&&(inserted_key < inserting_node->m_keys[i-1]); --i){ // move them all one step right to fit the new one
        inserting_node->m_keys[i] = inserting_node->m_keys[i-1];
    }
    inserting_node->m_keys[i] = inserted_key;
    ++inserting_node->m_key_counter;
    return out_ptr;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
void BPlusNode<N, Key, Mapped>::erase_all()
{
    if (this->m_data.index() == 0){
        for (std::size_t i = 0; i < this->m_key_counter; ++i){
            delete std::get<0>(this->m_data)[i];
        }
    }
    else{
        for (std::size_t i = 0; i < this->m_key_counter; ++i){
            std::get<1>(this->m_data)[i]->erase_all();
            delete std::get<1>(this->m_data)[i];
        }
    }
    this->m_key_counter = 0;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
bool BPlusNode<N, Key, Mapped>::erase(const Key& key)
{
    std::size_t i = this->find_smallest_bigger_key_index(key);
    if (i == 0) return false;
    if (this->m_data.index() != 0){
        --i;
        auto res = std::get<1>(this->m_data)[i]->erase(key);
        if (std::get<1>(this->m_data)[i]->m_key_counter >= N-N/2) return res; // if there's no underflow, return
        if (std::get<1>(this->m_data)[i]->m_data.index() == 0){ // else, fix the situation and then return
            handle_underflow<false>(i);
        }
        else{
            handle_underflow<true>(i);
        }
        return res;
    }
    if (this->m_keys[i - 1] != key)
        return false;

    delete std::get<0>(this->m_data)[i-1];
    
    for (; i < this->m_key_counter; ++i){ // move them all one step left
        std::get<0>(this->m_data)[i-1] = std::get<0>(this->m_data)[i];
        this->m_keys[i-1] = this->m_keys[i];
    }
    --this->m_key_counter;
    return true;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
std::size_t BPlusNode<N, Key, Mapped>::find_smallest_bigger_key_index(const Key &key) const
{
    std::size_t left = 0, right = (m_key_counter != 0) ? m_key_counter - 1 : 0, result = m_key_counter;
    // binary search, T(n) = O(log n) & S(n) = O(1)
    while (left <= right){
        auto mid = (left + right) / 2;
        if (key < m_keys[mid]){ // comparison is permitted because Key must support 'operator<(Key, Key)'
            result = mid;
            if (mid == 0) break; // prevents out-of-range indexing
            right = mid - 1;
        }
        else{
            left = mid + 1; 
        }
    }
    return result;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
template<bool is_internal>
void BPlusNode<N, Key, Mapped>::handle_underflow(std::size_t underflow_index)
{
    // assumes having at least two sons, for the case for less would have already been dealt with
    std::size_t neighbour_index = (underflow_index > 0) ? underflow_index - 1 : underflow_index + 1;
    BPlusNode<N, Key, Mapped>* underflow_subnode = std::get<1>(this->m_data)[underflow_index], * neighbour_subnode = std::get<1>(this->m_data)[neighbour_index];
    if (underflow_subnode->m_key_counter + neighbour_subnode->m_key_counter > N){
        // Case 1: borrow
        if (underflow_index > neighbour_index){
            // first move all things here rightwards
            for (std::size_t i = underflow_subnode->m_key_counter; i > 0; --i) {
                underflow_subnode->m_keys[i] = underflow_subnode->m_keys[i - 1];
                std::get<is_internal>(underflow_subnode->m_data)[i] = std::get<is_internal>(underflow_subnode->m_data)[i - 1];
            }
            // then borrow at the start
            underflow_subnode->m_keys[0] = neighbour_subnode->m_keys[neighbour_subnode->m_key_counter - 1];
            std::get<is_internal>(underflow_subnode->m_data)[0] = std::get<is_internal>(neighbour_subnode->m_data)[neighbour_subnode->m_key_counter - 1];

            --neighbour_subnode->m_key_counter;
            ++underflow_subnode->m_key_counter;
            // then fix extremes and this' keys
            this->m_keys[underflow_index] = underflow_subnode->m_keys[0];
        }
        else{
            // first borrow at the end
            underflow_subnode->m_keys[underflow_subnode->m_key_counter] = neighbour_subnode->m_keys[0];
            std::get<is_internal>(underflow_subnode->m_data)[underflow_subnode->m_key_counter] = std::get<is_internal>(neighbour_subnode->m_data)[0];

            --neighbour_subnode->m_key_counter;
            ++underflow_subnode->m_key_counter;
            // then move all the neighbour's leftwards
            for (std::size_t i = 0; i < neighbour_subnode->m_key_counter; ++i) {
                neighbour_subnode->m_keys[i] = neighbour_subnode->m_keys[i + 1];
                std::get<is_internal>(neighbour_subnode->m_data)[i] = std::get<is_internal>(neighbour_subnode->m_data)[i + 1];
            }
            // then fix extremes and this' keys
            this->m_keys[neighbour_index] = neighbour_subnode->m_keys[0];
        }
    }
    else{
        // Case 2: merge
        if (underflow_index > neighbour_index) { 
            merge<N, Key, Mapped, is_internal>(neighbour_subnode, underflow_subnode);
        }
        else { 
            merge<N, Key, Mapped, is_internal>(underflow_subnode, neighbour_subnode);
        }

        this->m_key_counter--;
        for (std::size_t i = underflow_index + 1; i < this->m_key_counter; ++i) {
            this->m_keys[i] = this->m_keys[i + 1];
            std::get<1>(this->m_data)[i] = std::get<1>(this->m_data)[i + 12];
        }
    }
}

#endif
