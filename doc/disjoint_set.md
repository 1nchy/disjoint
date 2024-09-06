# Disjoint Set

我们基于矮树 `short_tree` 的数据结构实现并查集（矮树是指子节点数量较多，高度较矮的树形结构），实现的关键，有下面三点：

1. 如何减少键值与节点存储的空间复杂度
2. 如何确定节点所在的矮树
3. 时间复杂度低的删除与归并操作

## 节点字典

并查集维护一个存储键和节点指针的字典是有必要的，如果我们不想自己实现一个可以自定义`_ExtractKey` 的字典。为减少冗余存储，**节点自身仅维护节点间信息，不存放键信息，且不依赖键信息**。

## 矮树字典/集合

如果我们把矮树的操作定义在 `short_tree` 结构中，那么存储各矮树的指针同样是有必要的（对于把操作定义在节点内的情况，我们后面将进行讨论，这正是我们最后选择的更优的方案），我们后面将推导出矮树字典的键应该是什么。

关于矮树字典的键，由于节点自身维护了父节点等信息，因此最初的方案是以根节点指针作为键。该方案在删除/归并矮树时，需要保留被操作树原有的键，并在操作结束后更新矮树字典信息。

## 矮树结构优化

要实现归并操作，意味着树必须有一个根节点，在归并操作中，该根节点将变为其他矮树的子节点。按照一般思路来说，数据被存储在矮树的每一个节点中，这对删除操作是很不利的——删除非叶节点要么更新所有子节点的父节点信息，要么更新节点字典信息——这违背了[减少冗余存储的约定](./disjoint_set.md#L9)。

这说明我们有必要分别处理叶节点 `node` 和非叶节点 `header`。

其中，叶节点以数据存储为主，非叶节点不存储数据，仅用于维护树的结构与节点间关系。

因此我们推理出矮树的数据结构应该是这样的：

~~~mermaid
graph TD;
A("header")-->B(("node"))
A-->C(("node"))
A-->D("header")
A-->E("header")
D-->F(("node"))
D-->G(("node"))
D-->H("header")
H-->I(("node"))
~~~

### node 节点

node 节点定义如下：

~~~cpp
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
    self* _left = nullptr; self* _right = nullptr;
    header_type* _header = nullptr;
public:
    value_type& value() { return _v; }
    const value_type& value() const { return _v; }
    void set_value(const value_type& _v) { this->_v = _v; }
private:
    value_type _v;
};
~~~

我们可以针对并查集不需要存储值得特点，特化一下 `node` 类型。

### header 节点

~~~cpp
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
private:
    self* _header = nullptr;
    self* _left = nullptr; self* _right = nullptr;
    self* _first = nullptr; self* _last = nullptr;
    node_type* _first_node = nullptr; node_type* _last_node = nullptr;
    size_t _node_count = 0ul;
};
~~~

unhook 函数用于将自己与父节点之间分离。

## 并查集结构优化

在对矮树的结构进行优化后，我们不再需要在并查集中维护矮树字典，取而代之的是 `header` 集合。`header` 既用于保存节点间信息，又参与节点运算，如删除、归并操作等。