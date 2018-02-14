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
    InsertionProxy( Subsection & d, const std::string & name="" );
    explicit InsertionProxy( Configuration * R );
    InsertionProxy( Configuration * r, InsertionTargetsStack & s );

    /// Inserts new flag (option that does not expect the argument) referenced by
    /// shortcut only.
    Self &
    flag( char shortcut, const std::string & description ) {
        assert(isalnum(shortcut));
        return flag( shortcut, "", description );
    }

    /// Inserts new flag (option that does not expect the argument) referenced by
    /// name only.
    Self &
    flag( const std::string & name, const std::string & description ) {
        assert(!name.empty());
        return flag( '\0', name, description );
    }

    /// Inserts new flag (option that does not expect the argument) referenced by
    /// name or shortcut.
    Self &
    flag( char shortcut, const std::string & name, const std::string & description ) {
        auto pPtr = _alloc_parameter<bool>( _alloc<aspects::Description>(description)
                                          , _alloc<aspects::TStringConvertible<bool, _Goo_m_VART_LIST_APP_CONF>>()
                                          , _alloc<aspects::CharShortcut>(shortcut)
                                          , _latestInsertedRequired = _alloc<aspects::ImplicitValue<bool, _Goo_m_VART_LIST_APP_CONF>>()
                                          , _alloc<aspects::IsSet>()
                                          , _alloc<aspects::Array>(false)
                                          );
        if( !name.empty() ) {
            _insert_parameter( name, pPtr );
        }
        if( '\0' != shortcut ) {
            assert( isalnum(shortcut) );
            _index_by_shortcut( shortcut, pPtr );
        }
        # ifndef NDEBUG
        else { assert( !name.empty() ); }
        # endif
        _latestInsertedRequired->set_required(false);
        _latestInsertedRequired->set_requires_argument(false);
        _latestInsertedRequired->set_being_implicit(true);
        static_cast<aspects::ImplicitValue<bool, _Goo_m_VART_LIST_APP_CONF>*>(_latestInsertedRequired)
                ->set_implicit_value(true);
        _latestInsertedRequired = nullptr;
        return *this;
    }

    /// Insert new parameter with shortcut and name, no default value.
    template<typename PT> Self &
    p( char shortcut
     , const std::string & name
     , const std::string & description ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>(shortcut)
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>()
                                        , _alloc<aspects::Array>(false)
                                        );
        _insert_parameter( name, pPtr );
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with shortcut only, no default value.
    template<typename PT> Self &
    p( char shortcut
     , const std::string & description ) {
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
    p( const std::string & name
     , const std::string & description ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>()  // no shortcut
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>()
                                        , _alloc<aspects::Array>(false)
                                        );
        _insert_parameter( name, pPtr );
        return *this;
    }

    /// Insert new parameter with shortcut and name, with default value.
    template<typename PT> Self &
    p( char shortcut
     , const std::string & name
     , const std::string & description
     , const PT & dft ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>(shortcut)
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>(true)
                                        , _alloc<aspects::Array>(false)
                                        );
        pPtr->value( dft );
        _insert_parameter( name, pPtr );
        _index_by_shortcut( shortcut, pPtr );
        return *this;
    }

    /// Insert new parameter with name only, with default value.
    template<typename PT> Self &
    p( const std::string & name
     , const std::string & description
     , const PT & dft ) {
        auto pPtr = _alloc_parameter<PT>( _alloc<aspects::Description>(description)
                                        , _alloc<aspects::TStringConvertible<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::CharShortcut>()  // no shortcut
                                        , _latestInsertedRequired = _alloc<aspects::ImplicitValue<PT, _Goo_m_VART_LIST_APP_CONF>>()
                                        , _alloc<aspects::IsSet>(true)
                                        , _alloc<aspects::Array>(false)
                                        );
        pPtr->value( dft );
        _insert_parameter( name, pPtr );
        return *this;
    }

    /// Insert new parameter with shortcut only, with default value.
    template<typename PT> Self &
    p( char shortcut
     , const std::string & description
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

    static void emplace_subsection_copy( Subsection &, const std::string &, Subsection * );

    /// Inserts the new parameters section with given name and description.
    Self & bgn_sect( const std::string &, const std::string & );

    /// Encloses insertion of the new parameters section. Provide section name
    /// to check yourself.
    Self & end_sect( const std::string & name="" );

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

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

