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
# include "goo_dict/app_conf_info.hpp"

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

/// Specialization for application configuration parameters. Each dictionary
/// and singular parameter have a description. Singular parameters may have
/// a shortcuts, while list entries are anonymous and have no additional info.
template<> struct IndexingTraits< iBaseValue, TValue
                         , pInfos::DescriptionInfo
                         , pInfos::RequiredParameterInfo
                         , pInfos::IsSetInfo
                         > {
    /// Keeps generic traits' types.
    typedef IndexingTraits<iBaseValue, TValue> BareTraits;
    /// Preserve library-defined container.
    template<typename KT, typename VT> using TIndex = BareTraits::TIndex<KT, VT>;
    /// Preserve library-defined container.
    template<typename KT, typename VT> using TPHash = BareTraits::TPHash<KT, VT>;
    /// Dictionary with string keys type keeps all the types of information.
    typedef GenericDictionary<std::string, iBaseValue, TValue
                             , pInfos::DescriptionInfo
                             , pInfos::RequiredParameterInfo
                             , pInfos::IsSetInfo
                             > Dictionary;
    /// Dictionary with integer keys keeps no auxilliary information.
    typedef GenericDictionary<ListIndex, iBaseValue, TValue> ListOfStructures;
};

typedef IndexingTraits< iBaseValue, TValue
                         , pInfos::DescriptionInfo
                         , pInfos::RequiredParameterInfo
                         , pInfos::IsSetInfo
                         > AppCfgTraits;

# if 0
template<typename T> class TInsertionProxyCommon;
template<typename T> class InsertionProxy;

class DictionaryParameter;

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
# endif

}  // namespace dict
/** @} */  // end of appParameters group
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

