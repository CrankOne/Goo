# ifndef H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H
# define H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

# include <stack>

# include "goo_exception.hpp"
# include "goo_dict/parameter.tcc"
# include "goo_dict/dict.hpp"
# include "goo_dict/parameters/logic.hpp"
# include "goo_dict/parameters/integral.tcc"
# include "goo_dict/parameters/floating_point.tcc"
# include "goo_dict/parameters/string.hpp"
# include "goo_dict/parameters/los.hpp"

namespace goo {
namespace dict {

/**@brief Base class for insertion proxies.
 * @class InsertionProxyBase
 *
 * Insertion proxies provides a convinient way to describe your configuration
 * structures (parameters dictionaries and parameters lists) using lexical
 * chaining.
 *
 * This class holds common functions for both types of insertion proxies (for
 * dictionaries and lists).
 */
class InsertionProxyBase {
public:
    /// Named list-of-structures type.
    typedef iParameter<List<iStringConvertibleParameter*>> NamedLoS;
    /// Anonymous list-of-structures type.
    typedef iTValue<List<iStringConvertibleParameter*> > LoS;
    /// Named parameters dictionary type.
    typedef DictionaryParameter NamedDict;
    /// Anonymous dictionary type.
    typedef Dictionary Dict;
    /// Wrapper type keeping pointers to the named and anonymous list or
    /// dictionary.
    class InsertionTarget {
    private:
        union {
            NamedDict * _namedDPtr;
             NamedLoS * _namedLoSPtr;
                  LoS * _LoSPtr;
                 Dict * _dictPtr;
        };
        bool _isDict, _isNamed;
        void _assert_is( bool named, bool isDict, bool forceRequireNamed=true );
    public:
        InsertionTarget( NamedDict * dPtr ) : _namedDPtr(dPtr),   _isDict(true),  _isNamed(true)  {}
        InsertionTarget( Dict * dPtr )      : _dictPtr(dPtr),     _isDict(true),  _isNamed(false) {}
        InsertionTarget( NamedLoS * lPtr )  : _namedLoSPtr(lPtr), _isDict(false), _isNamed(true)  {}
        InsertionTarget( LoS * lPtr )       : _LoSPtr(lPtr),      _isDict(false), _isNamed(false) {}
        template<typename T> T & as(bool);
        /// Returns name, if target entity is of named type. If target has no
        /// name, returns dft string. Raises badState, if target is anonymous
        /// and dft is nullptr.
        std::string get_name( const char * dft=nullptr );
    };
    /// Materialized path --- insertion targets stack type keeping history of
    /// insertions.
    typedef std::stack<InsertionTarget> InsertionTargetsStack;
    struct MaterializedPath {
        InsertionTargetsStack stack;
        iStringConvertibleParameter * parameterPtr;
        MaterializedPath( InsertionTargetsStack s ) : \
                                        stack(s), parameterPtr(nullptr) {}
        MaterializedPath( InsertionTargetsStack s, iStringConvertibleParameter * p ) : \
                                        stack(s), parameterPtr(p) {}
    };
private:
    /// Insertion history for insertion proxy instance. Controls validity of
    /// chaining insertions.
    InsertionTargetsStack _stack;
protected:
    /// Ctr used by child classes
    InsertionProxyBase( const InsertionTargetsStack & st ) : _stack(st) {}
    /// Copy ctr.
    InsertionProxyBase( const InsertionProxyBase & o ) : _stack(o._stack) {}
    /// Ctr for dict-rooted inserters.
    InsertionProxyBase( DictionaryParameter * d ) {
        _stack.push( InsertionTarget(d) );
    }
    /// Ctr for list-rooted inserters.
    InsertionProxyBase( LoS * l ) {
        _stack.push( InsertionTarget(l) );
    }
    /// Returns stack top as reference to dictionary parameter. Raises
    /// assertFailed exception if top refers to list instance instead of dict.
    /// @param fn controls whether to emit exception if requested object is
    ///     anonymous while "named" type is required (see NamedLoS/LoS,
    ///     NamedDict/dict).
    template<typename T> T & _top_as(bool fn=true) { return _stack.top().as<T>(fn); }
    /// Pops stack top.
    void _pop() { _stack.pop(); }
    /// Inserts dict to internal insertion targets stack top.
    void _push_dict( DictionaryParameter * d ) { _stack.push( InsertionTarget(d) ); }
    /// Inserts dict to internal insertion targets stack top.
    void _push_list( LoS * l ) { _stack.push( InsertionTarget(l) ); }
    /// Returns const reference to insertion target stack.
    const InsertionTargetsStack & stack() const { return _stack; }
public:
    /// Multi-purpose path processing routine performing translation of
    /// textual path string to insertion proxy referencing particular instance
    /// within the parameters dictionary or list.
    static MaterializedPath combine_path( InsertionTargetsStack & mpath
                                        , char * path
                                        , bool extend=false
                                        , const std::string extensionDescr="");
};  // class InsertionProxyBase

class LoDInsertionProxy;

/**@brief Helper class for parameter insertion.
 *
 * The typical lifetime of is instances is following:
 *  1. Instance first is created with `Configuration` class `insertion_proxy()`
 *     method.
 *  2. Then parameters are added using this instance with various appropriate
 *     methods like `p()`, `flag()` and `list()`.
 *  3. Insertion proxy then may be safely deleted.
 *
 * Note that `bgn_sect()` and `end_sect()` methods have to be paired --- i.e.
 * each created sub-section must be closed.
 *
 * To prevent user code from possible mistakes, positional arguments
 * have to be added with own `Configuration` setter method
 * `positional_arguments()`.
 * */
class DictInsertionProxy : public InsertionProxyBase {
    friend class Configuration;
    friend class DictionaryParameter;
    friend class LoDInsertionProxy;
protected:
    /// Constructs new insertion proxy insance.
    DictInsertionProxy( DictionaryParameter * );
    /// Protected ctr used by list insertion proxy.
    DictInsertionProxy( const std::stack<InsertionTarget> & los) : InsertionProxyBase(los) {}
public:
    /// Marks the last inserted parameter as mandatory one.
    DictInsertionProxy & required_argument();
    /// Copy ctr. Similar to default implementation generated by GCC.
    DictInsertionProxy( const DictInsertionProxy & o ) : InsertionProxyBase(o) {}
    /// Opens new section.
    DictInsertionProxy & bgn_sect( const char *, const char * );

