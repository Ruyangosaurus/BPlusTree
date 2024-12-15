#ifndef BPLUS_NODE_CLASS_DEFINED
#define BPLUS_NODE_CLASS_DEFINED

#include "Constraints.hpp"
#include <stdexcept>
#include <array>

/// @brief A base class that serves as the foundation for the tree's nodes and for the tree's dummy node.
/// The class defines pointers for the bidirectional linking of nodes, where each node has a `next` and `prev` pointer.
class BidirectionalNode{
protected:
    BidirectionalNode* next, *prev;
    void add_node_after_this(BidirectionalNode* new_node){
        if (this->next){
            this->next->prev = new_node;
            new_node->next = this->next;
        }
    }
    bool is_node_after_me (const BidirectionalNode* const other) const {return (other!=nullptr)&&
                                                                                (other->prev == this)&&
                                                                                (this->next == other);}
public:
    /// @brief Constructor that initializes the `next` and `prev` pointers to `nullptr`.
    BidirectionalNode() : next(nullptr), prev(nullptr){}
};

/// @brief An abstract base class representing a node in a B+ tree.
///        This class defines the shared behavior for both leaf nodes and internal nodes.
///        It provides common methods for key storage, search, and insertion for both node types,
///        but leaves specific implementations (e.g., data retrieval or node insertion) to derived classes.
/// @tparam N The maximum number of elements a node can store. 
/// @tparam Key The type of the key used in the B+ tree nodes. 
/// * must be move-constructible.
/// * must be copy-constructible.
/// * must be destructible.
/// * must supports the comparison operations '<' and '=='.
/// @tparam Mapped The type of the data mapped to each key in the leaf nodes.
/// * must be move-constructible.
/// * must be copy-constructible.
/// * must be destructible.
template<std::size_t N, OrderedKey Key, Storable Mapped>
class BPlusNode : public BidirectionalNode{
protected:
    std::array<Key, N> m_keys;
    std::size_t m_key_counter;
public:
    /// @brief Default constructor. Initializes the node with no keys and sets the key counter to 0.
    BPlusNode() : BidirectionalNode(), m_keys(), m_key_counter(0){}

    /// @brief Default destructor. Virtual to allow subclasses to be deletable.
    virtual ~BPlusNode(){}

    /// @brief A pure virtual function to search for the data associated with a given key in the node's subtree.
    /// @param key The key to search for.
    /// @return A pointer to the mapped data associated with the key or nullptr if there isn't one.
    virtual Mapped* search (const Key&) = 0;

    /// @brief A pure virtual function to insert a new key and its associated data into the node's subtree.
    /// @param key The key to insert.
    /// @param data The data associated with the key.
    /// @return A pointer to the newly created node if the node was full and split. Otherwise, returns `nullptr` if the insertion is successful and the node is not full.
    virtual BPlusNode* insert (const Key&, const Mapped&) = 0;

    /// @brief Deletes the data associated with the given key from the node. 
    /// @param key The key to erase.
    /// @return `true` if the deletion was successful, `false` otherwise.
    virtual bool erase(const Key&) = 0;

    /// @brief Removes all data if present.
    virtual void erase_all() = 0;

    /// @brief Gets the minimum key stored in the node.
    /// @return A reference to the minimum key.
    /// @note never to be called on an empty node.
    virtual const Key& min_key() const = 0;

    /// @brief Gets the maximum key stored in the node.
    /// @return A reference to the maximum key.
    /// @note never to be called on an empty node.
    virtual const Key& max_key() const = 0;

    /// @brief Finds the index of the smallest key in the node that is strictly greater than the given key.
    /// @param key The key to find the smallest bigger key for.
    /// @return The index of the smallest key larger than the given `key` in the `m_keys` array.
    /// @note Time Complexity: O(log x), where x is the number of keys currently stored in the node.
    /// @note Space Complexity: O(1).
    std::size_t find_smallest_bigger_key_index(const Key& key);
};

