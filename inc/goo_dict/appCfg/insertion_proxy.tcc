# ifndef H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H
# define H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

# include <stack>

# include "goo_exception.hpp"
# include "goo_dict/parameter.tcc"
# include "goo_dict/appCfg/traits.hpp"
# include "goo_dict/util/dpath.hpp"
# include "goo_dict/generic_dict.tcc"
# include "goo_dict/parameters/logic.hpp"

namespace goo {
namespace dict {

class Configuration;  // fwd

template<typename KeyT> class InsertionProxy;

/// Insertion proxy specialization for application configuration traits. Supports
/// Only textual sub-dictionaries (named here '[sub]section'). The bottom dictionary
/// is always a Configution instance where shortcut parameters and positional
/// parameter are indexed.
template<>
class InsertionProxy< std::string > : public AppConfTraits \
                                           ::template IndexBy<std::string>::Dictionary \
                                           ::BaseInsertionProxy< InsertableParameter > {
public:
    typedef InsertionProxy<std::string> Self;
    typedef AppConfNameIndex Subsection;
    typedef std::stack< std::pair<std::string, Subsection *> > InsertionTargetsStack;
    typedef AppConfTraits::VBase VBase;

private:
    Configuration * _root;
    InsertionTargetsStack _stack;
    aspects::Required * _latestInsertedRequired;  // TODO: set to null in ctr
protected:
    Subsection & _top();
    void _index_by_shortcut( char, VBase * );
public:
    InsertionProxy( Subsection & self );

    /// Insert new parameter with shortcut and name, no default value.
    template<typename PT> Self &
    p( char shortcut, const std::string & name, const std::string & description ) {
        // TODO: InsertibleParameter instead of Patameter<>
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT>>()
                                        , _alloc<aspects::CharShortcut>(shortcut)
                                        , _latestInsertedRequired = _alloc<aspects::Required>()
                                        , _alloc<aspects::IsSet>()
                                        , _alloc<aspects::Array>(false)
                                        );
        target()._insert_parameter( name, pPtr );
        _latestInsertedRequired = pPtr;
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with name only, no default value.
    template<typename PT> Self &
    p( const std::string & name, const std::string & description ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT>>()
                                        , _alloc<aspects::CharShortcut>()  // no shortcut
                                        , _latestInsertedRequired = _alloc<aspects::Required>()
                                        , _alloc<aspects::IsSet>()
                                        , _alloc<aspects::Array>(false)
                                        );
        target()._insert_parameter( name, pPtr );
        _latestInsertedRequired = pPtr;
        return *this;
    }

