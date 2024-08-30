#ifndef _ICY_DISJOINT_SET_HPP_
#define _ICY_DISJOINT_SET_HPP_

#include "short_tree.hpp"
// #include <hashtable.h>
#include <cassert>
#include <unordered_map>
// #include <unordered_set>

namespace icy {

template <typename _Key,typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Key>> struct disjoint_set;
template <typename _Value, typename _Alloc> struct disjoint_set_alloc;

template <typename _Value, typename _Alloc> struct disjoint_set_alloc : public _Alloc {
    typedef short_tree<_Value> tree_type;
    typedef typename tree_type::node_type node_type;
    typedef _Alloc elt_allocator_type;
    typedef std::allocator_traits<elt_allocator_type> elt_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<tree_type> tree_allocator_type;
    typedef std::allocator_traits<tree_allocator_type> tree_alloc_traits;

    elt_allocator_type& _M_get_elt_allocator() { return *static_cast<elt_allocator_type*>(this); }
    const elt_allocator_type& _M_get_elt_allocator() const { return *static_cast<const elt_allocator_type*>(this); }
    tree_allocator_type _M_get_tree_allocator() const { return tree_allocator_type(_M_get_elt_allocator()); }

    tree_type* _M_allocate_tree() {
        tree_allocator_type _tree_alloc = _M_get_tree_allocator();
        auto _ptr = tree_alloc_traits::allocate(_tree_alloc, 1);
        tree_type* _p = std::addressof(*_ptr);
        ::new(_p) tree_type();
        return _p;
    }
    tree_type* _M_allocate_tree(const tree_type& _x) {
        tree_allocator_type _tree_alloc = _M_get_tree_allocator();
        auto _ptr = tree_alloc_traits::allocate(_tree_alloc, 1);
        tree_type* _p = std::addressof(*_ptr);
        tree_alloc_traits::construct(_tree_alloc, _p, _x);
        return _p;
    }
    template <typename... _Args> tree_type* _M_allocate_tree(_Args&&... _args) {
        tree_allocator_type _tree_alloc = _M_get_tree_allocator();
        auto _ptr = tree_alloc_traits::allocate(_tree_alloc, 1);
        tree_type* _p = std::addressof(*_ptr);
        tree_alloc_traits::construct(_tree_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_tree(tree_type* const _p) {
        tree_allocator_type _tree_alloc = _M_get_tree_allocator();
        tree_alloc_traits::destroy(_tree_alloc, _p);
        tree_alloc_traits::deallocate(_tree_alloc, _p, 1);
    }
};

/**
 * @brief disjoint set, a container for managing the set to which elements belongs
 * @tparam _Key type of key object
 * @tparam _Hash hashing function object type, defaults to std::hash<_Key>.
 * @tparam _Alloc allocator type, defaults to std::allocator<_Key>.
 * @implements implemented by hash table and short tree
 * @details
 *   ELEMENTS:
 *   unordered_map<key_type, node_type*> _h: store node pointers for each key
 *   unordered_map<const node_type*, tree_type*> _sth: store each short tree
*/
template <typename _Key,typename _Hash, typename _Alloc>
struct disjoint_set : public disjoint_set_alloc<std::nullptr_t, _Alloc> {
    typedef disjoint_set_alloc<std::nullptr_t, _Alloc> base;
    typedef typename base::tree_type tree_type;
    typedef typename base::node_type node_type;
    typedef disjoint_set<_Key, _Hash, _Alloc> self;

    typedef _Key key_type;

    // <key_type, node_type*>
    std::unordered_map<key_type, node_type*> _h;
    // <header*, tree_type*>
    std::unordered_map<const node_type*, tree_type*> _sth;

public:
    typedef typename std::unordered_map<_Key, node_type*>::iterator iterator;
    typedef typename std::unordered_map<_Key, node_type*>::const_iterator const_iterator;

    disjoint_set() = default;
    disjoint_set(const self& _rhs);
    self& operator=(const self& _rhs);
    virtual ~disjoint_set() {}

    /**
     * @brief return whether the specific key in disjoint set
     * @param _k the specific key
     */
    bool existed(const key_type& _k) const { return _h.count(_k) != 0; }
    /**
     * @brief return the number of elements classification
     */
    size_t classification() const { return _sth.size(); }
    /**
     * @brief return the number of elements
     */
    size_t size() const { return _h.size(); }
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
     * @brief add the specific key to the classification, which contains the given existed key
     * @param _k the specific key
     * @param _existed_key the given existed key
     * @return return false when the @c _existed_key is not in disjoint set or the key fails to be added
     */
    bool add_to(const key_type& _k, const key_type& _existed_key);
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
    bool empty() const { return _h.empty(); }
    /**
     * @brief clear all keys and classifications
     */
    void clear();

// check function
    int check() const;
    // template <typename _K, typename _H, typename _A> friend
    // std::ostream& operator<<(std::ostream&, const disjoint_set<_K, _H, _A>&);
    // void demo(std::istream& _is = std::cin, std::ostream& _os = std::cout);

private:
    tree_type* _M_delegate(const key_type& _k) const;
    tree_type* _M_delegate(node_type* const _n) const;
    void _M_assign(const self& _rhs);
    /**
     * @brief update tree information in %_sth
     * @details it's recommended that storing the hash_code of a tree before its any operation.
    */
    void _M_update_tree_info(const node_type* _header, tree_type* const _t);
};

template <typename _Key,typename _Hash, typename _Alloc>
disjoint_set<_Key, _Hash, _Alloc>::disjoint_set(const self& _rhs) {
    _M_assign(_rhs);
};
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::operator=(const self& _rhs) -> self& {
    if (&_rhs == this) return *this;
    clear();
    _M_assign(_rhs);
    return *this;
};

template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_delegate(const key_type& _k) const -> tree_type* {
    if (!existed(_k)) return nullptr;
    return _M_delegate(_h.at(_k));
};
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_delegate(node_type* const _n) const -> tree_type* {
    if (_n == nullptr) return nullptr;
    return _sth.at(tree_type::query_header(_n));
};
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::sibling(const key_type& _x, const key_type& _y) const -> bool {
    if (!existed(_x) || !existed(_y)) return false;
    return tree_type::query_header(_h.at(_x)) == tree_type::query_header(_h.at(_y));
};
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::add(const key_type& _k) -> bool {
    if (existed(_k)) return false;
    tree_type* const _st = this->_M_allocate_tree();
    node_type* _n = _st->add(nullptr);
    _sth[_st->header()] = _st;
    _h[_k] = _n;
    return true;
};
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::add_to(const key_type& _k, const key_type& _existed_key) -> bool {
    if (!existed(_existed_key)) return false;
    del(_k);
    tree_type* const _st = _M_delegate(_existed_key);
    node_type* const _n = _st->add(nullptr);
    _h[_k] = _n;
    return true;
};
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::del(const key_type& _k) -> bool {
    if (!existed(_k)) return false;
    node_type* _n = _h.at(_k);
    tree_type* const _st = _M_delegate(_k);
    const auto* _oh = _st->header();
    _st->del(_n);
    _M_update_tree_info(_oh, _st);
    _h.erase(_k);
    return true;
};
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::merge(const key_type& _x, const key_type& _y) -> bool {
    if (!existed(_x) || !existed(_y)) return false;
    tree_type* const _xt = _M_delegate(_x);
    tree_type* const _yt = _M_delegate(_y);
    const auto* _xth = _xt->header();
    const auto* _yth = _yt->header();
    _xt->merge(*_yt);
    _M_update_tree_info(_xth, _xt);
    _M_update_tree_info(_yth, _yt);
    return true;
};
// template <typename _Key,typename _Hash, typename _Alloc> auto
// disjoint_set<_Key, _Hash, _Alloc>::elect(const key_type& _k) -> bool {
//     if (!existed(_k)) return false;
//     node_type* _n = *(_h.find(_k));
//     tree_type& _t = *_M_delegate(_n);
//     hash_code _oc = delegate_id(_t).second;
//     _h.erase(_k); _h.erase(_ExtKey()(_t.root()->val()));
//     _t.elect(_n);
//     _h.insert(_n); _h.insert(_t.root());
//     _M_update_tree_info(_oc, _t);
//     return true;
// };
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::clear() -> void {
    _h.clear();
    for (auto _i = _sth.begin(); _i != _sth.end(); ++_i) {
        (_i->second)->clear();
    }
    _sth.clear();
};

/// protected implementation
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_assign(const self& _rhs) -> void {
    std::vector<key_type> _treed_keys;
    auto index_of_key = [&](const key_type& _k) -> size_t {
        for (size_t _i = 0; _i != _treed_keys.size(); ++_i) {
            if (_rhs.sibling(_k, _treed_keys[_i])) return _i;
        }
        return _treed_keys.size();
    };
    for (const auto& _i : _rhs._h) {
        const key_type& _k = _i.first;
        auto _idx = _has_treed(_k);
        if (_idx == _treed_keys.size()) {
            auto* const _st = this->_M_allocate_tree();
            _sth[_st->header()] = _st;
            _h[_k] = _st->add(nullptr);
        }
        else {
            auto* const _st = _M_delegate(_treed_keys[_idx]);
            _h[_k] = _st->add(nullptr);
        }
    }
};
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::_M_update_tree_info(const node_type* _oh, tree_type* const _t) -> void {
    if (_t->empty()) {
        this->_M_deallocate_tree(_sth.at(_oh));
        _sth.erase(_oh);
        return;
    }
    const node_type* _r = _t->header();
    if (_r != _oh) {
        _sth.erase(_oh);
        _sth[_r] = _t;
    }
};

/// check implementation
/**
 * @brief check the uf_table
 * @returns 0 : normal
 *   1 : null tree pointer or empty tree in %_sth
 *   2 : null node in %_h
 *   3 : 
 *   4 : 
 *   100+ : error in short tree inside
*/
template <typename _Key,typename _Hash, typename _Alloc> auto
disjoint_set<_Key, _Hash, _Alloc>::check() const -> int {
    for (auto _i = _sth.cbegin(); _i != _sth.cend(); ++_i) {
        if (_i->second == nullptr || (_i->second)->empty()) return 1;
        int _tmp = _i->second->check();
        if (_tmp != 0) return 100 + _tmp;
    }
    // for (auto _i = _h.cbegin(); _i != _h.cend(); ++_i) {
    //     if (*_i == nullptr) return 2;
    // }
    return 0;
};

};

#endif // _ICY_DISJOINT_SET_HPP_