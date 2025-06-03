#ifndef _ICY_DISJOINT_HPP_
#define _ICY_DISJOINT_HPP_

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

namespace icy {

namespace {
template <typename _Tp> struct storage;
template <typename _Tp> struct storage {
public:
    using value_type = _Tp;
public:
    template <typename... _Args> storage(_Args&&... _args) : _v(std::forward<_Args>(_args)...) {}
    storage(const storage&) = default;
    virtual ~storage() = default;
public:
    inline auto value() -> value_type& { return _v; }
    inline auto value() const -> const value_type& { return _v; }
    inline auto set_value(const value_type& _v) -> void { this->_v = _v; }
private:
    value_type _v;
};
template <> struct storage<void> {
public:
    using value_type = void;
public:
    storage() = default;
    storage(const storage&) = default;
    virtual ~storage() = default;
};
}

namespace {

template <typename _Tp, typename _Alloc> struct alloc;

template <typename _Tp> struct node;
template <typename _Tp> struct header;

template <typename _Tp> struct node : public storage<_Tp> {
    using self = node<_Tp>;
    using base = storage<_Tp>;
    using value_type = _Tp;
    using header_type = header<_Tp>;
    template <typename... _Args> node(_Args&&... _args): base(std::forward<_Args>(_args)...) {}
    node(const self& _rhs) : base(_rhs) {}
    self& operator=(const self&) = delete;
    virtual ~node() = default;
    template <typename _T> friend struct header;
public:
    const header_type* get() const { return _header; }
    header_type* get() { return _header; }
    header_type* unhook();
private:
    self* _left = nullptr;
    self* _right = nullptr;
    header_type* _header = nullptr;
};
template <typename _Tp> struct header {
    using self = header<_Tp>;
    using node_type = node<_Tp>;
    header() = default;
    header(const self&) = default;
    self& operator=(const self&) = delete;
    ~header() = default;
    template <typename _T> friend struct node;
public:
    const self* get() const { return _header; }
    self* get() { return _header; }
    size_t size() const { return _node_count; }
    void append_node(node_type* _n);
    void append_header(self* _h);
    self* unhook();
    /**
     * @brief visit each header forward
     * @tparam _Handler [](header_type*){}
     */
    template <typename _Handler> void forward_headers(const _Handler& _hdr);
    /**
     * @brief visit each header backward
     * @tparam _Handler [](header_type*){}
     */
    template <typename _Handler> void backward_headers(const _Handler& _hdr);
    /**
     * @brief visit each node forward
     * @tparam _Handler [](node_type*){}
     */
    template <typename _Handler> void forward_nodes(const _Handler& _hdr);
    /**
     * @brief visit each node backward
     * @tparam _Handler [](node_type*){}
     */
    template <typename _Handler> void backward_nodes(const _Handler& _hdr);
    void check() const;
private:
    self* _header = nullptr;
    self* _left = nullptr;
    self* _right = nullptr;
    self* _first = nullptr;
    self* _last = nullptr;
    node_type* _first_node = nullptr;
    node_type* _last_node = nullptr;
    size_t _node_count = 0ul;
};

template <typename _Tp> auto node<_Tp>::unhook() -> header_type* {
    if (_left != nullptr) _left->_right = _right;
    else _header->_first_node = _right; // _header->_first == this
    if (_right != nullptr) _right->_left = _left;
    else _header->_last_node = _left; // _header->_last == this
    for (auto* _i = _header; _i != nullptr; _i = _i->_header) {
        --_i->_node_count;
    }
    header_type* _h = _header;
    _left = nullptr; _right = nullptr; _header = nullptr;
    return _h;
};
template <typename _Tp> auto header<_Tp>::unhook() -> self* {
    assert(_first == nullptr && _last == nullptr);
    assert(_first_node == nullptr && _last_node == nullptr);
    assert(_node_count == 0ul);
    if (_left != nullptr) _left->_right = _right;
    else _header->_first = _right; // _header->_first == this
    if (_right != nullptr) _right->_left = _left;
    else _header->_last = _left; // _header->_last == this
    self* _h = _header;
    _left = nullptr; _right = nullptr; _header = nullptr;
    return _h;
};
template <typename _Tp> auto header<_Tp>::append_node(node_type* _n) -> void {
    if (_first_node == nullptr) _first_node = _n;
    if (_last_node != nullptr) _last_node->_right = _n;
    _n->_left = _last_node;
    _last_node = _n;
    _n->_header = this;
    for (auto* _i = this; _i != nullptr; _i = _i->_header) {
        ++_i->_node_count;
    }
};
template <typename _Tp> auto header<_Tp>::append_header(self* _h) -> void {
    if (_first == nullptr) _first = _h;
    if (_last != nullptr) _last->_right = _h;
    _h->_left = _last;
    _last = _h;
    _h->_header = this;
    for (auto* _i = this; _i != nullptr; _i = _i->_header) {
        _i->_node_count += _h->_node_count;
    }
};

template <typename _Tp> template <typename _Handler> auto header<_Tp>::forward_headers(const _Handler& _hdr) -> void {
    for (self* _i = _first; _i != nullptr;) {
        auto* _prev = _i; _i = _i->_right;
        _hdr(_prev);
    }
}
template <typename _Tp> template <typename _Handler> auto header<_Tp>::backward_headers(const _Handler& _hdr) -> void {
    for (self* _i = _last; _i != nullptr;) {
        auto* _prev = _i; _i = _i->_left;
        _hdr(_prev);
    }
}
template <typename _Tp> template <typename _Handler> auto header<_Tp>::forward_nodes(const _Handler& _hdr) -> void {
    for (node_type* _i = _first_node; _i != nullptr;) {
        auto* _prev = _i; _i = _i->_right;
        _hdr(_prev);
    }
}
template <typename _Tp> template <typename _Handler> auto header<_Tp>::backward_nodes(const _Handler& _hdr) -> void {
    for (node_type* _i = _last_node; _i != nullptr;) {
        auto* _prev = _i; _i = _i->_left;
        _hdr(_prev);
    }
}

static constexpr inline const char* fatal_node_range = "_first_node ^ _last_node";
static constexpr inline const char* fatal_node_link = "invalid list<node>";
static constexpr inline const char* fatal_node_header = "\\exists(list<node>)._header != this";
static constexpr inline const char* fatal_header_range = "_first ^ _last";
static constexpr inline const char* fatal_header_link = "invalid list<header>";
static constexpr inline const char* fatal_header_header = "\\exists(list<header>)._header != this";
static constexpr inline const char* fatal_node_count = "\\sum(\\all(list<header>).size()) != size()";

template <typename _Tp> auto header<_Tp>::check() const -> void {
    size_t _count = 0ul;
    // check node
    if (_first_node == nullptr ^ _last_node == nullptr) throw std::logic_error(fatal_node_range);
    node_type* _prev_node = nullptr;
    for (auto* _i = _first_node; _i != nullptr; _prev_node = _i, _i = _i->_right) {
        if (_i->_left != _prev_node) throw std::logic_error(fatal_node_link);
        if (_i->_header != this) throw std::logic_error(fatal_node_header);
        ++_count;
    }
    // check sub-header
    if (_first == nullptr ^ _last == nullptr) throw std::logic_error(fatal_header_range);
    self* _prev_header = nullptr;
    for (auto* _i = _first; _i != nullptr; _prev_header = _i, _i = _i->_right) {
        if (_i->_left != _prev_header) throw std::logic_error(fatal_header_link);
        if (_i->_header != this) throw std::logic_error(fatal_header_header);
        _i->check();
        _count += _i->_node_count;
    }
    if (_count != _node_count) throw std::logic_error(fatal_node_count);
    return;
};

template <typename _Tp, typename _Alloc> struct alloc : public _Alloc {
    typedef node<_Tp> node_type;
    typedef header<_Tp> header_type;
    typedef typename node_type::value_type value_type;
    typedef _Alloc elt_allocator_type;
    typedef std::allocator_traits<elt_allocator_type> elt_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<node_type> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<header_type> header_allocator_type;
    typedef std::allocator_traits<header_allocator_type> header_alloc_traits;

    elt_allocator_type& _M_get_elt_allocator() { return *static_cast<elt_allocator_type*>(this); }
    const elt_allocator_type& _M_get_elt_allocator() const { return *static_cast<const elt_allocator_type*>(this); }
    node_allocator_type _M_get_node_allocator() const { return node_allocator_type(_M_get_elt_allocator()); }
    header_allocator_type _M_get_header_allocator() const { return header_allocator_type(_M_get_elt_allocator()); }

    template <typename... _Args> node_type* _M_allocate_node(_Args&&... _args) const {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        auto _ptr = node_alloc_traits::allocate(_node_alloc, 1);
        node_type* _p = std::addressof(*_ptr);
        node_alloc_traits::construct(_node_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_node(node_type* _p) const {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        node_alloc_traits::destroy(_node_alloc, _p);
        node_alloc_traits::deallocate(_node_alloc, _p, 1);
    }
    template <typename... _Args> header_type* _M_allocate_header(_Args&&... _args) const {
        header_allocator_type _header_alloc = _M_get_header_allocator();
        auto _ptr = header_alloc_traits::allocate(_header_alloc, 1);
        header_type* _p = std::addressof(*_ptr);
        header_alloc_traits::construct(_header_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_header(header_type* _p) const {
        header_allocator_type _header_alloc = _M_get_header_allocator();
        header_alloc_traits::destroy(_header_alloc, _p);
        header_alloc_traits::deallocate(_header_alloc, _p, 1);
    }
};

}

namespace {
template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
struct disjoint_base : public alloc<_Value, _Alloc> {
public:
    using base = alloc<_Value, _Alloc>;
    using self = disjoint_base<_Key, _Value, _Hash, _Alloc>;
    using node_type = typename base::node_type;
    using header_type = typename base::header_type;
    using key_type = _Key;
public:
    disjoint_base() = default;
    disjoint_base(const self& _rhs) : base(_rhs) {};
    virtual ~disjoint_base();
public:/**
     * @brief return whether the specific key in disjoint set
     * @param _k the specific key
     */
    auto contains(const key_type& _k) const -> bool { return _nodes.count(_k) != 0; }
    /**
     * @brief return the number of elements classification
     */
    auto classification() const -> size_t { return _final_headers.size(); }
    /**
     * @brief return the number of elements
     */
    auto size() const -> size_t { return _nodes.size(); }
    /**
     * @brief return the number of elements in the classification
     * @param _k the key
     */
    auto sibling(const key_type& _k) const -> size_t;
    /**
     * @brief return whether the given 2 keys in the one classification
     * @param _x the given key
     * @param _y the given key
     */
    auto sibling(const key_type& _x, const key_type& _y) const -> bool;
    /**
     * @brief delete the specific key
     * @param _k the specific key
     * @return return false when the key is not in disjoint set or the key fails to be deleted
     */
    auto del(const key_type& _k) -> bool;
    /**
     * @brief delete all elements in the same classification as the specific key
     * @param _k the specific key
     * @return return false when the key is not in disjoint set or the key fails to be deleted
     */
    auto del_all(const key_type& _k) -> bool;
    /**
     * @brief delete all elements in the classification, except the specific key
     * @param _k the specific key
     * @return return false when the key is not in disjoint set or the elements fail to be deleted
     */
    auto del_except(const key_type& _k) -> bool;
    /**
     * @brief make the specific key join a new classification
     * @param _k the specific key
     * @return return false when the key is not in disjoint set or the key fails to be deleted
     */
    auto join(const key_type& _k) -> bool;
    /**
     * @brief make the specific key join the classification, which contains the given key
     * @param _k the specific key
     * @param _target the given key
     * @return return false when the keys are not in disjoint set or the key fails to be deleted
     */
    auto join(const key_type& _k, const key_type& _target) -> bool;
    /**
     * @brief merge 2 classifications, which contains the given 2 keys respectively
     * @param _x the given key
     * @param _y the given key
     * @return return false when the keys are not in disjoint set or the classifications fail to be merged
     */
    auto merge(const key_type& _x, const key_type& _y) -> bool;
    /**
     * @brief return whether no element in the disjoint set
     */
    auto empty() const -> bool { return _nodes.empty(); }
    /**
     * @brief clear all keys and classifications
     */
    auto clear() -> void;

// check function
    auto check() const -> void;
protected:
    /**
     * @brief update final headers information
     * @param _h a final header
    */
    auto _M_update_final_headers(header_type* const _h) -> void;
    /**
     * @brief return the root header
     * @details compress _n
     */
    auto _M_final_header(node_type* const _n) const -> header_type*;
    /**
     * @brief return the root header
     * @details not compress _n
     */
    auto _M_final_header_const(node_type* const _n) const -> header_type*;
    /**
     * @brief remove empty headers from bottom to top, remain the final header
     */
    auto _M_remove_empty_headers_from_bottom_to_top(header_type* _h) const -> void;
    auto _M_deallocate_header_recursively(header_type* const _h) const -> void;
protected:
    std::unordered_map<key_type, node_type*, _Hash> _nodes;
    std::unordered_set<header_type*> _final_headers;
};

template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
disjoint_base<_Key, _Value, _Hash, _Alloc>::~disjoint_base() {
    clear();
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::sibling(const key_type& _k) const -> size_t {
    if (!contains(_k)) return 0;
    return _M_final_header(_nodes.at(_k))->size();
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::sibling(const key_type& _x, const key_type& _y) const -> bool {
    if (!contains(_x) || !contains(_y)) return false;
    if (_x == _y) return true;
    return _M_final_header(_nodes.at(_x)) == _M_final_header(_nodes.at(_y));
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::del(const key_type& _k) -> bool {
    if (!contains(_k)) return false;
    node_type* const _n = _nodes.at(_k);
    header_type* const _root = _M_final_header_const(_n);
    header_type* const _h = _n->unhook();
    _M_remove_empty_headers_from_bottom_to_top(_h);
    this->_M_deallocate_node(_n);
    _nodes.erase(_k);
    _M_update_final_headers(_root);
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::del_all(const key_type& _k) -> bool {
    if (!contains(_k)) return false;
    node_type* const _n = _nodes.at(_k);
    header_type* const _root = _M_final_header_const(_n);
    // erase all nodes and the header
    for (auto _i = _nodes.cbegin(); _i != _nodes.cend();) {
        node_type* const _node = _i->second;
        if (_M_final_header_const(_node) == _root) {
            _i = _nodes.erase(_i);
            this->_M_deallocate_node(_node);
            continue;
        }
        ++_i;
    }
    // all elements have been removed, and the information in `_root` is still retained, so remove it directly
    _final_headers.erase(_root);
    _M_deallocate_header_recursively(_root);
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::del_except(const key_type& _k) -> bool {
    if (!contains(_k)) return false;
    node_type* const _n = _nodes.at(_k);
    header_type* const _root = _M_final_header_const(_n);
    _n->unhook();
    // erase all nodes and the header
    for (auto _i = _nodes.cbegin(); _i != _nodes.cend();) {
        const key_type& _key = _i->first;
        node_type* const _node = _i->second;
        if (_key != _k && _M_final_header_const(_node) == _root) {
            assert(_n != _node);
            _i = _nodes.erase(_i);
            this->_M_deallocate_node(_node);
            continue;
        }
        ++_i;
    }
    // all elements have been removed, and the information in `_root` is still retained, so remove it directly
    _final_headers.erase(_root);
    _M_deallocate_header_recursively(_root);
    header_type* const _new_root = this->_M_allocate_header();
    _new_root->append_node(_n);
    _M_update_final_headers(_new_root);
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::join(const key_type& _k) -> bool {
    if (!contains(_k)) return false;
    node_type* const _n = _nodes.at(_k);
    header_type* const _root = _M_final_header_const(_n);
    header_type* const _h = _n->unhook();
    _M_remove_empty_headers_from_bottom_to_top(_h);
    _M_update_final_headers(_root);
    header_type* const _new_root = this->_M_allocate_header();
    _new_root->append_node(_n);
    _M_update_final_headers(_new_root);
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::join(const key_type& _k, const key_type& _target) -> bool {
    if (!contains(_k) || !contains(_target)) return false;
    if (sibling(_k, _target)) {
        return true;
    }
    node_type* const _n = _nodes.at(_k);
    header_type* const _root = _M_final_header_const(_n);
    header_type* const _h = _n->unhook();
    _M_remove_empty_headers_from_bottom_to_top(_h);
    _M_update_final_headers(_root);
    header_type* const _new_root = _M_final_header(_nodes.at(_target));
    _new_root->append_node(_n);
    _M_update_final_headers(_new_root);
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::merge(const key_type& _x, const key_type& _y) -> bool {
    if (!contains(_x) || !contains(_y)) return false;
    if (sibling(_x, _y)) return true;
    header_type* const _xr = _M_final_header(_nodes.at(_x));
    header_type* const _yr = _M_final_header(_nodes.at(_y));
    _xr->append_header(_yr);
    _M_update_final_headers(_xr);
    _M_update_final_headers(_yr);
    return true;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::clear() -> void {
    for (const auto& [_k, _n] : _nodes) {
        this->_M_deallocate_node(_n);
    }
    _nodes.clear();
    for (header_type* _h : _final_headers) {
        _M_deallocate_header_recursively(_h);
    }
    _final_headers.clear();
}


template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::_M_update_final_headers(header_type* const _h) -> void {
    if (_h->get() == nullptr) {
        if (_h->size() == 0) {
            assert(_final_headers.contains(_h));
            _final_headers.erase(_h);
            this->_M_deallocate_header(_h);
        }
        else if (!_final_headers.contains(_h)) {
            _final_headers.insert(_h);
        }
    }
    else { // not a final header, remove it
        _final_headers.erase(_h);
    }
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::_M_final_header(node_type* const _n) const -> header_type* {
    header_type* _fh = _n->get();
    for (; _fh->get() != nullptr; _fh = _fh->get());
    if (_fh != _n->get()) {
        header_type* _h = _n->unhook();
        _fh->append_node(_n);
        _M_remove_empty_headers_from_bottom_to_top(_h);
    }
    return _fh;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::_M_final_header_const(node_type* const _n) const -> header_type* {
    header_type* _fh = _n->get();
    for (; _fh->get() != nullptr; _fh = _fh->get());
    return _fh;
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::_M_remove_empty_headers_from_bottom_to_top(header_type* _h) const -> void {
    while (_h->get() != nullptr && _h->size() == 0) {
        header_type* _next = _h->unhook();
        this->_M_deallocate_header(_h);
        _h = _next;
    }
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::_M_deallocate_header_recursively(header_type* const _h) const -> void {
    _h->forward_headers([this](header_type* _i) {
        this->_M_deallocate_header_recursively(_i);
    });
    this->_M_deallocate_header(_h);
}

/// check implementation
namespace {
static constexpr inline const char* fatal_empty_header = "\\exists(_final_headers)?.empty()";
static constexpr inline const char* fatal_empty_node = "\\exists(_nodes) == nullptr";
static constexpr inline const char* fatal_node_in_header = "\\exists(_nodes) not in \\any(_final_headers)";
static constexpr inline const char* fatal_nodes_count = "_nodes.size() != \\sum(\\all(_final_headers).size())";
}
template <typename _Key, typename _Value, typename _Hash, typename _Alloc> auto
disjoint_base<_Key, _Value, _Hash, _Alloc>::check() const -> void {
    size_t _count_from_headers = 0ul;
    for (auto* _i : _final_headers) {
        if (_i == nullptr || _i->size() == 0) throw std::logic_error(fatal_empty_header);
        _i->check();
        _count_from_headers += _i->size();
    }
    if (_count_from_headers != _nodes.size()) throw std::logic_error(fatal_nodes_count);
    for (auto _i = _nodes.cbegin(); _i != _nodes.cend(); ++_i) {
        if (_i->second == nullptr) throw std::logic_error(fatal_empty_node);
        auto* const _h = _M_final_header_const(_i->second);
        if (!_final_headers.contains(_h)) throw std::logic_error(fatal_node_in_header);
    }
    return;
}
}

template <typename _Key, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Key>> struct disjoint_set;
template <typename _Key, typename _Value, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Key>> struct disjoint_map;

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

#endif // _ICY_DISJOINT_HPP_