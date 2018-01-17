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
# include "goo_dict/parameter_dict.hpp"
# include "goo_dict/dpath.hpp"

namespace goo {
namespace dict {

template<typename T>
struct InsertionTraits;

template<>
struct InsertionTraits<Dictionary> {
    typedef DictInsertionAttorney Attorney;

    typedef iSingularParameter SingularTarget;
    typedef DictionaryParameter StructureTarget;
    typedef LoSParameter ListTarget;
};

template<>
struct InsertionTraits<ListOfStructures> {
    typedef ListInsertionAttorney Attorney;

    typedef iBaseValue SingularTarget;
    typedef Dictionary StructureTarget;
    typedef ListOfStructures ListTarget;
};

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
    /// Anonymous list-of-structures type.
    typedef ListOfStructures LoS;
    /// Anonymous dictionary type.
    typedef Dictionary Dict;
    /// Named dictionary type.
    /// Wrapper type keeping pointers to the named and anonymous list or
    /// dictionary.
    class InsertionTarget {
    private:
        union {
                  LoS * _LoSPtr;
                 Dict * _dictPtr;
        };
        bool _isDict;
    public:
        InsertionTarget( Dict * dPtr )      : _dictPtr(dPtr),     _isDict(true) {}
        InsertionTarget( LoS * lPtr )       : _LoSPtr(lPtr),      _isDict(false) {}
        template<typename T> T & as();
    };
    /// Materialized path --- insertion targets stack type keeping history of
    /// insertions.
    typedef std::stack<InsertionTarget> InsertionTargetsStack;
    struct MaterializedPath {
        InsertionTargetsStack stack;
        iBaseValue * parameterPtr;
        MaterializedPath( InsertionTargetsStack s ) : \
                                        stack(s), parameterPtr(nullptr) {}
        MaterializedPath( InsertionTargetsStack s, iBaseValue * p ) : \
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
    template<typename T> T & _top_as() { return _stack.top().as<T>(); }
    /// Pops stack top.
    void _pop() { _stack.pop(); }
    /// Inserts dict to internal insertion targets stack top.
    void _push_dict( Dict * d ) { _stack.push( InsertionTarget(d) ); }
    /// Inserts dict to internal insertion targets stack top.
    void _push_list( LoS * l ) { _stack.push( InsertionTarget(l) ); }
    /// Returns const reference to insertion target stack.
    const InsertionTargetsStack & stack() const { return _stack; }
    /// Template recursive versatile items getter.
    template<typename T>
    static MaterializedPath _apply_path( InsertionTargetsStack & mpath
                                        , aux::DictPath & path
                                        , bool raise=true
                                        , const std::string extensionDescr="");
    # if 0
    {
        T & c = mpath.top().as<T>();
        Dict * newTopDict = nullptr;
        LoS * newTopLoS = nullptr;
        const bool emptyToken = path.isIndex ? 0 == path.id.index : !strlen( path.id.name )
             , lastToken = !( path.next )
             , isIndex = !(path.isIndex)
             ;
        if( lastToken ) {
            // Last token probably refers to parameter
            iSingularParameter * isp \
                = T::InsertionAttorney::probe<iSingularParameter>( path.key_for<T>(), c );
            // Otherwise we have to consider last token as key referencing to
            // a dictionary or a list instance.
            newTopDict = T::InsertionAttorney::probe<Dictionary>( path.key_for<T>(), c );
            newTopLoS = T::InsertionAttorney::probe<LoS>( path.key_for<T>(), c );
            uint32_t check = !isp ? 0x0 : 0x0
                         | !newTopDict ? 0x0 : 0x0
                         | !newTopLoS ? 0x0 : 0x0
                         ;
        }
    }
    # endif
public:
    /// Multi-purpose path processing routine performing translation of
    /// textual path string to insertion proxy referencing particular instance
    /// within the parameters dictionary or list.
    // ...
    //template<typename T>
    const InsertionTargetsStack & stack() const { return _stack; }
};  // class InsertionProxyBase

template<typename T>
class TInsertionProxyCommon : public InsertionProxyBase {
public:
    typedef InsertionTraits IT;
protected:
    void _put( IT::SingularTarget * ptr ) {
        IT::Attorney::push_parameter( ptr, _top_as<T>() );
    }
    void _put( IT::StructureTarget * ptr ) {
        IT::Attorney::push_subsection( ptr, _top_as<T>() );
    }
    void _put( IT::ListTarget * ptr ) {
        IT::Attorney::push_list_parameter( ptr, _top_as<T>() );
    }
    /// Constructs new insertion proxy insance.
    TInsertionProxyCommon( T * root ) : InsertionProxyBase(root) {}
    /// Protected ctr used by list insertion proxy.
    TInsertionProxyCommon( const std::stack<InsertionTarget> & los)
            : InsertionProxyBase(los) {}
};

template<typename T>
class InsertionProxy;

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
template<>
class InsertionProxy<Dictionary> : public TInsertionProxyCommon<Dictionary> {
    friend class Configuration;
    friend class DictionaryParameter;
    friend class LoDInsertionProxy;
public:
    typedef TInsertionProxyCommon<Dictionary> Parent;
private:
    iSingularParameter * _lastInsertedParameter;
protected:
    /// Constructs new insertion proxy insance.
    InsertionProxy( DictionaryParameter * );
    /// Protected ctr used by list insertion proxy.
    InsertionProxy( const InsertionTargetsStack & los)
                    : Parent(los)
                    , _lastInsertedParameter(nullptr) {}
public:
    /// Marks the last inserted parameter as mandatory one.
    InsertionProxy & required_argument();

