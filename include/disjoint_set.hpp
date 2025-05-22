#ifndef _ICY_DISJOINT_SET_HPP_
#define _ICY_DISJOINT_SET_HPP_

#include "disjoint_base.hpp"
#include <cassert>
#include <unordered_map>
#include <unordered_set>

namespace icy {

template <typename _Key, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Key>> struct disjoint_set;

/**
 * @brief disjoint set, a container for managing the set to which elements belongs
 * @tparam _Key type of key object
 * @tparam _Hash hashing function object type, defaults to std::hash<_Key>.
 * @tparam _Alloc allocator type, defaults to std::allocator<_Key>.
 * @implements implemented by hash table and short tree
*/
template <typename _Key, typename _Hash, typename _Alloc>
struct disjoint_set : public disjoint_base<_Key, void, _Hash, _Alloc> {
    using base = disjoint_base<_Key, void, _Hash, _Alloc>;
    using self = disjoint_set<_Key, _Hash, _Alloc>;
    using node_type = typename base::node_type;
    using header_type = typename base::header_type;
    using key_type = typename base::key_type;
public:
    disjoint_set() = default;
    disjoint_set(std::initializer_list<std::initializer_list<key_type>>);
    disjoint_set(const self& _rhs);
    auto operator=(const self& _rhs) -> self&;
    virtual ~disjoint_set() = default;
public:
    auto operator==(const self& _rhs) const -> bool;
    auto operator!=(const self& _rhs) const -> bool;
public:
    /**
     * @brief add the specific key to a new classification
     * @param _k the specific key
     * @return return false when the key is already in disjoint set or the key fails to be added
     */
    auto add(const key_type& _k) -> bool;
    /**
     * @brief add the specific key to the classification, which contains the given key
     * @param _k the specific key
     * @param _target the given key
     * @return return false when the @c _target is not in disjoint set or the key fails to be added
     */
    auto add(const key_type& _k, const key_type& _target) -> bool;
private:
    auto _M_assign(const self& _rhs) -> void;
};

template <typename _Key, typename _Hash, typename _Alloc>
disjoint_set<_Key, _Hash, _Alloc>::disjoint_set(std::initializer_list<std::initializer_list<key_type>> _llk) {
    for (auto _i = _llk.begin(); _i != _llk.end(); ++_i) {
        if (_i->begin() != _i->end()) {
            add(*(_i->begin()));
        }
        for (auto _j = _i->begin(); _j != _i->end(); ++_j) {
            add(*_j, *(_i->begin()));
        }
    }
};
template <typename _Key, typename _Hash, typename _Alloc>
disjoint_set<_Key, _Hash, _Alloc>::disjoint_set(const self& _rhs) : base(_rhs) {
    _M_assign(_rhs);
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::operator=(const self& _rhs) -> self& {
    if (&_rhs == this) return *this;
    this->clear(); _M_assign(_rhs);
    return *this;
};

/**
 * @implements T <= o(size) * o(classification), S = o(classification)
 * prove that `*this` \subseteq `_rhs` and `|*this|` \eq `|_rhs|`
 */
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::operator==(const self& _rhs) const -> bool {
    if (this->size() != _rhs.size() || this->classification() != _rhs.classification()) {
        return false;
    }
    std::vector<key_type> _delegate_keys; // for `_rhs`
    auto index_of_key = [this, &_delegate_keys](const key_type& _k) -> size_t {
        for (size_t _i = 0; _i != _delegate_keys.size(); ++_i) {
            if (this->sibling(_k, _delegate_keys[_i])) return _i;
        }
        return _delegate_keys.size();
    };
    for (const auto& _i : this->_nodes) {
        const key_type& _k = _i.first;
        node_type* const _n = _i.second;
        if (!_rhs.contains(_k)) {
            return false;
        }
        if (_rhs.sibling(_k) != this->sibling(_k)) {
            return false;
        }
        const size_t _index = index_of_key(_k);
        if (_index == _delegate_keys.size()) {
            if (this->sibling(_k) != 1) {
                _delegate_keys.push_back(_k);
            }
            continue;
        }
        const key_type& _dk = _delegate_keys[_index];
        if (!_rhs.sibling(_k, _dk)) {
            return false;
        }
    }
    return true;
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::operator!=(const self& _rhs) const -> bool {
    return !this->operator==(_rhs);
}

template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::add(const key_type& _k) -> bool {
    if (this->contains(_k)) return false;
    header_type* const _root = this->_M_allocate_header();
    node_type* const _n = this->_M_allocate_node();
    _root->append_node(_n);
    this->_nodes[_k] = _n;
    this->_M_update_final_headers(_root);
    return true;
}
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::add(const key_type& _k, const key_type& _target) -> bool {
    if (this->contains(_k) || !this->contains(_target)) return false;
    header_type* const _root = this->_M_final_header(this->_nodes.at(_target));
    node_type* const _n = this->_M_allocate_node();
    _root->append_node(_n);
    this->_nodes[_k] = _n;
    this->_M_update_final_headers(_root);
    return true;
}


/// protected implementation
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_assign(const self& _rhs) -> void {
    std::vector<key_type> _delegate_keys;
    auto index_of_key = [&](const key_type& _k) -> size_t {
        for (size_t _i = 0; _i != _delegate_keys.size(); ++_i) {
            if (_rhs.sibling(_k, _delegate_keys[_i])) return _i;
        }
        return _delegate_keys.size();
    };
    for (const auto& _i : _rhs._nodes) {
        const key_type& _k = _i.first;
        header_type* const _root = [this, &_delegate_keys](const size_t _idx) {
            if (_idx == _delegate_keys.size()) return this->_M_allocate_header();
            else return this->_M_final_header_const(this->_nodes.at(_delegate_keys.at(_idx)));
        } (index_of_key(_k));
        if (_root->size() == 0) {
            _delegate_keys.push_back(_k);
        }
        node_type* const _n = this->_M_allocate_node();
        _root->append_node(_n);
        this->_nodes[_k] = _n;
        this->_M_update_final_headers(_root);
    }
}

}

#endif // _ICY_DISJOINT_SET_HPP_