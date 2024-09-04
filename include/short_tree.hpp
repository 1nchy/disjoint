#ifndef _ICY_SHORT_TREE_HPP_
#define _ICY_SHORT_TREE_HPP_

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <memory>
#include <iostream>

namespace icy {

namespace short_tree {

template <typename _Tp, typename _Alloc> struct alloc;

template <typename _Tp> struct node;
template <typename _Tp> struct header;

template <typename _Tp> struct node {
    using self = node<_Tp>;
    using value_type = _Tp;
    using header_type = header<_Tp>;
    template <typename... _Args> node(_Args&&... _args): _v(std::forward<_Args>(_args)...) {}
    node(const self&) = default;
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
public:
    value_type& value() { return _v; }
    const value_type& value() const { return _v; }
    void set_value(const value_type& _v) { this->_v = _v; }
private:
    value_type _v;
};
template <> struct node<void> {
    using self = node<void>;
    using value_type = void;
    using header_type = header<void>;
    node() = default;
    node(const self&) = default;
    self& operator=(const self&) = delete;
    ~node() = default;
    // template <typename _T, typename _A> friend class icy::short_tree;
    // template <typename _A> friend class icy::short_tree<_Tp, _A>;
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
    // template <typename _T, typename _A> friend class icy::short_tree;
    // template <typename _A> friend class icy::short_tree<_Tp, _A>;
    template <typename _T> friend struct node;
    // template <> friend struct node<void>;
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
    unsigned check() const;
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
auto node<void>::unhook() -> header_type* {
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
}

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

/**
 * @brief check header
 * @returns
 *  0 : normal
 *  1 : error in _first_node or _last_node
 *  2 : error in node link
 *  3 : error in node::_header
 *  6 : error in _first or _last
 *  7 : error in header link
 *  8 : error in header::_header
 * 11 : error in _node_count
 */
template <typename _Tp> auto header<_Tp>::check() const -> unsigned {
    size_t _count = 0ul;
    // check node
    if (_first_node == nullptr ^ _last_node == nullptr) return 1u;
    node_type* _prev_node = nullptr;
    for (auto* _i = _first_node; _i != nullptr; _prev_node = _i, _i = _i->_right) {
        if (_i->_left != _prev_node) return 2u;
        if (_i->_header != this) return 3u;
        ++_count;
    }
    // check sub-header
    if (_first == nullptr ^ _last == nullptr) return 6u;
    self* _prev_header = nullptr;
    for (auto* _i = _first; _i != nullptr; _prev_header = _i, _i = _i->_right) {
        if (_i->_left != _prev_header) return 7u;
        if (_i->_header != this) return 8u;
        if (const auto _ic = _i->check()) return _ic;
        _count += _i->_node_count;
    }
    if (_count != _node_count) return 11u;
    return 0u;
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

};

};

#endif // _ICY_SHORT_TREE_HPP_