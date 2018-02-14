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

# include "goo_dict/util/dpath.hpp"
# include "goo_dict/common_aspects.hpp"
# include "goo_dict/util/subsections.tcc"

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
 *  (see Parameter<bool> template specification for details).
 *
 * @{*/

namespace dict {

template<typename KeyT> class InsertionProxy;

# define _Goo_m_VART_LIST_APP_CONF aspects::Description \
                                 , aspects::iStringConvertible \
                                 , aspects::CharShortcut \
                                 , aspects::Required \
                                 , aspects::IsSet \
                                 , aspects::Array

/// Specialization for common name-indexed dictionary structure used for
/// application configuration. Does not provide list-like structures.
template<>
struct Traits<_Goo_m_VART_LIST_APP_CONF> {
    typedef iAbstractValue VBase;
    typedef iBaseValue< _Goo_m_VART_LIST_APP_CONF > FeaturedBase;
    /// The default (generic, unspecified) IndexBy structure has not be used.
    /// For application configuration only two types of dictionaries are
    /// available: the string-indexed recurrent structure of subsections and
    /// the char-indexed one, bound directly to the Configuration instance, that
    /// does not keep any subsections.
    template<typename KeyT> struct IndexBy;
};

template<>
struct Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<std::string> {
    typedef GenericDictionary< std::string
                             , _Goo_m_VART_LIST_APP_CONF > Dictionary;

    typedef TValue< Hash< std::string
                        , iBaseValue<_Goo_m_VART_LIST_APP_CONF>*>
                  , aspects::Description > DictValue;

    /// In-place entry copying procedure. Takes the iterator to an existing
    /// mutable entry and pointer to owning dictionary. Copies the entry using
    /// dictionaries allocator overwriting the object the given iterator points
    /// to.
    static void copy_dict_entry( typename DictValue::Value::iterator
                               , Dictionary * );

    /// Aspect defines same-indexed subsections within the current
    /// dictionary.
    struct Aspect : public aux::iSubsections< std::string
                                            , Hash
                                            , GenericDictionary<std::string, _Goo_m_VART_LIST_APP_CONF> > {
        friend class InsertionProxy<std::string>;

        /// Constructs and returns insertion proxy referencing current
        /// dictionary instance. Defined in insertion_proxy.tcc
        virtual InsertionProxy<std::string> insertion_proxy();

        /// Returns a parameter entry by given path expression (const).
        virtual const FeaturedBase & operator[]( const std::string & path ) const {
            std::vector<char> namecache;
            return operator[]( utils::dpath( path, namecache ).front() );
        }

        /// Returns a parameter entry by given path expression (mutable).
        virtual FeaturedBase & operator[]( const std::string & path ) {
            std::vector<char> namecache;
            return operator[]( utils::dpath( path, namecache ).front() );
        }

        /// Returns a parameter entry by given path (const).
        virtual const FeaturedBase & operator[]( const utils::DictPath & dp ) const {
            if( dp.isIndex ) {
                emraise( badParameter, "Current path token is an integer index."
                    " Unable to dereference it within application configuration"
                    " context." );
            }
            if( dp.next ) {
                return subsection( dp.id.name )[*dp.next];
            }
            // that's a terminating path token:
            # ifdef NDEBUG
            return static_cast<const Subsection *>(this)->entry(dp.id.name);
            # else
            auto downCastedPtr = dynamic_cast<const Subsection *>(this);
            assert( downCastedPtr );  // failed, indicates broken inheritance
            return downCastedPtr->entry(dp.id.name);
            # endif
        }

        /// Returns a parameter entry by given path (const).
        virtual FeaturedBase & operator[]( const utils::DictPath & dp ) {
            if( dp.isIndex ) {
                emraise( badParameter, "Current path token is an integer index."
                    " Unable to dereference it within application configuration"
                    " context." );
            }
            if( dp.next ) {
                return subsection( dp.id.name )[*dp.next];
            }
            // that's a terminating path token:
            # ifdef NDEBUG
            return static_cast<Subsection *>(this)->entry(dp.id.name);
            # else
            auto downCastedPtr = dynamic_cast<Subsection *>(this);
            assert( downCastedPtr );  // failed, indicates broken inheritance
            return downCastedPtr->entry(dp.id.name);
            # endif
        }
    };
};


template<>
struct Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<char> {
    /// Char-indexed dictionary within the application configuration does not
    /// contain any sub-sections (within it's aspect).
    struct Aspect {
        friend class InsertionProxy<char>;

        /// Constructs an auxilliary insertion proxy object, specific for the
        /// application configuration dictionaries.
        virtual InsertionProxy<char> insertion_proxy();
    };
    typedef GenericDictionary<char, _Goo_m_VART_LIST_APP_CONF> Dictionary;
    /// The char-index has no description aspect as well.
    typedef TValue< Hash<char, iBaseValue<_Goo_m_VART_LIST_APP_CONF>*> > DictValue;

    /// In-place entry copying procedure. Takes the iterator to an existing
    /// mutable entry and pointer to owning dictionary. Copies the entry using
    /// dictionaries allocator overwriting the object the given iterator points
    /// to.
    static void copy_dict_entry( typename DictValue::Value::iterator
                               , Dictionary * );
};

typedef Traits<_Goo_m_VART_LIST_APP_CONF> AppConfTraits;

typedef AppConfTraits::IndexBy<std::string>::Dictionary AppConfNameIndex;


// Note: the char-indexing dictionary aspect is declared ath the configuration.hpp
// header.

}  // namespace dict
/** @} */  // end of appParameters group
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

