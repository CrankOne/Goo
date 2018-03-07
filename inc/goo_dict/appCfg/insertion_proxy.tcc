# ifndef H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H
# define H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

# include <stack>

# include "goo_exception.hpp"
# include "goo_dict/parameter.tcc"
# include "goo_dict/appCfg/traits.hpp"
# include "goo_dict/util/dpath.hpp"
# include "goo_dict/generic_dict.tcc"
# include "goo_dict/parameters/logic.hpp"

# if !defined(_Goo_m_DISABLE_DICTIONARIES) \
  && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)

namespace goo {
namespace dict {

class Configuration;  // fwd

template<typename KeyT> class InsertionProxy;


# if 0
/**@class AppConfParameter
 * @brief An abstract parameter is a base class for Goo's
 * dictionary entires.
 *
 * We're tending to follow here to POSIX convention:
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html#tag_12_02
 * extended with GNU long options:
 * https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html ,
 *
 * so the option-arguments are never optional
 * (see Guideline 7 of 12.2). That means one can not, for example, declare
 * an option `-c` with optional argument. Even if the `-c` option is a
 * logical flag the option-argument should either be necessarily presented,
 * or prohibited by declaration. In first case this option have to be
 * provided either as `-c true` or `-c false` (or in any equivalent
 * appropriate way for options of logic type). In the second case this option
 * can not receive an option-argument (and should be provided as `-c` or
 * omitted).
 *
 * The following subclasses are expected:
 *  - single-char keys (here referred as `shortcuts')   (flags or with values, multiple time)
 *  - long options                                      (flags or with values)
 *  - unnamed positional arguments
 *  - sections (dictionaries itself)
 *
 * Flag can not be required (required=false)
 *
 * ProgramOption can not be set (set=false)
 *
 * List-of-Structures has atomic=false, singular=true (TODO!)
 *
 * @ingroup appParameters
 */
# endif

/// Insertion proxy specialization for application configuration traits. Supports
/// Only textual sub-dictionaries (named here '[sub]section'). The bottom dictionary
/// is always a Configution instance where shortcut parameters and positional
/// parameter are indexed.
template<>
class InsertionProxy< String > : public AppConfNameIndex \
                                        ::BaseInsertionProxy< InsertableParameter > {
public:
    typedef InsertionProxy<String> Self;
    typedef AppConfNameIndex Subsection;
    typedef std::stack< std::pair<std::string, Subsection *> > InsertionTargetsStack;
    typedef AppConfTraits::VBase VBase;

private:
    Configuration * _root;
    InsertionTargetsStack _stack;
    aspects::ProgramOption * _latestInsertedRequired;  // TODO: set to null in ctr
protected:
    Subsection & _top();
    void _index_by_shortcut( char, VBase * );
public:
    InsertionProxy( Subsection & d, const char * name=nullptr );
    explicit InsertionProxy( Configuration * R );
    InsertionProxy( Configuration * r, InsertionTargetsStack & s );

    // /////////////////////////////////////////////////////////////////////////
    // Flags (options that explicitly do not expect argument)

    /// Inserts new flag (option that does not expect the argument) referenced by
    /// shortcut only.
    InsertionProxy<String>::

    /// Inserts new flag (option that does not expect the argument) referenced by
    /// name only.
    Self & flag( const char * name, const char * description );

    /// Inserts new flag (option that does not expect the argument) referenced by
    /// name or shortcut.
    Self & flag( char shortcut, const char * name, const char * description );

    // /////////////////////////////////////////////////////////////////////////
    // Ordinary parameters

    /// Insert new parameter with shortcut and name, no default value.
    template<typename PT> Self &
    p( char shortcut
     , const char * name
     , const char * description ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description, _top())
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF> >()
                                        , _alloc<aspects::CharShortcut>(shortcut)
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>()
                                        , _alloc<aspects::Array>(false)
                                        );
        if( nullptr != name && *name != '\0' ) {
            _insert_parameter(name, pPtr);
        }
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with shortcut only, no default value.
    template<typename PT> Self &
    p( char shortcut
     , const char * description ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>(shortcut)
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>()
                                        , _alloc<aspects::Array>(false)
                                        );
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with name only, no default value.
    template<typename PT> Self &
    p( const char * name
     , const char * description ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>()  // no shortcut
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>()
                                        , _alloc<aspects::Array>(false)
                                        );
        if( nullptr != name && *name != '\0' ) {
            _insert_parameter(name, pPtr);
        } else {
            emraise( badParameter, "Unable to insert parameter with empty name." );
        }
        return *this;
    }

