#ifndef BPLUS_NODE_TEST_INTERFACE_DEFINED
#define BPLUS_NODE_TEST_INTERFACE_DEFINED

#include "../src/BPlusNode.hpp"

template<std::size_t N, OrderedKey Key, Storable Mapped>
class BPlusLeafTestInterface : public BPlusLeafNode<N, Key, Mapped> {
public:
    BPlusLeafTestInterface() : BPlusLeafNode<N, Key, Mapped>(){}
    const BidirectionalNode* get_next() const {return this->next;}
    const BidirectionalNode* get_prev() const {return this->prev;}
    bool is_split_from_me (const BPlusNode<N, Key, Mapped>* const other) const {return this->is_node_after_me(other);}
    const Key& get_key(std::size_t index) const {return this->m_keys[index];}
    std::size_t get_key_num() const {return this->m_key_counter;}
    const Mapped* get_data(std::size_t index) const {return this->m_data[index];}
};

template<std::size_t N, OrderedKey Key, Storable Mapped>
class BPlusInternalNodeTestInterface : public BPlusInternalNode<N, Key, Mapped> {
public:
    BPlusInternalNodeTestInterface(BPlusNode<N, Key, Mapped>* init) : BPlusInternalNode<N, Key, Mapped>(){this->m_children[0] = init; this->m_min_key = this->m_children[0]->min_key();}
    const BidirectionalNode* get_next() const {return this->next;}
    const BidirectionalNode* get_prev() const {return this->prev;}
    bool is_split_from_me (const BPlusNode<N, Key, Mapped>* const other) const {return (other!=nullptr)&&(other->prev = this)&&(this->next = other);}
    std::size_t get_sum_sizes (const BPlusNode<N, Key, Mapped>* const other) const {return this->m_key_counter + other->m_key_counter ;}
    const Key& get_key(std::size_t index) const {return this->m_keys[index];}
    std::size_t get_key_num() const {return this->m_key_counter;}
    const BPlusNode<N, Key, Mapped>* get_child(std::size_t index) const {return this->m_children[index];}
    bool push_child(BPlusNode<N, Key, Mapped>* child) {
        if (this->m_key_counter == N + 1) return false;
        this->m_children[this->m_key_counter + 1] = child;
        this->m_keys[this->m_key_counter] = child->min_key();
        ++this->m_key_counter;
        this->m_max_key = child->max_key();
        return true;
    }
};

#endif
