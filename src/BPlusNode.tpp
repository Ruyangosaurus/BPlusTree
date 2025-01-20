#include "BPlusTree.hpp"

//////////////////////////////////////////////////////////////////////////////////
//                        BPlusNode Implementation Block                        //
//////////////////////////////////////////////////////////////////////////////////
template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusNode<Key, Mapped, N>::BPlusNode(bool is_leaf)
{
    m_key_counter = 0;
    this->m_next = nullptr;
    this->m_prev = nullptr;
    m_keys = std::array<Key, N>();
    if (is_leaf){
        m_data = std::array<pointer, N> ();
    }
    else{
        m_data = std::array<BPlusNode*, N> ();
    }
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusNode<Key, Mapped, N>::pointer BPlusNode<Key, Mapped, N>::search(const key_type& key) const
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
void BPlusNode<Key, Mapped, N>::restabilize(BPlusNode *existing_node, BPlusNode *new_node)
{
    std::variant<pointer, BPlusNode*> inserted_ptr;
    key_type inserted_key = new_node->m_keys[0];
    size_type i = existing_node->find_smallest_bigger_key_index(inserted_key);
    i -= (i != 0);
    if (existing_node->m_data.index() != 0){
        inserted_ptr = std::get<1>(new_node->m_data)[0];    
        restabilize(std::get<1>(existing_node->m_data)[i], std::get<1>(new_node->m_data)[0]);
        inserted_key = std::get<1>(new_node->m_data)[0]->m_keys[0];
    }
    else{
        inserted_ptr = std::get<0>(new_node->m_data)[0];  
    }
    std::size_t mid = (inserted_key < existing_node->m_keys[N/2]) ? N/2 : N - N/2;  // stabilizing the sizes
    BPlusNode* inserting_node = (inserted_key < existing_node->m_keys[N/2]) ? existing_node : new_node; // choose the smaller node
    
    std::copy_n(existing_node->m_keys.begin() + mid, existing_node->m_key_counter - mid, new_node->m_keys.begin());
    if (existing_node->m_data.index() == 0){
        std::copy_n(std::get<0>(existing_node->m_data).begin() + mid,               // first
                    existing_node->m_key_counter - mid,                             // count
                    std::get<0>(new_node->m_data).begin());                // result
    }
    else{
        std::copy_n(std::get<1>(existing_node->m_data).begin() + mid, 
                    existing_node->m_key_counter - mid, 
                    std::get<1>(new_node->m_data).begin());
    }
    existing_node->m_key_counter = mid;
    new_node->m_key_counter = N - mid;

    if (existing_node->m_next){
        existing_node->m_next->m_prev = new_node;
    }
    new_node->m_next = existing_node->m_next;
    existing_node->m_next = new_node;
    new_node->m_prev = existing_node;

    i = inserting_node->m_key_counter;
    if (inserting_node->m_data.index() == 0){
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
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
template<SingleElementAllocator ValAlloc, SingleElementAllocator NodeAlloc, bool Try, typename ... Args> requires std::constructible_from<Mapped, Args...>
BPlusNode<Key, Mapped, N> *BPlusNode<Key, Mapped, N>::emplace(BPlusTree<Key, Mapped, N, ValAlloc, NodeAlloc>& tree, key_type&&key, Args&& ... args) noexcept
{
    std::variant<pointer, BPlusNode*> inserted_ptr;
    Key inserted_key;
    if (m_data.index() == 0){                           // if leaf
        try{
            pointer val_ptr = search(key);
            if (val_ptr != nullptr){
                if constexpr(Try){
                    *val_ptr = std::move(args...);
                }
                return nullptr;
            };
            inserted_ptr = std::construct_at(tree.m_val_alloc.allocate(), std::move(args...));
        }
        catch(std::bad_alloc& e){
            return nullptr;
        }
        inserted_key = Key (std::move(key));
    }
    else{                                               // if internal
        std::size_t working_index = this->find_smallest_bigger_key_index(key);
        working_index -= (working_index != 0);
        if (this->m_keys[working_index] > key){
            this->m_keys[working_index] = key;
        }
        inserted_ptr = std::get<1>(this->m_data)[working_index]->emplace(tree, std::move(key), std::move(args...));
        if (std::get<1>(inserted_ptr) == nullptr) return nullptr;
        if (this->m_key_counter != N){
            restabilize(std::get<1>(this->m_data)[working_index], std::get<1>(inserted_ptr));
        }
        inserted_key = std::get<1>(inserted_ptr)->m_keys[0];
    }

    if (this->m_key_counter == N){ // split
        BPlusNode* new_node;
        try{
            new_node = std::construct_at(tree.m_node_alloc.allocate(), this->m_data.index() == 0);              // index == 0 <-> this is a leaf
        }
        catch(std::bad_alloc& e){
            while(inserted_ptr.index() != 0){
                auto temp = std::get<1>(inserted_ptr);
                if (temp->m_data.index() == 0){
                    inserted_ptr = std::get<1>(temp->m_data)[0];
                }
                else{
                    inserted_ptr = std::get<0>(temp->m_data)[0];
                }
                std::destroy_at(temp);
                tree.m_node_alloc.deallocate(temp);
            }
            std::destroy_at(std::get<0>(inserted_ptr));
            tree.m_val_alloc.deallocate(std::get<0>(inserted_ptr));
            return nullptr;
        }
        if (tree.m_max == this){
            tree.m_max = new_node;
        }
        new_node->m_keys[0] = inserted_key;
        if (this->m_data.index() == 0){
            std::get<0>(new_node->m_data)[0] = std::get<0>(inserted_ptr);

        }
        else{
            std::get<1>(new_node->m_data)[0] = std::get<1>(inserted_ptr);
        }
        return new_node;
    }
    std::size_t i = this->m_key_counter;
    if (this->m_data.index() == 0){
        for (; (i > 0)&&(inserted_key < this->m_keys[i-1]); --i){ // move them all one step right to fit the new one
            std::get<0>(this->m_data)[i] = std::get<0>(this->m_data)[i-1];
        }
        std::get<0>(this->m_data)[i] = std::get<0>(inserted_ptr);
    }
    else{
        for (; (i > 0)&&(inserted_key < this->m_keys[i-1]); --i){ // move them all one step right to fit the new one
            std::get<1>(this->m_data)[i] = std::get<1>(this->m_data)[i-1];
        }
        std::get<1>(this->m_data)[i] = std::get<1>(inserted_ptr);
    }
    for (i = this->m_key_counter; (i > 0)&&(inserted_key < this->m_keys[i-1]); --i){ // move them all one step right to fit the new one
        this->m_keys[i] = this->m_keys[i-1];
    }
    this->m_keys[i] = inserted_key;
    ++this->m_key_counter;
    return nullptr;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
template<SingleElementAllocator ValAlloc, SingleElementAllocator NodeAlloc>
bool BPlusNode<Key, Mapped, N>::erase(BPlusTree<Key, Mapped, N, ValAlloc, NodeAlloc>& tree, const key_type& key)
{
    std::size_t i = this->find_smallest_bigger_key_index(key);
    if (i == 0) return false;
    if (this->m_data.index() != 0){
        --i;
        auto res = std::get<1>(this->m_data)[i]->erase(tree, key);
        if (std::get<1>(this->m_data)[i]->m_key_counter >= N-N/2) return res; // if there's no underflow, return
        if (std::get<1>(this->m_data)[i]->m_data.index() == 0){ // else, fix the situation and then return
            handle_underflow<ValAlloc, NodeAlloc, false>(tree, i);
        }
        else{
            handle_underflow<ValAlloc, NodeAlloc, true>(tree, i);
        }
        return res;
    }
    if (this->m_keys[i - 1] != key)
        return false;

    std::destroy_at(std::get<0>(this->m_data)[i-1]);
    tree.m_val_alloc.deallocate(std::get<0>(this->m_data)[i-1]);
    
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
template<SingleElementAllocator ValAlloc, SingleElementAllocator NodeAlloc>
void BPlusNode<Key, Mapped, N>::erase_all(BPlusTree<Key, Mapped, N, ValAlloc, NodeAlloc>& tree)
{
    if (this->m_data.index() == 0){
        for (std::size_t i = 0; i < this->m_key_counter; ++i){
            std::destroy_at(std::get<0>(this->m_data)[i]);
            tree.m_val_alloc.deallocate(std::get<0>(this->m_data)[i]);
        }
    }
    else{
        for (std::size_t i = 0; i < this->m_key_counter; ++i){
            std::get<1>(this->m_data)[i]->erase_all(tree);
            std::destroy_at(std::get<1>(this->m_data)[i]);
            tree.m_node_alloc.deallocate(std::get<1>(this->m_data)[i]);
        }
    }
    this->m_key_counter = 0;
}

template <OrderedKey Key, Storable Mapped, std::size_t N>
BPlusNode<Key, Mapped, N>::size_type BPlusNode<Key, Mapped, N>::find_smallest_bigger_key_index(const key_type& key) const
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
template<SingleElementAllocator ValAlloc, SingleElementAllocator NodeAlloc, bool is_internal>
void BPlusNode<Key, Mapped, N>::handle_underflow(BPlusTree<Key, Mapped, N, ValAlloc, NodeAlloc>& tree, size_type underflow_index)
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
            if (tree.m_max == underflow_subnode) {
                tree.m_max = neighbour_subnode;
            }
            tree.template merge<is_internal>(neighbour_subnode, underflow_subnode);
        }
        else { 
            if (tree.m_max == neighbour_subnode) {
                tree.m_max = underflow_subnode;
            }
            tree.template merge<is_internal>(underflow_subnode, neighbour_subnode);
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
