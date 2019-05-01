# include <cstdint>
# include <string>
# include <cassert>

# include <unordered_map>
# include <list>

# include "goo_exception.hpp"
# include "goo_tsort.tcc"

# pragma once

namespace goo {
namespace dataflow {

class ValuesMap;
class iProcessor;

/// Processing status code.
typedef uint8_t PSC;

// TODO: this class will be probably further extended much to support
// description of advanced mechanisms of data transfer (by references, etc).
struct Link {
    dag::Node<iProcessor> & from, & to;
    std::string fromName, toName;
    // ... TODO: other properties of a link instance
    void maps( const std::string & fn
             , const std::string & tn );
};

class ValueEntry {
protected:
    void * _data;
    Link * _link;
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
};

class iProcessor {
public:
    typedef std::unordered_map<std::string, const std::type_info *> Slots;
private:
    Slots _slots;
protected:
    virtual PSC _V_eval( ValuesMap & ) = 0;
public:
    PSC eval( ValuesMap & vm ) {
        return _V_eval(vm);
    }
    /// Creates new typed slot.
    template<typename T> void slot( const std::string & slotName ) {
        auto ir = _slots.emplace( slotName, &(typeid(T)) );
        if( !ir.second ) {
            emraise(nonUniq, "Slot named \"%s\" already exists in processor.")
        }
    }

    const Slots & slots() const { return _slots; }
};

# if 0
class Processor : public iProcessor {
public:
    Processor() {
        slot<int>("a");
        slot<float>("b");
        slot<MyEvent&>("event");
        slot<float>("sum");
    }
    virtual PSC eval( const Values & values ) {
        values["sum"].as<float>() = values["a"].as<int>()
                                  + values.get_as<float>("b");
        auto & e = args["event"].as<MyEvent&>();
        // ... do stuff with mutable & e.
        return 0;  // equivalent to goo::dataflow::proceed;
    }
};
# endif

}  // namespace dataflow
}  // namespace goo


