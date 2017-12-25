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
# include "goo_dict/parameter.tcc"

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

class DictInsertionProxy;
class Configuration;
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
class Dictionary : public std::list<iSingularParameter *>
                 , public std::unordered_map<std::string, DictionaryParameter *>
                 , public mixins::iDuplicable< Dictionary
                                             , Dictionary
                                             , Dictionary
                                             , true> {
public:
    typedef std::list<char>    ShortOptString;
    typedef std::list<void*>   LongOptionEntries;  // ptrs are malloc()'d
    typedef std::list<iSingularParameter *> SingularsContainer;
    typedef std::unordered_map<std::string, DictionaryParameter *> DictionariesContainer;
    typedef std::unordered_map<std::string, iSingularParameter *> ParametersByName;
    typedef std::unordered_map<char, iSingularParameter *> ParametersByShortcut;
private:
    /// Long-name parameters index (aggregation).
    ParametersByName _parametersIndexByName;
    /// Shortcut parameters index (aggregation).
    ParametersByShortcut _parametersIndexByShortcut;

    /// Internal function mutating given path str --- parameter entry getter.
    virtual const iSingularParameter * _get_parameter( char [], bool noThrow=false ) const;

    /// Internal function mutating given path str --- subsection getter.
    virtual const DictionaryParameter * _get_subsection( char [], bool noThrow=false ) const;
protected:
    ParametersByName & parameters_by_name() { return _parametersIndexByName; }

    ParametersByShortcut & parameters_by_shortcut() { return _parametersIndexByShortcut; }
public:
    const ParametersByName & parameters_by_name() const { return _parametersIndexByName; }

    const ParametersByShortcut & parameters_by_shortcut() const { return _parametersIndexByShortcut; }

    virtual ~Dictionary();

    /// Inserts parameter instance.
    virtual void insert_parameter( iSingularParameter * );

    /// Inserts dictionary instance.
    virtual void insert_section( DictionaryParameter * );

    /// Public copy ctr for virtual copy ctr.
    Dictionary( const Dictionary & );

    Dictionary() {}

    /// This routine performs simple token extraction from option path.
    /// For example, the following string:
    ///     "one.three-four.five"
    /// must be splitted in following manner:
    ///     current = "one"
    ///     tail = "three-four.five".
    /// In case when only one token is provided, it will be written
    /// in `current', and the tail will be empty.
    /// @returns 0 if no token can be extracted
    /// @returns 1 if there are something in path after extraction.
    static int pull_opt_path_token( char *& path,
                                    char *& current );

    /// Get parameter instance by its full name.
    /// Note, that path delimeter here is dot symbol '.' (const getter).
    /// When parameter is not found, raises `notFound' exception.
    virtual const iSingularParameter & parameter( const char path [] ) const;

    virtual const iSingularParameter & parameter( const std::string & path ) const {
        return parameter( path.c_str() );
    }

    /// Get parameter instance by its full name.
    /// Note, that path delimeter here is dot symbol '.'.
    virtual iSingularParameter & parameter( const char path[] ) {
        const Dictionary * constThis = this;
        return const_cast<iSingularParameter &>(constThis->parameter( path ));
    }

    virtual iSingularParameter & parameter( const std::string & path ) {
        return parameter( path.c_str() );
    }

    /// Operator shortcut for `parameter()`.
    virtual const iSingularParameter & operator[]( const char p[] ) const {
        return parameter(p); }

    /// Const version of faulty-tolerant parameter instance getter. If
    /// parameter lookup fails, returns nullptr.
    virtual const iSingularParameter * probe_parameter( const char path[] ) const;

    /// Faulty-tolerant parameter instance getter. If parameter lookup fails,
    /// returns nullptr.
    virtual iSingularParameter * probe_parameter( const char path[] ) {
        const Dictionary * constThis = this;
        return const_cast<iSingularParameter *>(constThis->probe_parameter( path ));
    }

    virtual const iSingularParameter * probe_parameter( const std::string & sPath ) const;

    virtual iSingularParameter * probe_parameter( const std::string & sp ) {
        const Dictionary * constThis = this;
        return const_cast<iSingularParameter *>(constThis->probe_parameter( sp ));
    }

    /// Get sub-dictionary instance by its full name.
    /// Note, that path delimeter here is dot symbol '.' (const getter).
    virtual const DictionaryParameter & subsection( const char [] ) const;

    /// Get sub-dictionary instance by its full name.
    /// Note, that path delimeter here is dot symbol '.'.
    virtual DictionaryParameter & subsection( const char path[] ) {
        const Dictionary * constThis = this;
        return const_cast<DictionaryParameter &>(constThis->subsection( path ));
    }

    virtual DictionaryParameter & subsection( const std::string & s ) {
        return subsection(s.c_str());
    }

    virtual const DictionaryParameter & subsection( const std::string & s ) const {
        return subsection(s.c_str());
    }

    /// Const version of faulty-tolerant subsection instance getter. If
    /// parameter lookup fails, returns nullptr.
    virtual const DictionaryParameter * probe_subsection( const char path[] ) const;

    /// Faulty-tolerant subsection instance getter. If lookup fails,
    /// returns nullptr.
    virtual DictionaryParameter * probe_subsection( const char path[] ) {
        const Dictionary * constThis = this;
        return const_cast<DictionaryParameter *>(constThis->probe_subsection( path ));
    }

    virtual const DictionaryParameter * probe_subsection( const std::string & p ) const {
        return probe_subsection( p.c_str() );
    }

    virtual DictionaryParameter * probe_subsection( const std::string & p ) {
        return probe_subsection( p.c_str() );
    }

    /// Performs consistency check (only has sense, if extract() was performed
    /// before).
    virtual bool is_consistant( std::map<std::string, const iSingularParameter *> &,
                                const std::string & prefix ) const;

    /// Prints an ASCII-drawn tree with names and brief comments for all
    /// parameters and sub-sections.
    virtual void print_ASCII_tree( std::list<std::string> & ) const;
};  // class Dictionary

/**
 *
 * This class represents a node in a "Configuration" tree-like structure.
 * Instances (usually named and owned by another Dictionary instance) store
 * a set of named (or shortcut-referenced) parameters and other
 * sub-dictionaries.
 *
 * It is implied that user code will utilize InsertionProxy methods to fill
 * this container instances with particular parameters and sub-dictionaries.
 * */
class DictionaryParameter : public mixins::iDuplicable< iAbstractParameter
                                                      , DictionaryParameter
                                                      , iAbstractParameter>
                          , public Dictionary {
    friend class DictInsertionProxy;
    friend class Configuration;
public:
    typedef mixins::iDuplicable< iAbstractParameter
                               , DictionaryParameter
                               , iAbstractParameter> DuplicableParent;
protected:
    /// Marks last inserted parameter as required one.
    void _mark_last_inserted_as_required();
public:
    DictionaryParameter( const char *, const char * );
    DictionaryParameter( const DictionaryParameter & );
    /// Constructs a bound insertion proxy instance object.
    DictInsertionProxy insertion_proxy();

    // ...
};

}  // namespace dict
/** @} */  // end of appParameters group
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

