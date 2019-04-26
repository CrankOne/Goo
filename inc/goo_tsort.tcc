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

    friend void visit( DAGNode & n
                     , std::vector< std::unordered_set<DAGNode*> > & l );
    friend void dfs( std::unordered_set<DAGNode*> & s
                   , std::vector<std::unordered_set<DAGNode*> > & l );
};

/// Represents a DAG node with associated data. Contains set of dependencies
/// and reference to the data object.
template<typename T>
class Node : protected DAGNode {
private:
    /// Data constness should be guaranteed along all the DAG routines.
    typename Traits<T>::Ref _data;
public:
    /// Default node constructor.
    Node( typename Traits<T>::Ref d ) : _data(d) {}
    /// Ctr for dereferred data association.
    Node() {}
    /// Sets the relation 'node nd depends on this node'.
    Node & precedes( const Node<T> & nd ) {
        this->insert( &nd );
        return *this;
    }
    /// Sets the relation 'this node depends on nd'.
    const Node<T> & depends_on( Node<T> & nd ) const {
        nd->precedes( *this );
        return *this;
    }
    /// dtr: no heap operations.
    virtual ~Node(){}
    /// Data getter.
    typename Traits<T>::Ref data() { return _data; }
    /// Data const getter.
    typename Traits<T>::CRef data() const { return _data; }
    /// Returns true if node is terminative (does not precedes to anything).
    bool is_terminative() const { return this->empty(); }
    /// Returns list of dependencies (to which this node has to precede).
    const std::unordered_set<DAGNode *> & dependencies() const { return *this; }
};  // class Node

/**@class DAG
 * @brief Directed Acyclic Graph container.
 *
 * The template implements directed acyclic graph and some
 * routines utilizing it.
 * For details, see: https://en.wikipedia.org/wiki/Directed_acyclic_graph
 *
 * This DAG implementation is not so convinient as its labeled descendant,
 * but meets most of the strighforward needs (see LabeledDAG template).
 *
 * */
class DAG : public std::unordered_set<DAGNode*> {
public:
    DAG() {}
    virtual ~DAG( ) { }

    /// Deletes nodes.
    virtual void clear() {
        for( auto it = this->begin(); this->end() != it; ++it ) {
            delete (*it);
        }

    }

    # if 0
    /// Build a graph with reversed dependencies.
    DAG revert() const {
        DAG rg;
        // Place copy of nodes withoud pes.
        for( auto it = this->begin()
           ; this->end() != it
           ; ++it ) {
            rg.insert_data( (*it)->data_ptr() );
        }
        // Establish reverted connections.
        for( auto it = this->begin()
           ; this->end() != it
           ; ++it ) {
            Node<T> * cn = rg( (*it)->data_ptr() );
            const Node<T> * on = _dict.find((*it)->data_ptr())->second;
            for( auto depIt  = on->dependencies().begin();
                      on->dependencies().end() != depIt; ++depIt) {
                rg((*depIt)->data_ptr())->precedes( rg(cn->data_ptr()) );
            }
        }
        return rg;
    }
    # endif

    /// Provides a depth-first search.
    virtual size_t dfs( Order & ordered ) const {
        size_t nChains = 0,
               N = 0;  // overall processed
        while( N != this->size() ) {
            size_t nprocessed = 0;
            for( auto it = this->begin(); it != this->end(); ++it ) {
                if( Node<T>::discovered == (*it)->_status ) {
                    continue;  // pass visited or open node.
                }
                // start on terminal undiscovered node:
                nprocessed = (*it)->run_dfs_on_me( ordered );
                N += nprocessed;
                ++nChains;
            }
        }
        return nChains;
    }
};  // class DAG

# if 0
/**@class LabeledDAG
 * @brief Directed Acyclic graph with labeled nodes and associated data.
 *
 * Since the labeled DAG is a pretty common utility, we subclass our usual DAG
 * with it.
 *
 * This class offers insertion and depth-searching routines.
 *
 * Note, that our DAG conception implies only pointer management (no
 * data manipulation routines provided).
 * */
template<typename T>
class LabeledDAG : protected DAG<T> {
protected:
    class LabeledNode : public Node<T> {
    private:
        std::string _label;
        LabeledDAG<T> & _owner;
    public:
        Node( std::string label
            , const T * d
            , Self & set ) : Parent::Node(d)
                           , _label(label)
                           , _owner(set) {}

        Node & precedes( const std::string & depLbl ) {
            Parent::Node::precedes( &(_owner(depLbl)) );
            return *this;
        }
        Node & precedes( const Node * depNPtr ) {
            Parent::Node::precedes( depNPtr );
            return *this;
        }
        const std::string & label() const { return _label; }
    };
private:
    std::unordered_map<std::string, Node *> _byLabels;
public:
    LabeledDAG() : Parent() {}
    /// Get node from DAG by label.
    Node & get_node(const std::string & l ) {
        return *_byLabels[l];
    }
    /// Get node from DAG by label (operator).
    Node & operator()(const std::string & l ) {
        return get_node(l);
    }
    /// Get node by name.
    const Node & get_node(const std::string & l ) const {
        return *_byLabels[l];
    }
    /// Get node by name (operator).
    const Node & operator()(const std::string & l ) const {
        return get_node(l);
    }
    /// Deletes nodes.
    virtual void clear() {
        Parent::clear();
        _byLabels.clear();
    }
    /// Inserts node with given data ptr.
    virtual void insert( const std::string & label, const T * d ) {
        auto nn = new Node(label, d, *this);
        assert( Parent::_dictPtr );
        Parent::_dict[d] = nn;
        Parent::_nodesPtr->insert( nn );
        _byLabels[label] = nn;
    }
    /// Renders dependency graph in DOT notation. Useful for debug.
    void dump_DOT_notated_graph( std::ostream & os ) {
        os << "digraph Units {" << std::endl;
        for( auto it = _byLabels.begin(); it != _byLabels.end(); ++it ) {
            os << "    " << it->first << ";" << std::endl;
        }
        for( auto it = _byLabels.begin(); it != _byLabels.end(); ++it ) {
            for( auto depIt = (*it).second->dependencies().begin();
                      (*it).second->dependencies().end() != depIt; ++depIt ) {
                os << "    " << static_cast<Node*>(it->second)->label() << " -> "
                             << static_cast<Node*>(it->second)->label()
                   << ";" << std::endl;
            }
        }
        os << "}" << std::endl;
    }
    /// Provides a depth-first search.
    virtual size_t dfs( Order & ordered ) const {
        return Parent::dfs(ordered);
    }
    /// Obtain chain for node pointed out by particular label.
    virtual void chain_for_node_by_label( const std::string & label, Order & order ) {
        auto nodeIt = _byLabels.find(label);
        if( _byLabels.end() == nodeIt ) {
            emraise( notFound, "Has no node labeled as \"%s\".", label.c_str() );
        }
        Parent::chain_for_node( nodeIt->second->data_ptr(), order );
    }
    /// Returns list of all nodes.
    const std::unordered_map<std::string, Node *> & index() const {
        return _byLabels;
    }
};
# endif

}  // namespace goo::dag
}  // namespace goo

# endif  // H_TOPOLOGICAL_SORT_H

