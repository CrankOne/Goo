/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

# ifndef H_GOO_PARAMETERS_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_H

# include <map>
# include <unordered_map>
# include <vector>

# include <iostream>
# include <list>

# include "goo_dict/plural.hpp"
# include "goo_dict/parameters/los.hpp"
# include "goo_dict/parameter_singular.tcc"
# include "goo_utility.hpp"

namespace goo {

/**@defgroup appParameters Application Parameters
 * 
 * The Goo's Application Parameters implements an extensible facility for
 * declaring, parsing and merging a dictionary of parameters representing
 * a routine configuration. By mean "routine" here the applications
 * (utilities) should be implied, however the facility by itself allows
 * one to parameterize any function or a complex object that way.
 *
 * We're tending to follow to the POSIX convention:
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html#tag_12_02
 * extended with GNU long options:
 * https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html .
 * 
 * The Configuration class implements a wrapper around the standard
 * UNIX getopt_long() function which performs actual parsing procedure of
 * command-line arguments. The wrapper class instance represents a root node
 * in tree-like data structure called "Configuration".
 *
 * In order to build an actual configuration user's code should operate with
 * Configuration instance by the media of InsertionProxy class which provides
 * a set of special construction tools.
 *
 * Common definitions:
 *  - A *parameter* is a pair of *name* and *value*, the particular entry in
 *  *dictionary*. The name should be unique for each particular parameter
 *  inside a dictionary.
 *  - The parameter can also have a *shortcut*. Parameters with shortcut and
 *  without a name are allowed; in this case the shortcut should be
 *  considered as an unique identifier of parameter inside an owning
 *  configuration recursively. I.e. the short cut must be unique among all the
 *  tree defined by single `Configuration` class instance.
 *
 * Command-line arguments-specific definitions:
 *  - An *argument* is a token provided to utility by shell in form that is
 *  usually used in C++ applications (within argv[]).
 *  - An *option-name* is the utility argument uniquely defining the particular
 *  parameter name. They can be *short* or *long*. Short options consists
 *  of one letter and are prefixed with single hyphen (-) sign and long
 *  are prefixed with two hyphen characters (--) when provided from
 *  command-line.
 *  - The *option-name* for each particular parameter is formed by the following
 *  rules:
 *      - When the parameter is located in the root dictionary, it can be
 *      accessed by its shortcut or its name.
 *      - When the parameter is located in the child dictionary, the names
 *      of this dictionary will be stacked up forming a prefixes separated
 *      with a dot symbol (.) where outermost dictionary goes first and
 *      owning dictionary goes last, before own name of parameter.
 *      - If the parameter has a shortcut, it can be accessed directly with
 *      this shortcut.
 *  - An *option-argument* is a textual representation of value that is to be
 *  set of parameter.
 *  - A *flag* is an option that does not require an argument. According to POSIX
 *  convention a tri-state parameters are not supported, so logical parameter
 *  have to be either a flag either a logical option with required argument
 *  (see PArameter<bool> template specification for details).
 *
 * @{*/

namespace dict {

template<typename T> class TInsertionProxyCommon;
template<typename T> class InsertionProxy;

class DictionaryParameter;

/// Indexing traits defines, for given value-keeper base and supplementary info
/// type, the particular indexing features.
template< typename BVlT
        , class ... SuppInfoTs > struct IndexingTraits;

template< typename KeyT
        , typename BVlT
        , class ... SuppInfoTs > class GenericDictionary;

template< typename BVlT
        , class ... SuppInfoTs > struct IndexingTraits {
    /// Template container type used to store entries indexed by name or index.
    template<typename KT, typename VT> TIndex = std::unordered_map<KT, VT>;
    /// Template container type used to store supplementary information entries
    /// indexed by pointers of dictionary entries.
    template<typename KT, typename VT> TPHash = std::unordered_map<KT, VT>;

    typedef GenericDictionary<std::string, BVlT, SuppInfoTs...> Dictionary;
    typedef GenericDictionary<ListIndex, BVlT> ListOfStructures;
};

/// A dictionary entry representation. Keeps basic introspection info allowing
/// one quickly find out, whether the
template< typename BaseValueT
        , class ... SuppInfoTs >
struct DictEntry {
    typedef IndexingTraits<BaseValueT, SuppInfoTs...> Traits;
    union {
        typename Traits::Dictionary * toDict;
        typename Traits::ListOfStructures * toList;
        BaseValueT * toSngl;
    } pointer;
    enum : unsigned char {
        isSngl = 0x0,
        isList = 0x1,
        isDict = 0x2
    } code;
};

template< typename KeyT
        , typename BVlT
        , class ... SuppInfoTs >
class GenericDictionary : public typename IndexingTraits<BVlT, SuppInfoTs...>::TIndex<KeyT, DictEntry<BVlT, SuppInfoTs...> *>
                       , public typename IndexingTraits<BVlT, SuppInfoTs...>::TPHash<DictEntry<BVlT, SuppInfoTs...> *, SuppInfoTs>... {
public:
    typedef IndexingTraits<BVlT, SuppInfoTs...> Traits;
public:
    DictEntry<BVlT, SuppInfoTs...> * entry( const KeyT & key ) {
        auto it = std::unordered_map<KeyT, DictEntry<BVlT, SuppInfoTs...> *>::find( key );
        if( Traits::TIndex<KeyT, DictEntry<BVlT, SuppInfoTs...> *>::end() == it ) {
            emraise( notFound, "" );  // TODO <<
        }
        return it->second;
    }

    template<typename SuppInfoT>
        typename std::enable_if<stdE::is_one_of<SuppInfoT, SuppInfoTs...>::value, SuppInfoT * >::type
    info( const KeyT & key ) {
        auto ePtr = entry( key ) ;
        auto it = Traits::TPHash<DictEntry<BVlT, SuppInfoT> *, SuppInfoT>::find( ePtr );
        return it->second;
    }
};

/**@brief Parameters container with basic querying support.
 * @class Dictionary
 *
 * The basic container for parameter instances providing basic querying support.
 * Stores auxiliary indexes for efficient retrieval of a particular parameter
 * instance by its name or single-char shortcuts.
 *
 * Albeit this container is embedded into goo::dict facility, it does not yet
 * act as a standalone parameter and is used in subsequent class
 * DictionaryParameter for actual parameter dictionary representation. Its
 * intermediate functionality is used in AoS sequences as well.
 * */
class Dictionary : public mixins::iDuplicable< iBaseValue, Dictionary >
                , protected DictionaryIndex<std::string, iSingularParameter>
                , protected DictionaryIndex<std::string, DictionaryParameter, true>
                , protected DictionaryIndex<std::string, LoSParameter> {

    // TODO: https://en.wikibooks.org/wiki/More_C++_Idioms/Friendship_and_the_Attorney-Client
    friend class DictInsertionAttorney;

    virtual void acquire_parameter_ptr( iSingularParameter * );
    virtual void acquire_subsection_ptr( DictionaryParameter * );
    virtual void acquire_list_ptr( LoSParameter * );
public:
    /// We define std::string to be key type for dictionary here, but it is only
    /// for compatibility with other template routines --- user code may still
    /// reference parameters by single-char shortcut (of type char).
    typedef std::string Key;

    typedef DictionaryIndex<std::string, iSingularParameter> SingsByName;
    typedef DictionaryIndex<std::string, DictionaryParameter, true> DictionariesContainer;
    typedef DictionaryIndex<std::string, LoSParameter> ListsByName;
protected:
    virtual void * _V_data_addr() override {
        return &(static_cast<iDictionaryIndex<void, iBaseValue> *>(this)->container_ref());
    }

    virtual const void * _V_data_addr() const override {
        return &(static_cast<const iDictionaryIndex<void, iBaseValue> *>(this)->container_const_ref());
    }

    virtual const std::type_info & _V_target_type_info() const override {
        return typeid(Dictionary);
    }
public:
    /// Public copy ctr for virtual copy ctr.
    Dictionary( const Dictionary & );
    Dictionary() = default;
    ~Dictionary() override;

    /// Constructs a bound insertion proxy instance object.
    InsertionProxy<Dictionary> insertion_proxy();

    //template<typename T> const T & operator[](const std::string &) const;
    //template<typename T>       T & operator[](const std::string &)
    const iBaseValue & operator[](const std::string &) const;
    iBaseValue & operator[](const std::string &);
};  // class Dictionary

/// Interim class, an attorney for insertion proxy to access dictionary
/// insertion methods.
class DictInsertionAttorney {
    template<typename T> static const T * probe( const std::string & k, const Dictionary & d) {
        return d.DictionaryIndex<std::string, T>::item_ptr( k );
    }
    template<typename T> static T * probe( const std::string & k, Dictionary & d ) {
        const Dictionary & cd = d;
        return const_cast<T*>( probe<T>(k, cd) );
    }

    static void push_parameter( iSingularParameter *, Dictionary & );
    static void push_subsection( DictionaryParameter *, Dictionary & );
    static void push_list_parameter( LoSParameter *, Dictionary & );

    friend class TInsertionProxyCommon<Dictionary>;
};

}  // namespace dict
/** @} */  // end of appParameters group
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

