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

class InsertionProxy;
class Configuration;

/**@brief Parameters container with basic querying support.
 * @class Dictionary
 *
 * This class represents a node in a "Configuration" tree-like structure.
 * Instances (usually named and owned by another Dictionary instance) store
 * a set of named (or shortcut-referenced) parameters and other
 * sub-dictionaries.
 *
 * It is implied that user code will utilize InsertionProxy methods to fill
 * this container instances with particular parameters and sub-dictionaries.
 * */
class Dictionary : public mixins::iDuplicable<  iAbstractParameter,
                                                Dictionary,
                                                iAbstractParameter> {
protected:
    typedef std::list<char>    ShortOptString;
    typedef std::list<void*>   LongOptionEntries;  // ptrs are malloc()'d
private:
    /// A parameters storage (composition).
    std::list<iSingularParameter *> _parameters;

    /// A sub-dictionaries composition index.
    std::unordered_map<std::string, Dictionary *> _dictionaries;

    /// Long-name parameters index (aggregation).
    std::unordered_map<std::string, iSingularParameter *> 
                                                    _parametersIndexByName;
    /// Shortcut parameters index (aggregation).
    std::unordered_map<char, iSingularParameter *> _parametersIndexByShortcut;

protected:
    /// Inserts parameter instance created by insertion proxy.
    virtual void insert_parameter( iSingularParameter * );

    /// Inserts dictionary instance created by insertion proxy.
    virtual void insert_section( Dictionary * );

    # if 0
    /// Internal procedure --- appends access caches.
    virtual void _append_configuration_caches(
                const std::string & nameprefix,
                Configuration * conf
            ) const;
    # endif

    /// Marks last inserted parameter as required one.
    void _mark_last_inserted_as_required();

    /// Internal function mutating given path str --- parameter entry getter.
    virtual const iSingularParameter & _get_parameter( char [] ) const;

    /// Internal function mutating given path str --- subsection getter.
    virtual const Dictionary & _get_subsection( char [] ) const;

    friend class InsertionProxy;
    friend class Configuration;
public:
    Dictionary( const char *, const char * );

    ~Dictionary();

    /// Public copy ctr for virtual copy ctr.
    Dictionary( const Dictionary & );

    /// Constructs a bound insertion proxy instance object.
    InsertionProxy insertion_proxy();

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
    virtual const iSingularParameter & parameter( const char path [] ) const;

    /// Get parameter instance by its full name.
    /// Note, that path delimeter here is dot symbol '.'.
    virtual iSingularParameter & parameter( const char path[] ) {
        const Dictionary * constThis = this;
        return const_cast<iSingularParameter &>(constThis->parameter( path ));
    }

    /// Get sub-dictionary instance by its full name.
    /// Note, that path delimeter here is dot symbol '.' (const getter).
    virtual const Dictionary & subsection( const char [] ) const;

    /// Get sub-dictionary instance by its full name.
    /// Note, that path delimeter here is dot symbol '.'.
    virtual Dictionary & subsection( const char path[] ) {
        const Dictionary * constThis = this;
        return const_cast<Dictionary &>(constThis->subsection( path ));
    }

    /// Performs consistency check (only has sense, if extract() was performed
    /// before).
    virtual bool is_consistant( std::map<std::string, const iSingularParameter *> &,
                                const std::string & prefix ) const;

    /// Prints an ASCII-drawn tree with names and brief comments for all
    /// parameters and sub-sections.
    virtual void print_ASCII_tree( std::list<std::string> & ) const;
    
    const std::list<iSingularParameter *> & parameters() const
                                                    { return _parameters; }

    /// A sub-dictionaries composition index.
    const std::unordered_map<std::string, Dictionary *> & dictionaries() const
                                                    { return _dictionaries; }
};  // class Dictionary

}  // namespace dict
/** @} */  // end of appParameters group
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

