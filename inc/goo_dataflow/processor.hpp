# pragma once

# include <cstdint>
# include <string>
# include <cassert>

# include <unordered_map>
# include <list>

# include "goo_exception.hpp"
# include "goo_tsort.tcc"

namespace goo {
namespace dataflow {

class ValuesMap;
class iProcessor;
class Tier;
class Link;

struct ValueTypeInfo {
    const std::type_info * typeInfo;
    size_t typeSize;
};

template<typename T>
struct Traits {
    static ValueTypeInfo type_info() {
        return ValueTypeInfo{ &(typeid(T)), sizeof(T)};
    }
};

/// Processing status code.
typedef uint8_t PSC;

class ValueEntry {
protected:
    void * _data;
    Link * _link;
    size_t _dataSize;
public:
    template<typename T> T as() {
        //assert( typeid(T) == _link->from.type() );
        return *reinterpret_cast<T*>(_data);
    }

    friend class ::goo::dataflow::ValuesMap;
};

/// Represents the values set, that processor operates. Built by worker
/// procedure, based on slots declaration.
class ValuesMap {
public:
    typedef std::unordered_map<std::string, ValueEntry>::iterator Iterator;
private:
    std::unordered_map<std::string, ValueEntry> _values;
protected:
    template<typename T> Iterator
    _find(const std::string & vName) {
        auto it = _values.find(vName);
        if( _values.end() == it ) {
            emraise( noSuchKey, "Slot \"%s\" is not declared in processor."
                   , vName.c_str() );
        }
        # if 0
        if( it->second._slot->type() != typeid(T) ) {
            emraise( badCast, "Type mismatch for \"%s\": declared \"%s\","
                    " requested \"%s\".", vName.c_str()
                    , it->second._slot->type().name()
                    , typeid(T).name() );
        }
        # endif
        return it;
    }
public:
    template<typename T> void
    set( const std::string & vName
       , const T & value ) {
        auto it = _find<T>(vName);
        *reinterpret_cast<T*>(it->second._data) = value;
    }
    template<typename T> const T &
    get( const std::string & vName ) {
        auto it = _find<T>(vName);
        return *reinterpret_cast<T*>(it->second._data);
    }
    
    /// Emplaces value entry with given name. TODO: protect it from users code.
    void _add_value_entry( const std::string &, ValueEntry );
};

class iProcessor {
public:
    typedef std::unordered_map<std::string, ValueTypeInfo> Ports;
private:
    Ports _ports;
protected:
    virtual PSC _V_eval( ValuesMap & ) = 0;
public:
    PSC eval( ValuesMap & vm ) {
        return _V_eval(vm);
    }
    /// Creates new typed port.
    template<typename T> void port( const std::string & portName ) {
        auto ir = _ports.emplace( portName, Traits<T>::type_info() );
        if( !ir.second ) {
            emraise(nonUniq, "Port named \"%s\" already exists in processor.")
        }
    }

    const Ports & ports() const { return _ports; }
};

# if 0
// TODO: this class will be probably further extended much to support
// description of advanced mechanisms of data transfer (by references, etc).
struct Link {
private:
    typename iProcessor::Ports::const_iterator _from, _to;
public:
    Link( iProcessor::Ports::const_iterator from_
        , iProcessor::Ports::const_iterator to_ ) : _from(from_), _to(to_) {}

    /// Returns data size in bytes to be reserved in thread-local storage.
    virtual size_t data_size( Tier & tier, size_t nProc );
    //virtual void free( std::vector<uint8_t> &, Tier &, size_t ) = 0;
    //
    typename iProcessor::Ports::const_iterator from() const { return _from; }
    typename iProcessor::Ports::const_iterator to() const { return _to; }
};
# endif

}  // namespace dataflow
}  // namespace goo


