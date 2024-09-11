#ifndef _ICY_DISJOINT_SET_HPP_
#define _ICY_DISJOINT_SET_HPP_

#include "short_tree.hpp"
#include <cassert>
#include <unordered_map>
#include <unordered_set>

namespace icy {

template <typename _Key, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Key>> struct disjoint_set;
template <typename _Value, typename _Alloc> struct disjoint_set_alloc;

/**
 * @brief disjoint set, a container for managing the set to which elements belongs
 * @tparam _Key type of key object
 * @tparam _Hash hashing function object type, defaults to std::hash<_Key>.
 * @tparam _Alloc allocator type, defaults to std::allocator<_Key>.
 * @implements implemented by hash table and short tree
*/
template <typename _Key, typename _Hash, typename _Alloc>
struct disjoint_set : public short_tree::alloc<void, _Alloc> {
    typedef short_tree::alloc<void, _Alloc> base;
    typedef disjoint_set<_Key, _Hash, _Alloc> self;
    using node_type = typename base::node_type;
    using header_type = typename base::header_type;

    typedef _Key key_type;

    // <key_type, node_type*>
    std::unordered_map<key_type, node_type*> _nodes;
    // <header*>
    std::unordered_set<header_type*> _headers;

public:
    disjoint_set() = default;
    disjoint_set(const self& _rhs);
    self& operator=(const self& _rhs);
    virtual ~disjoint_set();