/// @brief A class representing a leaf node in a B+ tree. 
/// @tparam N The maximum number of elemenets the leaf node can store.
/// @tparam Key The type of the key used in the B+ tree nodes. 
/// * must be move-constructible.
/// * must be copy-constructible.
/// * must be destructible.
/// * must supports the comparison operations '<' and '=='.
/// @tparam Mapped The type of the data mapped to each key in the leaf nodes.
/// * must be move-constructible.
/// * must be copy-constructible.
/// * must be destructible.
template<std::size_t N, OrderedKey Key, Storable Mapped>
class BPlusLeafNode : public BPlusNode<N, Key, Mapped> {
protected:
    std::array<Mapped*, N> m_data;
public:
    /// @brief Constructor for the `BPlusLeafNode` class. Initializes the node with no keys, no data and no linked nodes.
    BPlusLeafNode() : BPlusNode<N, Key, Mapped>(), m_data(){}

    /// @brief Default destructor. Virtual to allow subclasses to be deletable.
    virtual ~BPlusLeafNode(){}

    /// @brief Searches for the data associated with the given key in the leaf node.
    /// @param key The key to search for.
    /// @return A pointer to the mapped data associated with the key or nullptr if there isn't one.
    Mapped* search(const Key& key) override;

    /// @brief Inserts the given key and associated data into the leaf node. If the node becomes full, it splits
    ///        into a new node and returns a pointer to the new node. No duplicates allowed.
    /// @param key The key to insert.
    /// @param value The data to associate with the key.
    /// @return A pointer to the newly created node if the current node is full; otherwise, a null pointer.
    BPlusNode<N, Key, Mapped>* insert(const Key& key, const Mapped& value) override;

    /// @brief Removes the key and its associated data if present.
    /// @param key The key to delete.
    /// @return `true` if the deletion was successful, `false` if the key was not found.
    bool erase(const Key& key) override;

    /// @brief Removes all data if present.
    void erase_all() override;

    /// @brief Gets the minimum key stored in the node.
    /// @return A reference to the minimum key.
    /// @note never to be called on an empty node.
    const Key& min_key() const override {
        return this->m_keys[0]; // The minimum key is always the first key in a leaf node.
    }

    /// @brief Gets the maximum key stored in the node.
    /// @return A reference to the maximum key.
    /// @note never to be called on an empty node.
    const Key& max_key() const override {
        return this->m_keys[this->m_key_counter - 1]; // The maximum key is always the last key in a leaf node.
    }
};

/// @brief A class representing an internal node in a B+ tree. 
/// @tparam N The maximum number of elements the leaf node can store.
/// @tparam Key The type of the key used in the B+ tree nodes. 
/// * must be move-constructible.
/// * must be copy-constructible.
/// * must be destructible.
/// * must supports the comparison operations '<' and '=='.
/// @tparam Mapped The type of the data mapped to each key in the leaf nodes.
/// * must be move-constructible.
/// * must be copy-constructible.
/// * must be destructible.
template<std::size_t N, OrderedKey Key, Storable Mapped>
class BPlusInternalNode : public BPlusNode<N, Key, Mapped> {
protected:
    std::array<BPlusNode<N, Key, Mapped>*, N + 1> m_children; // one more child than declared in the tparam, but is only used temporarily in insertion
    Key m_min_key, m_max_key; // extremal keys in subtree
public:
    /// @brief Constructor for the `BPlusInternalNode`. Initializes the node with no keys, no children, and no linked nodes.
    BPlusInternalNode() : BPlusNode<N, Key, Mapped>(), m_children(), m_min_key(), m_max_key(){}

    /// @brief Default destructor. Virtual to allow subclasses to be deletable.
    virtual ~BPlusInternalNode(){}

    /// @brief Searches for the data associated with the given key by traversing the internal node's children.
    ///        This method delegates the search to the appropriate child node.
    /// @param key The key to search for.
    /// @return A pointer to the mapped data associated with the key or nullptr if there isn't one.
    Mapped* search(const Key& key) override;

    /// @brief Inserts the given key and associated data into the internal node's subtree. If the node becomes full, 
    ///        it splits into a new node and returns a pointer to the newly created node.
    /// @param key The key to insert.
    /// @param value The data to associate with the key.
    /// @return A pointer to the newly created node if the current node is full; otherwise, a null pointer.
    BPlusNode<N, Key, Mapped>* insert(const Key& key, const Mapped& value) override;

