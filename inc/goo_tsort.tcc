# ifndef H_TOPOLOGICAL_SORT_H
# define H_TOPOLOGICAL_SORT_H

/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

# include <forward_list>
# include <unordered_set>
# include <unordered_map>
# include <set>
# include <cassert>

# include "goo_exception.hpp"
# include "goo_mixins/iterable.tcc"

/// Type for DAG nodes composite descriptor. This type is supposed to keep both
/// the depth within graph and the coloring flags during depth-first search
/// algorithm.
# ifndef GOO_DAG_NODE_DESCRIPTOR_TYPE
#   define GOO_DAG_NODE_DESCRIPTOR_TYPE uint32_t
# endif

namespace goo {
namespace dag {

template<typename T>
struct Traits {
    typedef       T &  Ref;
    typedef const T &  CRef;
};
template<typename T> struct Traits<const T> : public Traits<T> {};

typedef GOO_DAG_NODE_DESCRIPTOR_TYPE DFSDescriptor;
constexpr DFSDescriptor temporary_mark = 0x1;
constexpr DFSDescriptor visited_mark = 0x2;
constexpr DFSDescriptor depth_max = ((~((DFSDescriptor)0)) >> 2);

class DAGNode;

typedef std::vector<std::unordered_set<DAGNode*> > Order;

void visit( DAGNode & n
          , std::vector< std::unordered_set<DAGNode*> > & l );

Order dfs(const std::unordered_set<DAGNode*> & s );

class DAGNode : public std::unordered_set<DAGNode *> {
private:
    /// Depth-first search descriptor mark.
    DFSDescriptor _f;
protected:
    DAGNode() : _f(0x0) {}
    bool is_marked() const { return _f; }
    bool has_temporary_mark() const { return (0x3 & _f) & temporary_mark; }
    void mark_as_temporary() { _f |= temporary_mark; }
    void clear_temporary() { _f &= ~temporary_mark; }
    bool has_visited_mark() const { return (0x3 & _f) & visited_mark; }
    void mark_as_visited() { _f |= visited_mark; }
    DFSDescriptor depth() const { return _f >> 2; }
    void set_depth(DFSDescriptor d) { assert(d <= depth_max); _f = (0x3 & _f) | (d << 2); }
public:
    virtual ~DAGNode() {}
    /// Sets the relation 'node nd depends on this node'.
    DAGNode & precedes( DAGNode & nd ) {
        this->insert( &nd );
        return *this;
    }
    /// Sets the relation 'this node depends on nd'.
    DAGNode & depends_on( DAGNode & nd ) {
        nd.precedes( *this );
        return *this;
    }
    /// Returns true if node is terminative (does not precedes to anything).
    bool is_terminative() const { return this->empty(); }
    /// Returns list of dependencies (to which this node has to precede).
    const std::unordered_set<DAGNode *> & dependencies() const { return *this; }
    /// After single visit() call one may need to manually re-set the
    /// DFS-properties descriptor prior to running some look-up procedures
    /// within the DAG again.
    void reset_dfs_descriptor() { _f = 0x0; }

    friend void visit( DAGNode & n
                     , std::vector< std::unordered_set<DAGNode*> > & l );
    friend std::vector<std::unordered_set<DAGNode*> > dfs(
                     const std::unordered_set<DAGNode*> & s );
};

/// Represents a DAG node with associated data. Contains set of dependencies
/// and reference to the data object.
template<typename T>
class Node : public DAGNode {
private:
    /// Data constness should be guaranteed along all the DAG routines.
    typename Traits<T>::Ref _data;
public:
    /// Default node constructor.
    Node( typename Traits<T>::Ref d ) : _data(d) {}
    /// Ctr for dereferred data association.
    Node() {}
    /// dtr: no heap operations.
    virtual ~Node(){}
    /// Data getter.
    typename Traits<T>::Ref data() { return _data; }
    /// Data const getter.
    typename Traits<T>::CRef data() const { return _data; }
    /// Data setter.
    void data(typename Traits<T>::Ref d) { _data = d; }
};  // class Node

}  // namespace goo::dag
}  // namespace goo

# endif  // H_TOPOLOGICAL_SORT_H

