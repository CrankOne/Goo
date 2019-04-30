# include <cstdint>
# include <string>
# include <cassert>

# include <unordered_map>
# include <list>

# include "goo_exception.hpp"

# pragma once

namespace goo {
namespace dataflow {

class ValuesMap;

/// Processing status code.
typedef uint8_t PSC;

class Slot {
private:
    const std::string _name;
    const std::type_info * _typeInfo;
public:
    Slot( const std::string & nm
        , const std::type_info & ti ) : _name(nm)
                                      , _typeInfo(&ti) {}
    const std::string & name() const { return _name; }
    const std::type_info & type() const { return *_typeInfo; }
};

class ValueEntry {
protected:
    void * _data;
    Slot * _slot;
public:
    template<typename T> T as() {
        assert( typeid(T) == _slot->type() );
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
        if( it->second._slot->type() != typeid(T) ) {
            emraise( badCast, "Type mismatch for \"%s\": declared \"%s\","
                    " requested \"%s\".", vName.c_str()
                    , it->second._slot->type().name()
                    , typeid(T).name() );
        }
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
private:
    std::list<Slot *> _slots;
protected:
    virtual PSC _V_eval( ValuesMap & ) = 0;
public:
    PSC eval( ValuesMap & vm ) {
        return _V_eval(vm);
    }
    /// Creates new typed slot.
    template<typename T> void slot( const std::string & slotName ) {
        _slots.push_back(new Slot(slotName, typeid(T)));
    }

    const std::list<Slot *> & slots() const { return _slots; }
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