    /// @brief Deletes the data associated with the given key from the internal node's subtree.
    ///        If any child node is underflowed, it is its responsibility for handling underflow.
    /// @param key The key to delete.
    /// @return `true` if the deletion was successful, `false` if the key was not found.
    bool erase(const Key& key) override;

    /// @brief Removes all data and children in subtree if present.
    void erase_all() override;

    /// @brief Gets the minimum key stored in the node.
    /// @return A reference to the minimum key.
    /// @note never to be called on an empty node.
    const Key& min_key() const override {
        return this->m_min_key; // saved inside.
    }

    /// @brief Gets the maximum key stored in the node.
    /// @return A reference to the maximum key.
    /// @note never to be called on an empty node.
    const Key& max_key() const override {
        return this->m_max_key; // saved inside.
    }

    /// @brief Handles underflows that occur after deletion in an internal node. This method is responsible for
    ///        deciding whether to merge nodes or redistribute keys between siblings, and it propagates the underflow
    ///        signal to the parent if necessary.
    /// @param underflow_index The index of the underflowing subnode.
    void handle_underflow(std::size_t underflow_index);
};

//////////////////////////////////////////////////////////////////////////////////
//                        BPlusNode Implementation Block                        //
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, OrderedKey Key, Storable Mapped>
std::size_t BPlusNode<N, Key, Mapped>::find_smallest_bigger_key_index(const Key& key)
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