    /// Insert new parameter with shortcut and name, with default value.
    template<typename PT> Self &
    p( char shortcut
     , const std::string & name
     , const std::string & description
     , const PT & dft ) {
        auto pPtr = _alloc_parameter<PT>( dft
                                        , _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT>>()
                                        , _alloc<aspects::CharShortcut>(shortcut)
                                        , _latestInsertedRequired = _alloc<aspects::Required>()
                                        , _alloc<aspects::IsSet>(true)
                                        , _alloc<aspects::Array>(false)
                                        );
        target()._insert_parameter( name, pPtr );
        _latestInsertedRequired = pPtr;
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with name only, with default value.
    template<typename PT> Self &
    p( const std::string & name
     , const std::string & description
     , const PT & dft ) {
        auto pPtr = _alloc_parameter<PT>( dft
                                        , _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT>>()
                                        , _alloc<aspects::CharShortcut>()  // no shortcut
                                        , _latestInsertedRequired = _alloc<aspects::Required>()
                                        , _alloc<aspects::IsSet>(true)
                                        , _alloc<aspects::Array>(false)
                                        );
        target()._insert_parameter( name, pPtr );
        _latestInsertedRequired = pPtr;
        return *this;
    }

    /// Inserts the new parameters section with given name and description.
    Self & bgn_sect( const std::string &, const std::string & );

    /// Encloses insertion of the new parameters section. Provide section name
    /// to check yourself.
    Self & end_sect( const std::string & name="" );
};

typedef InsertionProxy<std::string> AppConfInsertionProxy;

template<> inline InsertionProxy<std::string>  // TODO: move it somewhere...
Traits<_Goo_m_VART_LIST_APP_CONF>::template IndexBy<std::string>::Aspect::insertion_proxy() {
    return InsertionProxy<std::string>(dynamic_cast<InsertionProxy<std::string>::Subsection &>(*this));
}

# if 0
/**@brief Base class for generic configuration insertion proxies.
 * @class GenericConfInsertionProxyBase
 *
 * This class provides goo::dict::
 */
template< typename KeyT>
class GenericConfInsertionProxyBase {
public:
    typedef Traits<BVlT, TVlT, SuppInfoTs...> Traits;
    /// Frequently used list-of-structures type shortcut.
    typedef typename Traits::ListOfStructures LoS;
    /// Frequently used dictionary type shortcut.
    typedef typename Traits::Dictionary Dict;
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
        InsertionTarget( Dict * dPtr ) : _dictPtr(dPtr), _isDict(true) {}
        InsertionTarget( LoS * lPtr )  : _LoSPtr(lPtr),  _isDict(false) {}
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
    typedef InsertionProxy< ListIndex
                    , BVlT
                    , TVlT
                    , ParameterT
                    , SuppInfoTs ...
                    > ListInsertionProxy;
    typedef InsertionProxy< std::string
                    , BVlT
                    , TVlT
                    , ParameterT
                    , SuppInfoTs ...
                    > DictInsertionProxy;
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
    InsertionProxyBase( Dict * d ) {
        _stack.push( InsertionTarget(d) );
    }
    /// Ctr for list-rooted inserters.
    InsertionProxyBase( LoS * l ) {
        _stack.push( InsertionTarget(l) );
    }
    /// Returns stack top as reference of required type. Raises
    /// assertFailed exception if top refers to list instance instead of dict.
    template<typename T> T & _top_as() { return _stack.top().template as<T>(); }
    // Returns stack bottom element (root) as a reference of required type.
    // Raises badCast if type cast fails.
    //template<typename T> T & _bottom_as() { return _stack.top().as<T>(); }
    /// Pops stack top.
    void _pop() { _stack.pop(); }
    /// Inserts dict to internal insertion targets stack top.
    void _push_dict( Dict * d ) { _stack.push( InsertionTarget(d) ); }
    /// Inserts dict to internal insertion targets stack top.
    void _push_list( LoS * l ) { _stack.push( InsertionTarget(l) ); }
    /// Returns const reference to insertion target stack.
    const InsertionTargetsStack & stack() const { return _stack; }
    /// Inserts item of any allowed type (dict, list, singular) with appropriate
    /// supp information into the top stack object.
    template<typename KeyT, typename ItemT, typename ... ArgsTs>
            void insert_item( typename IndexingKeyTraits<KeyT>::KeyHandle key, ItemT * e, ArgsTs ... suppInfos ) {
        _stack.top().template as< GenericDictionary<KeyT, BVlT, TVlT, SuppInfoTs...> >().acquire_item( key, e, suppInfos... );
    }