    /// Insert new parameter with shortcut and name, with default value.
    template<typename PT> Self &
    p( char shortcut
     , const char * name
     , const char * description
     , const PT & dft ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>(shortcut)
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>(true)
                                        , _alloc<aspects::Array>(false)
                                        );
        pPtr->value( dft );
        if( nullptr != name && *name != '\0' ) {
            _insert_parameter(name, pPtr);
        }
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with name only, with default value.
    template<typename PT> Self &
    p( const char * name
     , const char * description
     , const PT & dft ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>()  // no shortcut
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>(true)
                                        , _alloc<aspects::Array>(false)
                                        );
        pPtr->value( dft );
        if( nullptr != name && *name != '\0' ) {
            _insert_parameter(name, pPtr);
        } else {
            emraise( badParameter, "Unable to insert parameter with empty name." );
        }
        return *this;
    }

    /// Insert new parameter with shortcut only, with default value.
    template<typename PT> Self &
    p( char shortcut
     , const char * description
     , const PT & dft ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                       , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                       , _alloc<aspects::CharShortcut>(shortcut)
                                       , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                       , _alloc<aspects::IsSet>()
                                       , _alloc<aspects::Array>(false)
                                       );
        pPtr->value( dft );
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    // /////////////////////////////////////////////////////////////////////////
    // Arrays

    /// Insert new parameter with shortcut and name, no default value.
    template<typename PT> Self &
    array( char shortcut
         , const char * name
         , const char * description ) {
        auto pPtr = _alloc_parameter< goo::dict::Array<PT> >( _alloc<aspects::Description>(description)
                                    , _alloc<aspects::TStringConvertible<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                    , _alloc<aspects::CharShortcut>(shortcut)
                                    , _latestInsertedRequired = _alloc<aspects::ImplicitValue<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                    , _alloc<aspects::IsSet>()
                                    , _alloc<aspects::Array>(true)
                                    );
        if( nullptr != name && *name != '\0' ) {
            _insert_parameter(name, pPtr);
        }
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with shortcut only, no default value.
    template<typename PT> Self &
    array( char shortcut
         , const char * description ) {
        auto pPtr = _alloc_parameter< goo::dict::Array<PT> >( _alloc<aspects::Description>(description)
                                    , _alloc<aspects::TStringConvertible<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                    , _alloc<aspects::CharShortcut>(shortcut)
                                    , _latestInsertedRequired = _alloc<aspects::ImplicitValue<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                    , _alloc<aspects::IsSet>()
                                    , _alloc<aspects::Array>(true)
                                    );
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with name only, no default value.
    template<typename PT> Self &
    array( const char * name
         , const char * description ) {
        auto pPtr = _alloc_parameter< goo::dict::Array<PT> >( _alloc<aspects::Description>(description)
                                    , _alloc<aspects::TStringConvertible<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                    , _alloc<aspects::CharShortcut>()  // no shortcut
                                    , _latestInsertedRequired = _alloc<aspects::ImplicitValue<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                    , _alloc<aspects::IsSet>()
                                    , _alloc<aspects::Array>(true)
                                    );
        if( nullptr != name && *name != '\0' ) {
            _insert_parameter(name, pPtr);
        } else {
            emraise( badParameter, "Unable to insert parameter with empty name." );
        }
        return *this;
    }

    /// Insert new parameter with shortcut and name, with default value.
    template<typename PT> Self &
    array( char shortcut
         , const char * name
         , const char * description
         , const std::initializer_list<PT> & dft ) {
        auto pPtr = _alloc_parameter< goo::dict::Array<PT> >( _alloc<aspects::Description>(description)
                                    , _alloc<aspects::TStringConvertible<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                    , _alloc<aspects::CharShortcut>(shortcut)
                                    , _latestInsertedRequired = _alloc<aspects::ImplicitValue<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                    , _alloc<aspects::IsSet>(true)
                                    , _alloc<aspects::Array>(true)
                                    );
        pPtr->assign( Array<PT>(dft) );
        pPtr->set_to_default(true);
        if( nullptr != name && *name != '\0' ) {
            _insert_parameter(name, pPtr);
        }
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with name only, with default value.
    template<typename PT> Self &
    array( const char * name
         , const char * description
         , const std::initializer_list<PT> & dft ) {
        auto pPtr = _alloc_parameter< goo::dict::Array<PT> >( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>()  // no shortcut
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>(true)
                                        , _alloc<aspects::Array>(true)
                                        );
        pPtr->assign( Array<PT>(dft) );
        pPtr->set_to_default(true);
        if( nullptr != name && *name != '\0' ) {
            _insert_parameter(name, pPtr);
        } else {
            emraise( badParameter, "Unable to insert parameter with empty name." );
        }
        return *this;
    }

    /// Insert new parameter with shortcut only, with default value.
    template<typename PT> Self &
    array( char shortcut
         , const char * description
         , const std::initializer_list<PT> & dft ) {
        auto pPtr = _alloc_parameter< goo::dict::Array<PT> >( _alloc<aspects::Description>(description)
                                       , _alloc<aspects::TStringConvertible<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                       , _alloc<aspects::CharShortcut>(shortcut)
                                       , _latestInsertedRequired = _alloc<aspects::ImplicitValue<goo::dict::Array<PT>, _Goo_m_VART_LIST_APP_CONF>>()
                                       , _alloc<aspects::IsSet>()
                                       , _alloc<aspects::Array>(true)
                                       );
        pPtr->assign( Array<PT>(dft) );
        pPtr->set_to_default(true);
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    // /////////////////////////////////////////////////////////////////////////
    // Subsection insertion methods

    static void emplace_subsection_copy( Subsection &
                                       , const char *
                                       , Subsection * );

    /// Inserts the new parameters section with given name and description.
    Self & bgn_sect( const char *
                   , const char * );

    /// Encloses insertion of the new parameters section. Provide section name
    /// to check yourself.
    Self & end_sect( const char * name=nullptr );

    // /////////////////////////////////////////////////////////////////////////
    // Modifiers

    /// Operates with shell parameter aspect. Sets the "requires value" flag.
    Self & required_argument() {
        if( !_latestInsertedRequired ) {
            emraise( badState, "No recent inserted parameters available known to"
                    " current insertion proxy instance." );
        }
        _latestInsertedRequired->set_required( true );
        return *this;
    }

    /// Operates with shell parameter aspect. Sets the implicit value.
    template<typename PT> Self & implicit( const PT & iv ) {
        aspects::ImplicitValue<PT> * p;
        if( !_latestInsertedRequired ) {
            emraise( badState, "No recent inserted parameters available known to"
                    " current insertion proxy instance." );
        }
        if( !(p = dynamic_cast<aspects::ImplicitValue<PT>*>(_latestInsertedRequired)) ) {
            emraise( badCast, "Type mismatch while setting implicit"
                    " parameter value. Consider explicit type specification"
                    " within .implicit<...>() invocation." );
        }
        p->set_implicit_value( iv );
        return *this;
    }
};

template<>
class InsertionProxy<char> : public AppConfTraits \
                                    ::template IndexBy<char>::Dictionary \
                                    ::BaseInsertionProxy< InsertableParameter > {
public:
    typedef AppConfTraits::template IndexBy<char>::Dictionary TheDictionary;

    InsertionProxy( TheDictionary & target );


    std::pair<typename AppConfTraits::template IndexBy<char>::DictValue::Value::iterator, bool>
            insert( char, AppConfTraits::VBase * );
    // ...
};

}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES) && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