    /// Copy ctr. Similar to default implementation generated by GCC.
    InsertionProxy( const DictInsertionProxy & o ) : Parent(o) {}

    /// Opens new section.
    InsertionProxy & bgn_sect( const char *, const char * );

    /// Closes most recent section and switches to its previous
    /// (or finalizes last). Argument is optional and useful for self-check.
    InsertionProxy & end_sect( const char * = nullptr );

    /// Used to "close" dictionary insertion within list parent.
    InsertionProxy<ListOfStructures> end_dict( const char * = nullptr );

    /// newName may be null if name has to be preserved.
    void insert_copy_of( const iSingularParameter &, const char * newName=nullptr );

    //
    // Parameter inserters

    // TODO: restrict method for
    template<typename ParameterT, class ... Types> DictInsertionProxy &
    p( Types ... args ) {
        _lastInsertedParameter = new InsertableParameter<ParameterT>( args ... );
        _lastInsertedParameter->_check_initial_validity();
        _put( _lastInsertedParameter );
        //_top_as<Dict>().acquire_parameter_ptr( _lastInsertedParameter );
        return *this;
    }

    template<class ... Types> DictInsertionProxy &
    flag( Types ... args ) {
        Parameter<bool> * newParameterPtr = new Parameter<bool>( args ... );
        newParameterPtr->reset_flag();
        newParameterPtr->_check_initial_validity();
        _put( _lastInsertedParameter = newParameterPtr );
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
        _put( _lastInsertedParameter =
                new Parameter<Array<ParameterT> >( dfts, shortcut, name, description ) );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( const char * name
         , const char * description
         , const std::initializer_list<ParameterT> & dfts ) {
        _put( _lastInsertedParameter =
                new Parameter<Array<ParameterT> >( dfts, name, description ) );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( char shortcut
         , const char * description
         , const std::initializer_list<ParameterT> & dfts ) {
        _put( _lastInsertedParameter =
                new Parameter<Array<ParameterT> >( dfts, shortcut, description ) );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( char shortcut
         , const char *name
         , const char *description ) {
        _put( _lastInsertedParameter =
                new Parameter<Array<ParameterT> >( shortcut, name, description ) );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( const char *name
         , const char *description ) {
        _put( _lastInsertedParameter =
                new Parameter<Array<ParameterT> >( name, description ) );
        return *this;
    }

    template<typename ParameterT> DictInsertionProxy &
    array( char shortcut
         , const char *description ) {
        _put( _lastInsertedParameter =
                new Parameter<Array<ParameterT> >( shortcut, description ) );
        return *this;
    }

    /// Inserts the named list of dictionaries parameter and returns its
    /// insertion proxy object.
    template<class ... Types>
    InsertionProxy<ListOfStructures> bgn_list( Types ... args ) {
        auto p = new LoSParameter( args ... );
        _put(p);
        _push_list(p);
        _lastInsertedParameter = nullptr;
        return nullptr;
    }

    InsertionProxy<ListOfStructures> end_dict() {
        _pop();
        return InsertionProxy<ListOfStructures>(stack());
    }
};  // class DictInsertionProxy

template<>
class InsertionProxy<ListOfStructures> : public TInsertionProxyCommon<ListOfStructures> {
    friend class Configuration;
    friend class DictionaryParameter;
    friend class DictInsertionProxy;
public:
    typedef TInsertionProxyCommon<ListOfStructures> Parent;
    typedef DictInsertionProxy::InsertionTarget InsertionTarget;
private:
    std::stack<InsertionTarget> _stack;
protected:
    InsertionProxy( ListOfStructures * root ) : Parent( root ) {}
    InsertionProxy( const InsertionTargetsStack & st ) : Parent( st ) {}
public:
    /// Closes the LoD and pops insertion targets stack when LoD was created
    /// within dictionary.
    InsertionProxy<Dictionary> end_list( const char * = nullptr );

    /// Inserts a dictionary and returns an insertion proxy for on it.
    InsertionProxy<Dictionary> bgn_dict();

    /// Insert (anonymous) element in list.
    template<typename T> InsertionProxy<ListOfStructures> & e( const T & v ) {
        auto p = new iTValue<T>( v );
        _put( p );
        return *this;
    }

    /// Used to "open" new list insertion within current list.
    InsertionProxy<ListOfStructures> bgn_sublist();

    /// Used to "close" list insertion within other list.
    InsertionProxy<ListOfStructures> end_sublist();
};

template<> inline InsertionProxyBase::Dict &
InsertionProxyBase::InsertionTarget::as<InsertionProxyBase::Dict>() {
    if( !_isDict ) {
        emraise( badCast, "Mismatched dict/list on insertion:"
            " targets stack's top is list, while dictionary context"
            " demanded." );
    }
    return *_dictPtr;
}

template<> inline InsertionProxyBase::LoS &
InsertionProxyBase::InsertionTarget::as<InsertionProxyBase::LoS>() {
    if( _isDict ) {
         emraise( badCast, "Mismatched dict/list on insertion:"
            " targets stack's top is dictionary, while list context"
            " demanded." );
    }
    return *_LoSPtr;
}

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

