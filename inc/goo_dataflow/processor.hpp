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
class Storage;

class PortInfo {
public:
    /// I/O flags and size to be encoded in this type.
    typedef size_t Features_t;
private:
    constexpr static Features_t flag_inputPort = 0x1;
    constexpr static Features_t flag_outputPort = 0x2;

    const std::type_info * _typeInfoPtr;
    Features_t _features;
public:
    PortInfo( const std::type_info & ti
            , size_t size_
            , bool isI, bool isO );

    size_t data_size() const {
        return (_features & (~(flag_inputPort | flag_outputPort)) >> 2); }
    bool is_input() const  { return _features & flag_inputPort; }
    bool is_output() const { return _features & flag_outputPort; }
    const std::type_info & type() const { return *_typeInfoPtr; }
};

template<typename T>
struct Traits {
    static PortInfo type_info( bool isInput, bool isOutput ) {
        return PortInfo( typeid(T), sizeof(T), isInput, isOutput );
    }
};

/// Processing status code.
typedef uint8_t PSC;

/// Represents a single value entry within variables map.
class ValueEntry {
private:
    void * _data;
protected:
    ValueEntry( void * dataPtr ) : _data(dataPtr) {}
public:
    template<typename T> T as() {
        return *reinterpret_cast<T*>(_data);
    }

    friend class ValuesMap;
    friend class Storage;
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
    /// Emplaces value entry with given name. TODO: protect it from users code.
    void add_value_entry( const std::string &, ValueEntry );
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
    
    friend class Storage;
};

class iProcessor {
public:
    typedef std::unordered_map<std::string, PortInfo> Ports;
private:
    Ports _ports;
protected:
    virtual PSC _V_eval( ValuesMap & ) = 0;
public:
    PSC eval( ValuesMap & vm ) {
        return _V_eval(vm);
    }
    /// Creates new typed I/O port.
    template<typename T> void
    port( const std::string & portName
        , bool isI=true
        , bool isO=true ) {
        auto ir = _ports.emplace( portName
                                , Traits<T>::type_info( isI, isO ) );
        if( !ir.second ) {
            emraise(nonUniq, "Port named \"%s\" already exists in processor.")
        }
    }

    template<typename T> void
    in_port( const std::string & portName ) {
        port<T>( portName, true, false );
    }

    template<typename T> void
    out_port( const std::string & portName ) {
        port<T>( portName, false, true );
    }

    const Ports & ports() const { return _ports; }
};

}  // namespace dataflow
}  // namespace goo