//////////////////////////////////////////////////////////////////////////////////
//                      BPlusLeafNode Implementation Block                      //
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, OrderedKey Key, Storable Mapped>
Mapped *BPlusLeafNode<N, Key, Mapped>::search(const Key& key)
{
    auto index = this->find_smallest_bigger_key_index(key);
    // in leafs, d[i] corresponds to k[i]; in internal nodes, d[i] contains search keys which are all less than k[i]
    if (index == 0 || this->m_keys[index - 1] != key) return nullptr; 
    return m_data[index-1];
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
BPlusNode<N, Key, Mapped>* BPlusLeafNode<N, Key, Mapped>::insert(const Key& key, const Mapped& value)
{
    auto index = this->find_smallest_bigger_key_index(key);
    if (index != 0 && this->m_keys[index - 1] == key) return nullptr; 
    BPlusLeafNode* inserting_node = this, * out_ptr = nullptr;
    if (this->m_key_counter == N){ // split
        auto new_node = new BPlusLeafNode();
        std::size_t mid = (key < this->m_keys[N/2]) ? N/2 : N - N/2; // stabilizing the sizes
        inserting_node = (key < this->m_keys[N/2]) ? this : new_node; // choose the smaller node
        out_ptr = new_node;

        for (std::size_t i = mid; i < N; ++i) {
            new_node->m_data[i - mid] = this->m_data[i];
            new_node->m_keys[i - mid] = this->m_keys[i];
        }
        this->m_key_counter = mid;
        new_node->m_key_counter = N - mid;

        this->add_node_after_this(new_node);
        this->next = new_node;
        new_node->prev = this;
    }
    std::size_t i = inserting_node->m_key_counter;
    for (; (i > 0)&&(key < inserting_node->m_keys[i-1]); --i){ // move them all one step right to fit the new one
        inserting_node->m_data[i] = inserting_node->m_data[i-1];
        inserting_node->m_keys[i] = inserting_node->m_keys[i-1];
    }
    inserting_node->m_data[i] = new Mapped (value);
    inserting_node->m_keys[i] = key;
    ++inserting_node->m_key_counter;
    return out_ptr;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
bool BPlusLeafNode<N, Key, Mapped>::erase(const Key &key)
{
    std::size_t i = this->find_smallest_bigger_key_index(key);
    if (i==0 || this->m_keys[i-1] != key)
        return false;

    delete this->m_data[i-1];
    
    for (; i < this->m_key_counter; ++i){ // move them all one step left
        this->m_data[i-1] = this->m_data[i];
        this->m_keys[i-1] = this->m_keys[i];
    }
    --this->m_key_counter;
    return true;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
void BPlusLeafNode<N, Key, Mapped>::erase_all()
{
    for (std::size_t i = 0; i < this->m_key_counter; ++i){
        delete this->m_data[i];
    }
    this->m_key_counter = 0;
}

//////////////////////////////////////////////////////////////////////////////////
//                    BPlusInternalNode Implementation Block                    //
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, OrderedKey Key, Storable Mapped>
Mapped* BPlusInternalNode<N, Key, Mapped>::search(const Key& key)
{
    return this->m_children[this->find_smallest_bigger_key_index(key)]->search(key);
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
BPlusNode<N, Key, Mapped> *BPlusInternalNode<N, Key, Mapped>::insert(const Key& key, const Mapped& value)
{
    std::size_t working_index = this->find_smallest_bigger_key_index(key);
    auto new_ptr = this->m_children[working_index]->insert(key, value);
    if (new_ptr == nullptr) return nullptr; // case 0: child was not split, nothing to do further
    // case 1: child was split, need to handle split
    std::size_t i = this->m_key_counter;
    for (; i > working_index; --i){ // move them all one step right to fit the new one, the new one is never in 0.
        this->m_children[i+1] = this->m_children[i];
        this->m_keys[i] = this->m_keys[i-1];
    }
    this->m_children[i+1] = new_ptr;
    this->m_keys[i] = new_ptr->min_key();
    ++this->m_key_counter;
    
    BPlusInternalNode<N, Key, Mapped>* out_ptr = nullptr;
    // case 2: this too must split if it violates the definition of N
    if (this->m_key_counter == N+1){ 
        auto new_node = new BPlusInternalNode<N, Key, Mapped>();
        std::size_t mid = N/2; // stabilizing the sizes
        out_ptr = new_node;

        for (std::size_t i = mid; i < N; ++i) {
            new_node->m_children[i - mid] = this->m_children[i];
            new_node->m_keys[i - mid] = this->m_keys[i];
        }
        this->m_key_counter = mid;
        new_node->m_key_counter = N - mid;

        this->add_node_after_this(new_node);
        this->next = new_node;
        new_node->prev = this;
    }
    // stabilize keys for parent handling
    this->m_max_key = this->m_children[this->m_key_counter]->max_key();
    if (out_ptr){
        out_ptr->m_min_key = out_ptr->m_children[0]->min_key();
        out_ptr->m_max_key = out_ptr->m_children[out_ptr->m_key_counter]->max_key();
    }
    return out_ptr;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
bool BPlusInternalNode<N, Key, Mapped>::erase(const Key& key)
{
    std::size_t working_index = this->find_smallest_bigger_key_index(key);
    auto res = this->m_children[working_index]->erase(key);
    if (this->m_children[working_index]->m_key_counter >= N/2) return res; // if there's no underflow, return
    handle_underflow(working_index);                                       // else, fix the situation and then return
    return res;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
void BPlusInternalNode<N, Key, Mapped>::erase_all()
{
    for (std::size_t i = 0; i <= this->m_key_counter; ++i){
        this->m_children[i]->erase_all();
        delete this->m_children[i];
    }
    this->m_key_counter = 0;
}

template <std::size_t N, OrderedKey Key, Storable Mapped>
void BPlusInternalNode<N, Key, Mapped>::handle_underflow(std::size_t underflow_index)
{
    // assumes having at least two sons, for the case for less would have already been dealt with
    std::size_t neighbour_index = (underflow_index > 0) ? underflow_index - 1 : underflow_index + 1;
    BPlusNode<N, Key, Mapped>* underflow_subnode = this->m_children[underflow_index], * neighbour_subnode = this->m_children[neighbour_index];
    if (underflow_subnode->m_key_counter + neighbour_subnode->m_key_counter > N){
        // Case 1: borrow
        if (underflow_index > neighbour_index){
            // first move all things here rightwards
            underflow_subnode->m_children[underflow_subnode->m_key_counter + 1] = underflow_subnode->m_children[underflow_subnode->m_key_counter];
            for (std::size_t i = underflow_subnode->m_key_counter; i > 0; --i) {
                underflow_subnode->m_keys[i] = underflow_subnode->m_keys[i - 1];
                underflow_subnode->m_children[i] = underflow_subnode->m_children[i - 1];
            }
            // then borrow at the start
            underflow_subnode->m_keys[0] = underflow_subnode->m_children[1].min_key();
            underflow_subnode->m_children[0] = neighbour_subnode->m_children[neighbour_subnode->m_key_counter];

            --neighbour_subnode->m_key_counter;
            ++underflow_subnode->m_key_counter;
            // then fix extremes and this' keys
            neighbour_subnode->m_max_key = neighbour_subnode->m_children[neighbour_subnode->m_key_counter]->max_key();
            underflow_subnode->m_min_key = underflow_subnode->m_children[0]->min_key();
            this->m_keys[neighbour_index] = underflow_subnode->min_key;
        }
        else{
            // first borrow at the end
            underflow_subnode->m_keys[underflow_subnode->m_key_counter] = this->m_keys[underflow_index];
            underflow_subnode->m_children[underflow_subnode->m_key_counter + 1] = neighbour_subnode->m_children[0];

            --neighbour_subnode->m_key_counter;
            ++underflow_subnode->m_key_counter;
            // then move all the neighbour's leftwards
            for (std::size_t i = 0; i < underflow_subnode->m_key_counter; ++i) {
                neighbour_subnode->m_keys[i] = underflow_subnode->m_keys[i + 1];
                neighbour_subnode->m_children[i] = underflow_subnode->m_children[i + 1];
            }
            neighbour_subnode->m_children[neighbour_subnode->m_key_counter] = neighbour_subnode->m_children[neighbour_subnode->m_key_counter + 1];
            // then fix extremes and this' keys
            underflow_subnode->m_max_key = underflow_subnode->m_children[underflow_subnode->m_key_counter]->max_key();
            neighbour_subnode->m_min_key = neighbour_subnode->m_children[0]->min_key();
            this->m_keys[neighbour_index] = neighbour_subnode->min_key;
        }
    }
    else{
        // Case 2: merge
        if (underflow_index > neighbour_index) { 
            neighbour_subnode->m_keys[neighbour_subnode->m_key_counter - 1] = this->m_keys[underflow_index];

            for (std::size_t i = 0; i < underflow_subnode->m_key_counter; ++i) {
                neighbour_subnode->m_keys[neighbour_subnode->m_key_counter + i + 1] = underflow_subnode->m_keys[i];
                neighbour_subnode->m_children[neighbour_subnode->m_key_counter + i + 1] = underflow_subnode->m_children[i];
            }
            neighbour_subnode->m_children[neighbour_subnode->m_key_counter + 
                                            underflow_subnode->m_key_counter + 1] = underflow_subnode->m_children[underflow_subnode->m_key_counter];

            neighbour_subnode->m_key_counter += underflow_subnode->m_key_counter + 1;
            neighbour_subnode->m_max_key = neighbour_subnode->m_children[neighbour_subnode->m_key_counter]->max_key();

            if (underflow_subnode->next){
                underflow_subnode->next->prev = neighbour_subnode;
            }
            neighbour_subnode->next = underflow_subnode->next;

            delete underflow_subnode;
        }
        else { 
            underflow_subnode->m_keys[underflow_subnode->m_key_counter - 1] = this->m_keys[neighbour_index];

            for (std::size_t i = 0; i < neighbour_subnode->m_key_counter; ++i) {
                underflow_subnode->m_keys[underflow_subnode->m_key_counter + i + 1] = neighbour_subnode->m_keys[i];
                underflow_subnode->m_children[underflow_subnode->m_key_counter + i + 1] = neighbour_subnode->m_children[i];
            }
            underflow_subnode->m_children[underflow_subnode->m_key_counter + 
                                            neighbour_subnode->m_key_counter + 1] = neighbour_subnode->m_children[neighbour_subnode->m_key_counter];

            underflow_subnode->m_key_counter += neighbour_subnode->m_key_counter + 1;
            underflow_subnode->m_max_key = underflow_subnode->m_children[underflow_subnode->m_key_counter]->max_key();

            if (neighbour_subnode->next){
                neighbour_subnode->next->prev = underflow_subnode;
            }
            underflow_subnode->next = neighbour_subnode->next;

            delete neighbour_subnode;
        }

        this->m_key_counter--;
        for (std::size_t i = underflow_index; i < this->m_key_counter; ++i) {
            this->m_keys[i] = this->m_keys[i + 1];
            this->m_children[i + 1] = this->m_children[i + 2];
        }

        this->m_max_key = this->m_children[this->m_key_counter]->max_key();
    }
}

#endif
