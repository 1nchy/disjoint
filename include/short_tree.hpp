#ifndef _ICY_SHORT_TREE_HPP_
#define _ICY_SHORT_TREE_HPP_

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <memory>
#include <iostream>

namespace icy {

template <typename _Tp, typename _Alloc> struct short_tree_alloc;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> class short_tree;

namespace {

template <typename _Tp> struct short_tree_node;
template <typename _Tp> struct short_tree_header;

template <typename _Tp> struct short_tree_node {
    using self = short_tree_node<_Tp>;
    using value_type = _Tp;
    using header_type = short_tree_header<_Tp>;
    template <typename... _Args> short_tree_node(header_type* _h, _Args&&... _args): _header(_h), _v(std::forward<_Args>(_args)...) {}
    short_tree_node(const self&) = default;
    self& operator=(const self&) = delete;
    virtual ~short_tree_node() = default;
    template <typename _T, typename _A> friend class icy::short_tree;
    // template <typename _A> friend class icy::short_tree<_Tp, _A>;
    template <typename _T> friend struct short_tree_header;
public:
    const header_type* header() const { return _header; }
    void unhook();
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
template <> struct short_tree_node<void> {
    using self = short_tree_node<void>;
    using value_type = void;
    using header_type = short_tree_header<void>;
    short_tree_node(header_type* _h) : _header(_h) {}
    short_tree_node(const self&) = default;
    self& operator=(const self&) = delete;
    ~short_tree_node() = default;
    template <typename _T, typename _A> friend class icy::short_tree;
    // template <typename _A> friend class icy::short_tree<_Tp, _A>;
    template <typename _T> friend struct short_tree_header;
public:
    const header_type* header() const { return _header; }
    void unhook();
private:
    self* _left = nullptr;
    self* _right = nullptr;
    header_type* _header = nullptr;
};
template <typename _Tp> struct short_tree_header {
    using self = short_tree_header<_Tp>;
    using node_type = short_tree_node<_Tp>;
    short_tree_header() = default;
    short_tree_header(const self&) = default;
    self& operator=(const self&) = delete;
    ~short_tree_header() = default;
    template <typename _T, typename _A> friend class icy::short_tree;
    // template <typename _A> friend class icy::short_tree<_Tp, _A>;
    template <typename _T> friend struct short_tree_node;
    // template <> friend struct short_tree_node<void>;
public:
    const self* header() const { return _header; }
    void unhook();
    void append_node(node_type* _n);
    void append_header(self* _h);
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

template <typename _Tp> auto short_tree_node<_Tp>::unhook() -> void {
    if (_left != nullptr) _left->_right = _right;
    else _header->_first_node = _right; // _header->_first == this
    if (_right != nullptr) _right->_left = _left;
    else _header->_last_node = _left; // _header->_last == this
    for (auto* _i = _header; _i != nullptr; _i = _i->_header) {
        --_i->_node_count;
        if (_i->_header->_header == _i) break;
    }
    _left = nullptr; _right = nullptr; _header = nullptr;
};
template <typename _Tp> auto short_tree_header<_Tp>::unhook() -> void {
    assert(_first == nullptr && _last == nullptr);
    assert(_first_node == nullptr && _last_node == nullptr);
    assert(_node_count == 0ul);
    if (_left != nullptr) _left->_right = _right;
    else _header->_first = _right; // _header->_first == this
    if (_right != nullptr) _right->_left = _left;
    else _header->_last = _left; // _header->_last == this
    _left = nullptr; _right = nullptr; _header = nullptr;
};
template <typename _Tp> auto short_tree_header<_Tp>::append_node(node_type* _n) -> void {
    if (_first_node == nullptr) _first_node = _n;
    if (_last_node != nullptr) _last_node->_right = _n;
    _n->_left = _last_node;
    _last_node = _n;
    _n->_header = this;
    for (auto* _i = this; _i != nullptr; _i = _i->_header) {
        ++_i->_node_count;
        if (_i->_header->_header == _i) break;
    }
};
template <typename _Tp> auto short_tree_header<_Tp>::append_header(self* _h) -> void {
    if (_first == nullptr) _first = _h;
    if (_last != nullptr) _last->_right = _h;
    _h->_left = _last;
    _last = _h;
    _h->_header = this;
    for (auto* _i = this; _i != nullptr; _i = _i->_header) {
        _i->_node_count += _h->_node_count;
        if (_i->_header->_header == _i) break;
    }
};
auto short_tree_node<void>::unhook() -> void {
    if (_left != nullptr) _left->_right = _right;
    else _header->_first_node = _right; // _header->_first == this
    if (_right != nullptr) _right->_left = _left;
    else _header->_last_node = _left; // _header->_last == this
    for (auto* _i = _header; _i != nullptr; _i = _i->_header) {
        --_i->_node_count;
        if (_i->_header->_header == _i) break;
    }
    _left = nullptr; _right = nullptr; _header = nullptr;
}
}

template <typename _Tp, typename _Alloc> struct short_tree_alloc : public _Alloc {
    typedef short_tree_node<_Tp> node_type;
    typedef short_tree_header<_Tp> header_type;
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

    template <typename... _Args> node_type* _M_allocate_node(_Args&&... _args) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        auto _ptr = node_alloc_traits::allocate(_node_alloc, 1);
        node_type* _p = std::addressof(*_ptr);
        node_alloc_traits::construct(_node_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_node(node_type* _p) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        node_alloc_traits::destroy(_node_alloc, _p);
        node_alloc_traits::deallocate(_node_alloc, _p, 1);
    }
    template <typename... _Args> header_type* _M_allocate_header(_Args&&... _args) {
        header_allocator_type _header_alloc = _M_get_header_allocator();
        auto _ptr = header_alloc_traits::allocate(_header_alloc, 1);
        header_type* _p = std::addressof(*_ptr);
        header_alloc_traits::construct(_header_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_header(header_type* _p) {
        header_allocator_type _header_alloc = _M_get_header_allocator();
        header_alloc_traits::destroy(_header_alloc, _p);
        header_alloc_traits::deallocate(_header_alloc, _p, 1);
    }
};

/**
 * @brief short_tree: decrease the height of the short_tree as possible, by compression, etc.
 * @details short tree memory model: ←→↑↓↙↖↗↘
 *   ----------------------------------
 *                       (_mark)
 *                         ↑ ↓
 *                      (_header)
 *          ↙         ↙         ↘       ↘
 *      (p1)←→(p2)←→(p3)       (_header)←→(_header)
 *                              ↙  ↙       ↘  ↘
 *                              .....         .....
 *   -----------------------------------
 * @implements
 *  - add node
 *  - del node
 *  - compress node
 *  - merge another short_tree %_r
 *   
*/
template <typename _Tp, typename _Alloc> class short_tree : public short_tree_alloc<_Tp, _Alloc> {
public:
    typedef short_tree<_Tp, _Alloc> self;
    typedef short_tree_alloc<_Tp, _Alloc> base;
    typedef typename base::elt_allocator_type elt_allocator_type;
    typedef typename base::elt_alloc_traits elt_alloc_traits;
    typedef typename base::node_allocator_type node_allocator_type;
    typedef typename base::node_alloc_traits node_alloc_traits;
    using node_type = typename base::node_type;
    using header_type = typename base::header_type;
    using value_type = typename node_type::value_type;
private:
    header_type _mark;
    // header_type* _header = nullptr;
public:
    short_tree();
    short_tree(const self& _rhs);
    self& operator=(const self& _rhs);
    virtual ~short_tree();

    size_t size() const { return header()->_node_count; }
    bool empty() const { return header()->_node_count == 0; }
    const header_type* header() const { return _mark.header(); }
    const header_type* mark() const { return &_mark; }
    template <typename... _Args> const node_type* add(_Args&&... _args);
    void del(const node_type* _x);
    void compress(const node_type* _x);
    void merge(self& _rhs);
    void clear();
    unsigned check() const;
    template <typename _NodeHandler> void traverse(const _NodeHandler& _handler) const { _M_traverse(header(), _handler); };
    static const header_type* query_header(const node_type* _x);
    static const header_type* query_mark(const node_type* _x);
protected:
    header_type* header() { return _mark._header; }
    void _M_hook_mark(header_type* const _h) { _mark._header = _h; _h->_header = &_mark; }
    void _M_unhook_mark() { _mark._header = nullptr; }
    /**
     * @brief assign short tree to this
     * @param _rhs short tree
    */
    void _M_assign(const self& _rhs);
    /**
     * @brief clone subtree to _p
     * @param _src the root of the subtree
     * @param _p parent of header
    */
    header_type* _M_clone_subtree(const header_type* const _src, header_type* _p);
    /**
     * @brief erase subtree under _s
     * @param _s the root of the subtree
     */
    void _M_erase_subtree(header_type* _s);

    void _M_add(node_type* _x);
    void _M_del(node_type* _x);
    void _M_compress(node_type* _x);
    template <typename _NodeHandler> void _M_traverse(const header_type* _x, const _NodeHandler& _handler) const;
    unsigned _M_check(const header_type* const _x, const header_type* const _p) const;
};
/// (de)constructor
template <typename _Tp, typename _Alloc>
short_tree<_Tp, _Alloc>::short_tree() {
    _M_hook_mark(this->_M_allocate_header());
}
template <typename _Tp, typename _Alloc>
short_tree<_Tp, _Alloc>::short_tree(const self& _rhs) {
    _M_assign(_rhs);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::operator=(const self& _rhs) -> self& {
    if (&_rhs != this) _M_assign(_rhs);
    return *this;
};
template <typename _Tp, typename _Alloc>
short_tree<_Tp, _Alloc>::~short_tree() {
    _M_erase_subtree(header());
    _M_unhook_mark();
};

/// protected implementation
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_assign(const self& _rhs) -> void {
    _M_erase_subtree(header());
    _M_hook_mark(_M_clone_subtree(_rhs.header(), nullptr));
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_clone_subtree(const header_type* const _src, header_type* _p)
 -> header_type* {
    if (_src == nullptr) return nullptr;
    header_type* _h = this->_M_allocate_header();
    if (_p != nullptr) _p->append_header(_h);
    for (const node_type* _i = _src->_first_node; _i != nullptr; _i = _i->_right) {
        node_type* const _n = _M_allocate_node(header(), *_i);
        _h->append_node(_n);
    }
    for (const header_type* _i = _src->_first; _i != nullptr; _i = _i->_right) {
        _M_clone_subtree(_i, _h);
    }
    return _h;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_erase_subtree(header_type* _s) -> void {
    if (_s == nullptr) return;
    for (auto* _i = _s->_first_node; _i != nullptr;) {
        auto* _prev = _i; _i = _i->_right;
        this->_M_deallocate_node(_prev);
    }
    _s->_first_node = nullptr; _s->_last_node = nullptr;
    for (auto* _i = _s->_first; _i != nullptr;) {
        auto* _prev = _i; _i = _i->_right;
        _M_erase_subtree(_prev);
    }
    _s->_first = nullptr; _s->_last = nullptr;
    this->_M_deallocate_header(_s);
};


template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_add(node_type* _x) -> void {
    header()->append_node(_x);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_del(node_type* _x) -> void {
    _x->unhook();
    this->_M_deallocate_node(_x);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_compress(node_type* _x) -> void {
    if (_x->header() == header()) return;
    _x->unhook();
    header()->append_node(_x);
};
template <typename _Tp, typename _Alloc> template <typename _NodeHandler> auto
short_tree<_Tp, _Alloc>::_M_traverse(const header_type* _x, const _NodeHandler& _handler) const -> void {
    if (_x == nullptr) return;
    for (const node_type* _i = _x->_first_node; _i != nullptr; _i = _i->_right) {
        _handler(_i);
    }
    for (const header_type* _i = _x->_first; _i != nullptr; _i = _i->_right) {
        _M_traverse(_i, _handler);
    }
};

/// public implementation
template <typename _Tp, typename _Alloc> template <typename... _Args> auto
short_tree<_Tp, _Alloc>::add(_Args&&... _args) -> const node_type* {
    node_type* _n = this->_M_allocate_node(header(), std::forward<_Args>(_args)...);
    _M_add(_n);
    return _n;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::del(const node_type* _x) -> void {
    _M_del(const_cast<node_type*>(_x));
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::compress(const node_type* _x) -> void {
    _M_compress(const_cast<node_type*>(_x));
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::merge(self& _rhs) -> void {
    header()->append_header(_rhs.header());
    _rhs._M_hook_mark(_rhs._M_allocate_header());
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::clear() -> void {
    _M_erase_subtree(header());
    _M_hook_mark(this->_M_allocate_header());
};

template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::query_header(const node_type* _x) -> const header_type* {
    if (_x == nullptr) return nullptr;
    const header_type* _p = _x->_header;
    assert(_p != nullptr && _p->_header != nullptr);
    while (_p->_header->_header != _p) {
        _p = _p->_header;
        assert(_p != nullptr && _p->_header != nullptr);
    }
    return _p;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::query_mark(const node_type* _x) -> const header_type* {
    if (_x == nullptr) return nullptr;
    const header_type* _p = _x->_header;
    assert(_p != nullptr && _p->_header != nullptr);
    while (_p->_header->_header != _p) {
        _p = _p->_header;
        assert(_p != nullptr && _p->_header != nullptr);
    }
    return _p->_header;
};

/// check implementation
/**
 * @brief check the short_tree
 * @returns 0 : normal
 *   1 : this->_header == nullptr
 *   2 : error in this->_header
 *   3 : the parent of node's children isn't itself
 *   4 : the number of nodes inequal %size()
 *   5 : error in header brothers' link
 *   6 : error in node brothers' link
*/
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::check() const -> unsigned {
    size_t _node_count_in_check = 0ul;
    // _header check
    if (header() == nullptr) return 1;
    if (header()->_header != &_mark) return 2;
    // recursively check
    return _M_check(header(), &_mark);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_check(const header_type* const _x, const header_type* const _p) const -> unsigned {
    if (_x->_header != _p) return 3;
    size_t _count = 0ul;
    for (const node_type* _i = _x->_first_node; _i != nullptr; _i = _i->_right) {
        ++_count;
    }
    for (const header_type* _i = _x->_first; _i != nullptr; _i = _i->_right) {
        _count += _i->_node_count;
    }
    if (_count != _x->_node_count)
        return 4;

    for (const header_type* _i = _x->_first; _i != nullptr; _i = _i->_right) {
        if (_i == _x->_first && _i->_left != nullptr) {
            return 5;
        }
        if (_i == _x->_last && _i->_right != nullptr) {
            return 5;
        }
        if (_i->_right != nullptr && _i->_right->_left != _i) {
            return 5;
        }
    }
    for (const node_type* _i = _x->_first_node; _i != nullptr; _i = _i->_right) {
        if (_i == _x->_first_node && _i->_left != nullptr) {
            return 6;
        }
        if (_i == _x->_last_node && _i->_right != nullptr) {
            return 6;
        }
        if (_i->_right != nullptr && _i->_right->_left != _i) {
            return 6;
        }
    }
    return 0;
};

};

#endif // _ICY_SHORT_TREE_HPP_