    /**
     * @brief return whether the specific key in disjoint set
     * @param _k the specific key
     */
    bool contains(const key_type& _k) const { return _nodes.count(_k) != 0; }
    /**
     * @brief return the number of elements classification
     */
    size_t classification() const { return _headers.size(); }
    /**
     * @brief return the number of elements
     */
    size_t size() const { return _nodes.size(); }
    /**
     * @brief return whether the given 2 keys in the one classification
     * @param _x the given key
     * @param _y the given key
     */
    bool sibling(const key_type& _x, const key_type& _y) const;
    /**
     * @brief add the specific key to a new classification
     * @param _k the specific key
     * @return return false when the key is already in disjoint set or the key fails to be added
     */
    bool add(const key_type& _k);
    /**
     * @brief add the specific key to the classification, which contains the given contains key
     * @param _k the specific key
     * @param _target the given contains key
     * @return return false when the @c _target is not in disjoint set or the key fails to be added
     */
    bool add_to(const key_type& _k, const key_type& _target);
    /**
     * @brief delete the specific key
     * @param _k the specific key
     * @return return false when the key is not in disjoint set or the key fails to be deleted
     */
    bool del(const key_type& _k);
    /**
     * @brief merge 2 classifications, which contains the given 2 keys respectively
     * @param _x the given key
     * @param _y the given key
     * @return return false when the keys are not in disjoint set or the classifications fail to be merged
     */
    bool merge(const key_type& _x, const key_type& _y);
    /**
     * @brief return whether no element in the disjoint set
     */
    bool empty() const { return _nodes.empty(); }
    /**
     * @brief clear all keys and classifications
     */
    void clear();

// check function
    unsigned check() const;

private:
    header_type* _M_delegate(const key_type& _k) const;
    header_type* _M_delegate(node_type* const _n) const;
    void _M_assign(const self& _rhs);
    /**
     * @brief update headers information
    */
    void _M_update_headers(header_type* const _h);
    /**
     * @brief return the root header
     * @details compress _n
     */
    header_type* _M_final_header(node_type* const _n) const;
    /**
     * @brief return the root header
     * @details not compress _n
     */
    header_type* _M_final_header_const(node_type* const _n) const;
    /**
     * @brief remove empty headers (exclude roots)
     */
    void _M_remove_empty_headers(header_type* _h) const;
    void _M_deallocate_header_and_node_recursively(header_type* const _h) const;
};

template <typename _Key, typename _Hash, typename _Alloc>
disjoint_set<_Key, _Hash, _Alloc>::disjoint_set(const self& _rhs) { // todo
    _M_assign(_rhs);
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::operator=(const self& _rhs) -> self& { // todo
    if (&_rhs == this) return *this;
    clear();
    _M_assign(_rhs);
    return *this;
};
template <typename _Key, typename _Hash, typename _Alloc>
disjoint_set<_Key, _Hash, _Alloc>::~disjoint_set() {
    clear();
};

template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_delegate(const key_type& _k) const -> header_type* {
    if (!contains(_k)) return nullptr;
    return _M_final_header(_nodes.at(_k));
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_delegate(node_type* const _n) const -> header_type* {
    if (_n == nullptr) return nullptr;
    return _M_final_header(_n);
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::sibling(const key_type& _x, const key_type& _y) const -> bool {
    if (!contains(_x) || !contains(_y)) return false;
    if (_x == _y) return true;
    return _M_final_header(_nodes.at(_x)) == _M_final_header(_nodes.at(_y));
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::add(const key_type& _k) -> bool {
    if (contains(_k)) return false;
    header_type* const _root = this->_M_allocate_header();
    node_type* const _n = this->_M_allocate_node();
    _root->append_node(_n);
    _nodes[_k] = _n;
    _M_update_headers(_root);
    return true;
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::add_to(const key_type& _k, const key_type& _target) -> bool {
    if (contains(_target)) {
        if (sibling(_k, _target)) return true;
        del(_k);
        header_type* const _root = _M_final_header(_nodes.at(_target));
        node_type* const _n = this->_M_allocate_node();
        _root->append_node(_n);
        _nodes[_k] = _n;
        _M_update_headers(_root);
        return true;
    }
    else {
        if (_k != _target) return false;
        return add(_k);
    }
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::del(const key_type& _k) -> bool {
    if (!contains(_k)) return false;
    node_type* const _n = _nodes.at(_k);
    header_type* const _root = _M_final_header_const(_n);
    header_type* const _h = _n->unhook();
    _M_remove_empty_headers(_h);
    this->_M_deallocate_node(_n);
    _nodes.erase(_k);
    _M_update_headers(_root);
    return true;
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::merge(const key_type& _x, const key_type& _y) -> bool {
    if (!contains(_x) || !contains(_y)) return false;
    if (sibling(_x, _y)) return true;
    header_type* const _xr = _M_final_header(_nodes.at(_x));
    header_type* const _yr = _M_final_header(_nodes.at(_y));
    _xr->append_header(_yr);
    _M_update_headers(_xr);
    _headers.erase(_yr);
    // _M_update_headers(_yr);
    return true;
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::clear() -> void {
    _nodes.clear();
    for (header_type* _h : _headers) {
        _M_deallocate_header_and_node_recursively(_h);
    }
    _headers.clear();
};

/// protected implementation
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_assign(const self& _rhs) -> void { // todo
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
            else return _M_final_header_const(this->_nodes.at(_delegate_keys.at(_idx)));
        } (index_of_key(_k));
        node_type* const _n = this->_M_allocate_node();
        _root->append_node(_n);
        _nodes[_k] = _n;
        _M_update_headers(_root);
    }
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_update_headers(header_type* const _h) -> void {
    if (_h->size() == 0) {
        assert(_headers.contains(_h));
        _headers.erase(_h);
        this->_M_deallocate_header(_h);
        return;
    }
    else if (!_headers.contains(_h)) {
        _headers.insert(_h);
    }
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_final_header(node_type* const _n) const -> header_type* {
    header_type* _fh = _n->get();
    for (; _fh->get() != nullptr; _fh = _fh->get());
    if (_fh != _n->get()) {
        header_type* _h = _n->unhook();
        _fh->append_node(_n);
        _M_remove_empty_headers(_h);
    }
    return _fh;
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_final_header_const(node_type* const _n) const -> header_type* {
    header_type* _fh = _n->get();
    for (; _fh->get() != nullptr; _fh = _fh->get());
    return _fh;
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_remove_empty_headers(header_type* _h) const -> void {
    while (_h->get() != nullptr && _h->size() == 0) {
        header_type* _next = _h->unhook();
        this->_M_deallocate_header(_h);
        _h = _next;
    }
};
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_deallocate_header_and_node_recursively(header_type* const _h) const -> void {
    _h->forward_nodes([this](node_type* _i) {
        this->_M_deallocate_node(_i);
    });
    _h->forward_headers([this](header_type* _i) {
        this->_M_deallocate_header_and_node_recursively(_i);
    });
    this->_M_deallocate_header(_h);
};

/// check implementation
/**
 * @brief check the uf_table
 * @returns 0 : normal
 *   1 : null header pointer or empty header in %_headers
 *   2 : null node in %_nodes
 *   3 : node not in any header
 *   4 : count from _headers \eq _nodes
 *   100+ : error in header inside
*/
template <typename _Key, typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::check() const -> unsigned {
    size_t _count_from_headers = 0ul;
    for (auto* _i : _headers) {
        if (_i == nullptr || _i->size() == 0) return 1;
        if (const auto _ic = _i->check()) return 100 + _ic;
        _count_from_headers += _i->size();
    }
    if (_count_from_headers != _nodes.size()) return 4;
    for (auto _i = _nodes.cbegin(); _i != _nodes.cend(); ++_i) {
        if (_i->second == nullptr) return 2;
        auto* const _h = _M_final_header_const(_i->second);
        if (!_headers.contains(_h)) return 3;
    }
    return 0;
};

};

#endif // _ICY_DISJOINT_SET_HPP_