#ifndef _ICY_DISJOINT_MAP_HPP_
#define _ICY_DISJOINT_MAP_HPP_

#include "disjoint_base.hpp"
#include <cassert>
#include <unordered_map>
#include <unordered_set>

namespace icy {

template <typename _Key, typename _Value, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Key>> struct disjoint_map;

/**
 * @brief disjoint map, a container for managing the set to which elements belongs
 * @tparam _Key type of key object
 * @tparam _Value type of value object
 * @tparam _Hash hashing function object type, defaults to std::hash<_Key>.
 * @tparam _Alloc allocator type, defaults to std::allocator<_Key>.
 * @implements implemented by hash table and short tree
*/
template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
struct disjoint_map : public disjoint_base<_Key, _Value, _Hash, _Alloc> {
    using base = disjoint_base<_Key, _Value, _Hash, _Alloc>;
    using self = disjoint_map<_Key, _Value, _Hash, _Alloc>;
    using node_type = typename base::node_type;
    using header_type = typename base::header_type;
    using key_type = typename base::key_type;
    using mapped_type = _Value;
    using value_type = std::pair<const key_type, mapped_type>;
public:
    disjoint_map() = default;
    disjoint_map(std::initializer_list<std::initializer_list<value_type>>);
    disjoint_map(const self& _rhs);
    auto operator=(const self& _rhs) -> self&;
    virtual ~disjoint_map() = default;
public:
    auto operator==(const self& _rhs) const -> bool;
    auto operator!=(const self& _rhs) const -> bool;
    auto operator[](const key_type& _k) -> mapped_type&;
    auto operator[](const key_type& _k) const -> const mapped_type&;
public:
    /**
     * @brief add key-value pair to a new classification
     * @param _v key-value pair
     * @return return false when the key is already in disjoint set or the pair fails to be added
     */
    auto add(const value_type& _v) -> bool;
    /**
     * @brief add key-value pair to the classification, which contains the given key
     * @param _v key-value pair
     * @param _target the given key
     * @return return false when the @c _target is not in disjoint set or the pair fails to be added
     */
    auto add(const value_type& _v, const key_type& _target) -> bool;
    /**
     * @brief update the value associated with the specific key
     * @param _k the specific key
     * @param _m the value
     * @return return false when the key is not in disjoint set or the value fails to be updated
     */
    auto update(const key_type& _k, mapped_type&& _m) -> bool;
    
    /**
     * @brief return value according to the given key
     */
    auto at(const key_type& _k) -> mapped_type&;
    auto at(const key_type& _k) const -> const mapped_type&;
private:
    auto _M_assign(const self& _rhs) -> void;
};



template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
disjoint_map<_Key, _Value, _Hash, _Alloc>::disjoint_map(std::initializer_list<std::initializer_list<value_type>> _llv) {
    for (auto _i = _llv.begin(); _i != _llv.end(); ++_i) {
        if (_i->begin() != _i->end()) {
            add(*(_i->begin()));
        }
        for (auto _j = _i->begin(); _j != _i->end(); ++_j) {
            add(*_j, _i->begin()->first);
        }
    }
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
disjoint_map<_Key, _Value, _Hash, _Alloc>::disjoint_map(const self& _rhs) : base(_rhs) {
    _M_assign(_rhs);
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::operator=(const self& _rhs) -> self& {
    if (&_rhs == this) return *this;
    this->clear(); _M_assign(_rhs);
    return *this;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::operator==(const self& _rhs) const -> bool {
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
        if (_rhs.at(_k) != this->at(_k)) {
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
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::operator!=(const self& _rhs) const -> bool {
    return !this->operator==(_rhs);
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::operator[](const key_type& _k) -> mapped_type& {
    if (!this->contains(_k)) {
        header_type* const _root = this->_M_allocate_header();
        node_type* const _n = this->_M_allocate_node();
        _root->append_node(_n);
        this->_nodes[_k] = _n;
        this->_M_update_final_headers(_root);
    }
    return at(_k);
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::operator[](const key_type& _k) const -> const mapped_type& {
    return at(_k);
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::add(const value_type& _v) -> bool {
    const key_type& _k = _v.first;
    if (this->contains(_k)) return false;
    header_type* const _root = this->_M_allocate_header();
    node_type* const _n = this->_M_allocate_node(_v.second);
    _root->append_node(_n);
    this->_nodes[_k] = _n;
    this->_M_update_final_headers(_root);
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::add(const value_type& _v, const key_type& _target) -> bool {
    const key_type& _k = _v.first;
    if (this->contains(_k) || !this->contains(_target)) return false;
    header_type* const _root = this->_M_final_header(this->_nodes.at(_target));
    node_type* const _n = this->_M_allocate_node(_v.second);
    _root->append_node(_n);
    this->_nodes[_k] = _n;
    this->_M_update_final_headers(_root);
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::update(const key_type& _k, mapped_type&& _m) -> bool {
    if (!this->contains(_k)) { return false; }
    this->_nodes.at(_k)->set_value(std::move(_m));
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::at(const key_type& _k) -> mapped_type& {
    return this->_nodes.at(_k)->value();
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::at(const key_type& _k) const -> const mapped_type& {
    return this->_nodes.at(_k)->value();
}


template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_map<_Key, _Value, _Hash, _Alloc>::_M_assign(const self& _rhs) -> void {
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
        node_type* const _n = this->_M_allocate_node(_i.second->value());
        _root->append_node(_n);
        this->_nodes[_k] = _n;
        this->_M_update_final_headers(_root);
    }
}

}

#endif // _ICY_DISJOINT_MAP_HPP_