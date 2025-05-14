#include "main.hpp"

#include "short_tree.hpp"

template <typename _Tp> using alloc = icy::short_tree::alloc<_Tp, std::allocator<_Tp>>;
template <typename _Tp> using node = typename alloc<_Tp>::node_type;
template <typename _Tp> using header = typename alloc<_Tp>::header_type;

using T = unsigned;

int main(void) {
    alloc<T> _alloc;
    std::vector<header<T>*> _headers;
    std::vector<node<T>*> _nodes;
    _headers.push_back(_alloc._M_allocate_header());
    _headers.push_back(_alloc._M_allocate_header());
    _nodes.push_back(_alloc._M_allocate_node());
    _nodes.push_back(_alloc._M_allocate_node());
    _nodes.push_back(_alloc._M_allocate_node());
    _nodes.push_back(_alloc._M_allocate_node());
    _nodes.push_back(_alloc._M_allocate_node());
    _nodes.push_back(_alloc._M_allocate_node());
    _headers[0]->append_node(_nodes[0]);
    _headers[0]->append_node(_nodes[1]);
    _headers[0]->append_node(_nodes[2]);
    _headers[1]->append_node(_nodes[3]);
    _headers[1]->append_node(_nodes[4]);
    _headers[1]->append_node(_nodes[5]);
    EXPECT_EQ(_headers[0]->check(), 0);
    EXPECT_EQ(_headers[1]->check(), 0);
    _headers[0]->append_header(_headers[1]);
    EXPECT_EQ(_headers[0], _headers[1]->get());
    EXPECT_EQ(_headers[0]->check(), 0);
    EXPECT_EQ(_headers[0]->size(), 6);
    EXPECT_EQ(_headers[1]->size(), 3);
    _nodes[5]->unhook();
    EXPECT_EQ(_headers[1]->check(), 0);
    EXPECT_EQ(_headers[0]->size(), 5);
    EXPECT_EQ(_headers[1]->size(), 2);
    _headers[0]->append_node(_nodes[5]);
    EXPECT_EQ(_headers[0]->check(), 0);
    EXPECT_EQ(_headers[0]->size(), 6);
    _nodes[1]->unhook();
    _nodes[3]->unhook();
    _nodes[4]->unhook();
    _headers[1]->unhook();
    EXPECT_EQ(_headers[0]->check(), 0);
    EXPECT_EQ(_headers[0]->size(), 3);
    for (auto* _header : _headers) {
        _alloc._M_deallocate_header(_header);
    }
    for (auto* _node : _nodes) {
        _alloc._M_deallocate_node(_node);
    }
    return 0;
}