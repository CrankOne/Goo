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

/**@struct EvalStatus
 * @brief Processing status code.
 *
 * Each processor have to return a status code for encompassing
 * graph-traversal routine at the end of its invocation. This status code
 * may interrupt DAG traversal (data propagation). One has to distinguish
 * following cases of abortion:
 *      - interrupt DAG traversal; useful for look-up procedures, filtering or
 *      fault-tolerant errors. By "fault tolerance" we imply that processor is
 *      capable to recover after the error and be able to continue.
 *      - abort DAG processing; besides of fatal errors, is useful for global
 *      look-up procedure when all the parallel-running tasks have to be stopped
 *      (`reduce'-like application patterns).
 *
 * Default return code is `0' (integer), meaning that DAG traversal shall
 * continue.
 */
struct EvalStatus {
    /// Continue traversal.
    static constexpr int ok = 0;
    /// Abort DAG traversal: interrupt DAG propagation in current worker.
    static constexpr int skip = 1;
    /// Abort DAG processing: interrupt all the workers and set failure flag.
    static constexpr int done = 2;
    /// Abort DAG processing: interrupt all the workers, but do not set the
    /// failure flag.
    static constexpr int error = -1;
    int value;
    EvalStatus() {}
    EvalStatus(int v) : value(v) {}
    bool operator==(int cv) { return cv == value; }
};

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
            emraise( noSuchKey, "Unable to retrieve value."
                   " Port \"%s\" has not been declared."
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
    virtual EvalStatus _V_eval( ValuesMap & ) = 0;
public:
    EvalStatus eval( ValuesMap & vm ) {
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


