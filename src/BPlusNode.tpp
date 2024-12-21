#include "BPlusTree.hpp"

//////////////////////////////////////////////////////////////////////////////////
//                        BPlusNode Implementation Block                        //
//////////////////////////////////////////////////////////////////////////////////
template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::BPlusNode::BPlusNode(bool is_leaf)
{
    m_key_counter = 0;
    m_next = nullptr;
    m_prev = nullptr;
    m_keys = std::array<Key, N>();
    if (is_leaf){
        m_data = std::array<pointer, N> ();
    }
    else{
        m_data = std::array<BPlusNode*, N> ();
    }
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::pointer BPlusTree<Key, Mapped, N>::BPlusNode::search(const key_type& key) const
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

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::BPlusNode *BPlusTree<Key, Mapped, N>::BPlusNode::insert(key_type&&key, mapped_type&&mapped)
{
    std::variant<pointer, BPlusNode*> inserted_ptr;
    Key inserted_key;
    if (m_data.index() == 0){                           // if leaf
        if (search(key) != nullptr) return nullptr;
        inserted_ptr = new value_type (std::move(mapped));
        inserted_key = Key (std::move(key));
    }
    else{                                               // if internal
        std::size_t working_index = this->find_smallest_bigger_key_index(key);
        working_index -= (working_index != 0);
        inserted_ptr = std::get<1>(this->m_data)[working_index]->insert(std::move(key), std::move(mapped));
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

        std::copy_n(this->m_keys.begin() + mid, this->m_key_counter - mid, new_node->m_keys.begin());
        if (this->m_data.index() == 0){
            std::copy_n(std::get<0>(this->m_data).begin() + mid,               // first
                        this->m_key_counter - mid,                             // count
                        std::get<0>(new_node->m_data).begin());                // result
        }
        else{
            std::copy_n(std::get<1>(this->m_data).begin() + mid, 
                        this->m_key_counter - mid, 
                        std::get<1>(new_node->m_data).begin());
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

template <OrderedKey Key, Storable Mapped, std::size_t N>
bool BPlusTree<Key, Mapped, N>::BPlusNode::erase(const key_type& key)
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
    
    std::copy_n(std::get<0>(this->m_data).begin() + i,      // move everything left
                this->m_key_counter - i, 
                std::get<0>(this->m_data).begin() + i - 1);
    std::copy_n(this->m_keys.begin() + i, 
                this->m_key_counter - i, 
                this->m_keys.begin() + i - 1);

    --this->m_key_counter;
    return true;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
void BPlusTree<Key, Mapped, N>::BPlusNode::erase_all()
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

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::size_type BPlusTree<Key, Mapped, N>::BPlusNode::find_smallest_bigger_key_index(const key_type& key) const
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

template <OrderedKey Key, Storable Mapped, std::size_t N>
template <bool is_internal>
void BPlusTree<Key, Mapped, N>::BPlusNode::handle_underflow(size_type underflow_index)
{
    // assumes having at least two sons, for the case for less would have already been dealt with
    std::size_t neighbour_index = (underflow_index > 0) ? underflow_index - 1 : underflow_index + 1;
    BPlusNode* underflow_subnode = std::get<1>(this->m_data)[underflow_index], * neighbour_subnode = std::get<1>(this->m_data)[neighbour_index];
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
            std::copy_n(std::get<is_internal>(neighbour_subnode->m_data).begin() + 1,      // move everything left
                        neighbour_subnode->m_key_counter, 
                        std::get<is_internal>(neighbour_subnode->m_data).begin());
            std::copy_n(neighbour_subnode->m_keys.begin() + 1, 
                        neighbour_subnode->m_key_counter, 
                        neighbour_subnode->m_keys.begin());
            // then fix extremes and this' keys
            this->m_keys[neighbour_index] = neighbour_subnode->m_keys[0];
        }
    }
    else{
        // Case 2: merge
        if (underflow_index > neighbour_index) { 
            merge<is_internal>(neighbour_subnode, underflow_subnode);
        }
        else { 
            merge<is_internal>(underflow_subnode, neighbour_subnode);
        }
        std::size_t i = std::max(underflow_index, neighbour_index) + 1;
        std::copy_n(std::get<1>(this->m_data).begin() + i,      // move everything left
                    this->m_key_counter - i, 
                    std::get<1>(this->m_data).begin() + i - 1);
        std::copy_n(this->m_keys.begin() + i, 
                    this->m_key_counter - i, 
                    this->m_keys.begin() + i - 1);
        this->m_key_counter--;
    }
}


template <OrderedKey Key, Storable Mapped, std::size_t N>
template <bool is_internal>
void BPlusTree<Key, Mapped, N>::merge(BPlusTree::BPlusNode* lower_node, BPlusTree::BPlusNode* higher_node){
    for (std::size_t i = 0; i < higher_node->m_key_counter; ++i) {
        lower_node->m_keys[lower_node->m_key_counter + i] = higher_node->m_keys[i];
        std::get<is_internal>(lower_node->m_data)[lower_node->m_key_counter + i] = std::get<is_internal>(higher_node->m_data)[i];
    }

    lower_node->m_key_counter += higher_node->m_key_counter;

    if (higher_node->m_next){
        higher_node->m_next->m_prev = lower_node;
    }
    lower_node->m_next = higher_node->m_next;

    delete higher_node;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusTree<Key, Mapped, N>::BPlusNode *BPlusTree<Key, Mapped, N>::copy(const BPlusNode *source, const BPlusNode *prev, bool has_min, bool has_max)
{
    BPlusNode* out = new BPlusNode (source->m_data.index());
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