    # if 0
    /// Template recursive versatile items getter.
    template<typename T>
    static MaterializedPath _apply_path( InsertionTargetsStack & mpath
                                        , aux::DictPath & path
                                        , bool raise=true
                                        , const std::string extensionDescr="");
    # endif
};  // class InsertionProxyBase

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
template< typename BVlT
        , template <class> class TVlT
        , template <class> class ParameterT
        , template<typename> class ... SuppInfoTs >
class InsertionProxy< std::string
                    , BVlT
                    , TVlT
                    , ParameterT
                    , SuppInfoTs ...
                    > : public InsertionProxyBase< BVlT
                                                 , TVlT
                                                 , ParameterT
                                                 , SuppInfoTs...> {
    friend class InsertionProxy< ListIndex
                    , BVlT
                    , TVlT
                    , ParameterT
                    , SuppInfoTs ...
                    >;
public:
    //template<typename TheValueT> using Parameter = typename Traits::ParameterT<TheValueT>;
    typedef InsertionProxyBase< BVlT
                              , TVlT
                              , ParameterT
                              , SuppInfoTs...> Parent;
    typedef InsertionProxy< std::string
                          , BVlT
                          , TVlT
                          , ParameterT
                          , SuppInfoTs ...
                          > Self;
    typedef typename Parent::Traits Traits;
protected:
    /// Constructs new insertion proxy insance.
    InsertionProxy( DictionaryParameter * );
    /// Protected ctr used by list insertion proxy.
    InsertionProxy( const typename Parent::InsertionTargetsStack & los)
                  : Parent(los) {}
public:
    /// Copy ctr. Similar to default implementation generated by GCC.
    InsertionProxy( const Self & o ) : Parent( o.stack() ) {}

    template<typename T, typename ... CtrArgs> Self &
            p( const IndexingKeyTraits<std::string>::KeyHandle & key, CtrArgs ... args ) {
        ParameterT<T> p( args ... );
        Parent::template _top_as<Parent::Dict>().acquire_item( key
                                                            , p
                                                            , p.template as<SuppInfoTs>()... );
        return *this;
    }
};  // class InsertionProxy<Dictionary>

template< typename BVlT
        , template <class> class TVlT
        , template <class> class ParameterT
        , template<typename> class ... SuppInfoTs >
class InsertionProxy< ListIndex
                    , BVlT
                    , TVlT
                    , ParameterT
                    , SuppInfoTs ...
                    > : public InsertionProxyBase< BVlT
                                                 , TVlT
                                                 , ParameterT
                                                 , SuppInfoTs...> {
    friend class InsertionProxy< std::string
                               , BVlT
                               , TVlT
                               , ParameterT
                               , SuppInfoTs ...
                               >;
public:
    typedef InsertionProxyBase< BVlT
                              , TVlT
                              , ParameterT
                              , SuppInfoTs...> Parent;
    typedef InsertionProxy< ListIndex
                          , BVlT
                          , TVlT
                          , ParameterT
                          , SuppInfoTs ...
                          > Self;

protected:
    InsertionProxy( typename Parent::LoS * root ) : Parent( root ) {}
    InsertionProxy( const typename Parent::InsertionTargetsStack & st ) : Parent( st ) {}
public:
    # if 0
    /// Closes the LoD and pops insertion targets stack when LoD was created
    /// within dictionary.
    InsertionProxy<Parent::Dict> end_list( const char * = nullptr );

    /// Inserts a dictionary and returns an insertion proxy for on it.
    InsertionProxy<Pa> bgn_dict();

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
    # endif
};

# if 0
template<> inline InsertionProxyBase::Dict &
InsertionProxyBase::InsertionTarget::as<InsertionProxyBase::Dict>() {
    if( !_isDict ) {
        emraise( badCast, "Mismatched dict/list on insertion:"
            " targets stack's top is list, while dictionary context"
            " demanded." );
    }
    return *_dictPtr;
}

/// Specialization for goo::dict::Configuration downcast.
template<> inline Configuration &
InsertionProxyBase::InsertionTarget::as<Configuration>();

template<> inline InsertionProxyBase::LoS &
InsertionProxyBase::InsertionTarget::as<InsertionProxyBase::LoS>() {
    if( _isDict ) {
         emraise( badCast, "Mismatched dict/list on insertion:"
            " targets stack's top is dictionary, while list context"
            " demanded." );
    }
    return *_LoSPtr;
}

/// Inserts the named list of dictionaries parameter and returns its
/// insertion proxy object.
template<class ... Types> InsertionProxy<ListOfStructures>
InsertionProxy<Dictionary>::bgn_list( Types ... args ) {
    auto p = new LoSParameter( args ... );
    _put(p);
    _push_list(p);
    _lastInsertedParameter = nullptr;
    return nullptr;
}
# endif

# endif

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