    /// Closes most recent section and switches to its previous
    /// (or finalizes last). Argument is optional and useful for self-check.
    DictInsertionProxy & end_sect( const char * = nullptr );

    /// Used to "close" dictionary insertion within list parent.
    LoDInsertionProxy end_dict( const char * = nullptr );

    /// newName may be null if name has to be preserved.
    void insert_copy_of( const iSingularParameter &, const char * newName=nullptr );

    //
    // Parameter inserters

    template<typename ParameterT, class ... Types> DictInsertionProxy &
    p( Types ... args ) {
        auto * p = new InsertableParameter<ParameterT>( args ... );
        p->_check_initial_validity();
        _top_as<Dict>(false).insert_parameter( p );
        return *this;
    }

    template<class ... Types> DictInsertionProxy &
    flag( Types ... args ) {
        Parameter<bool> * newParameterPtr = new Parameter<bool>( args ... );
        newParameterPtr->reset_flag();
        newParameterPtr->_check_initial_validity();
        _top_as<Dict>(false).insert_parameter( newParameterPtr );
        return *this;
    }

    //
    // Array inserters
    //

    template<typename ParameterT> DictInsertionProxy &
    array( char shortcut
         , const char * name
         , const char * description
         , const std::initializer_list<ParameterT> & dfts ) {
        _top_as<Dict>(false).insert_parameter(
                new Parameter<Array<ParameterT> >( dfts, shortcut, name, description )
            );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( const char * name
         , const char * description
         , const std::initializer_list<ParameterT> & dfts ) {
        _top_as<Dict>(false).insert_parameter(
                new Parameter<Array<ParameterT> >( dfts, name, description )
            );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( char shortcut
         , const char * description
         , const std::initializer_list<ParameterT> & dfts ) {
        _top_as<Dict>(false).insert_parameter(
                new Parameter<Array<ParameterT> >( dfts, shortcut, description )
            );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( char shortcut
         , const char *name
         , const char *description ) {
        _top_as<Dict>(false).insert_parameter(
                new Parameter<Array<ParameterT> >( shortcut, name, description )
            );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( const char *name
         , const char *description ) {
        _top_as<Dict>(false).insert_parameter(
                new Parameter<Array<ParameterT> >( name, description )
            );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( char shortcut
         , const char *description ) {
        _top_as<Dict>(false).insert_parameter(
                new Parameter<Array<ParameterT> >( shortcut, description )
            );
        return *this;
    }

    /// Declares the list of dictionaries parameter and returns its insertion
    /// proxy object.
    LoDInsertionProxy bgn_list( const char *
                              , const char * );

    //LoDInsertionProxy end_dict();
};  // class DictInsertionProxy

class LoDInsertionProxy : public InsertionProxyBase {
    friend class Configuration;
    friend class DictionaryParameter;
    friend class DictInsertionProxy;
public:
    typedef DictInsertionProxy::InsertionTarget InsertionTarget;
private:
    std::stack<InsertionTarget> _stack;
protected:
    LoDInsertionProxy( InsertionProxyBase::LoS * l ) : InsertionProxyBase(l) {}
    LoDInsertionProxy( const std::stack<InsertionTarget> & st ) : InsertionProxyBase(st) {}
public:
    /// Closes the LoD and pops insertion targets stack when LoD was created
    /// within dictionary.
    DictInsertionProxy end_list( const char * = nullptr );

    /// Inserts a dictionary and returns an insertion proxy for on it.
    DictInsertionProxy bgn_dict();

    /// Insert (anonymous) parameter in list.
    template<typename T> LoDInsertionProxy & v( const T & v ) {
        auto p = new iTStringConvertibleParameter<T>( v );
        _top_as<LoS>(false)._mutable_value().push_back( p );
        return *this;
    }

    /// Used to "open" new list insertion within current list.
    LoDInsertionProxy bgn_sublist();

    /// Used to "close" list insertion within other list.
    LoDInsertionProxy end_sublist();
};

template<> inline InsertionProxyBase::NamedDict &
InsertionProxyBase::InsertionTarget::as<InsertionProxyBase::NamedDict>(bool) {
    _assert_is( true, true, true );
    return *_namedDPtr;
}

template<> inline InsertionProxyBase::NamedLoS &
InsertionProxyBase::InsertionTarget::as<InsertionProxyBase::NamedLoS>(bool) {
    _assert_is( true, false, true );
    return *_namedLoSPtr;
}

template<> inline InsertionProxyBase::Dict &
InsertionProxyBase::InsertionTarget::as<InsertionProxyBase::Dict>(bool fn) {
    _assert_is( true, true, fn );
    return *(_isNamed ? static_cast<Dict*>( _namedDPtr ) : _dictPtr );
}

template<> inline InsertionProxyBase::LoS &
InsertionProxyBase::InsertionTarget::as<InsertionProxyBase::LoS>(bool fn) {
    _assert_is( true, false, fn );
    return *(_isNamed ? static_cast<LoS*>( _namedLoSPtr ) : _LoSPtr );
}

inline std::string
InsertionProxyBase::InsertionTarget::get_name( const char * dft ) {
    if( !_isNamed ) {
        if( !dft ) {
            emraise( badState, "Diagnostic getter failure: target is anonymous and"
                    " no default name given for insertion target %p.", this );
        }
        return dft;
    }
    if( _isDict ) {
        return _namedDPtr->name();
    }
    return _namedLoSPtr->name();
}

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

