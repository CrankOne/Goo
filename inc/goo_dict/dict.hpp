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
# include <queue>
# include "goo_dict/parameter.tcc"

namespace goo {

/**@defgroup appParameters Application Parameters
 * 
 * The Goo's Application Parameters implements an extensible facility for
 * declaring, parsing and merging a dictionary of parameters representing
 * a routine configuration. By mean "routine" here the applications
 * (utilities) should be implied, however the facility by itself allows
 * one to parameterize any object this way.
 *
 * We're tending to follow to the POSIX convention:
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html#tag_12_02
 * extended with GNU long options:
 * https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html .
 * 
 * The Configuration class implements an wrapping object around the standard
 * UNIX getopt_long() function performing parsing procedure of command-line
 * arguments. This class instance also represents a root node in tree-like
 * data structure called "Dictionary".
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
 *  dictionaries recursively.
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

class Dictionary : public mixins::iDuplicable<  iAbstractParameter,
                                                Dictionary,
                                                iAbstractParameter> {
protected:
    // TODO: queue -> list!
    typedef std::queue<char>    ShortOptString;
    typedef std::queue<void*>   LongOptionEntries;  // ptrs are malloc()'d
private:
    std::map<std::string, std::pair<iSingularParameter *, bool> > _parameters;
    std::map<char, iSingularParameter *>        _byShortcutIndexed;
    std::map<std::string, Dictionary *>         _dictionaries;

    /// Aux insertion method for long options (reentrant routine).
    static void _insert_long_option( const std::string &,
                                     Dictionary::LongOptionEntries &,
                                     const iSingularParameter & );
protected:
    /// Inserts parameter instance created by insertion proxy.
    virtual void insert_parameter( iSingularParameter * );

    /// Inserts dictionary instance created by insertion proxy.
    virtual void insert_section( Dictionary * );

    Dictionary( const char *, const char * );

    ~Dictionary();

    /// Internal procedure --- composes short & long options data structures.
    virtual void _append_options( const std::string & nameprefix,
                                  ShortOptString &,
                                  LongOptionEntries & ) const;

    /// Internal procedure --- appends access caches.
    virtual void _append_configuration_caches(
                const std::string & nameprefix,
                Configuration * conf
            ) const;

    /// Get parameter instance by its full name.
    /// Note, that path delimeter here is dot symbol '.'.
    virtual const iSingularParameter & _get_parameter( char [] ) const;

    friend class InsertionProxy;
    friend class Configuration;
public:
    /// Public copy ctr for virtual copy ctr.
    Dictionary( const Dictionary & );

    /// This routine performs regex-based token extraction from option path.
    /// For example, the following string:
    ///     "one.three-four.five"
    /// must be splitted in following manner:
    ///     current = "one"
    ///     tail = "three-four.five".
    /// In case when only one token is provided, it will be written
    /// in `current', and the tail will be empty.
    static int pull_opt_path_token( char *& path,
                                    char *& current );

    /// Performs consistency check (only has sense, if extract() was performed before).
    virtual bool is_consistant( std::map<std::string, const iSingularParameter *> &,
                                const std::string & prefix ) const;

    virtual void print_ASCII_tree( std::list<std::string> & ) const;
};  // class Dictionary


class Configuration : public Dictionary {
private:
    // getopt_long() aux caches:
    mutable void * _cache_longOptionsPtr;
    mutable char * _cache_shortOptionsPtr;
    mutable bool _getoptCachesValid;

    /// Access cache for short options.
    mutable std::unordered_map<char, iSingularParameter *>          _shortcuts;
    /// Access cache for long options.
    mutable std::unordered_map<std::string, iSingularParameter *>   _longOptions;

    void _free_caches_if_need() const;
protected:
    /// Recursively iterates through all the options and section producing getopt()-strings.
    void _recache_getopt_arguments() const;

    /// Inserts parameter instance created by insertion proxy with caches invalidation.
    virtual void insert_parameter( iSingularParameter * ) override;

    /// Inserts dictionary instance created by insertion proxy with caches invalidation.
    virtual void insert_section( Dictionary * ) override;

    /// Inserts shortcut inside hashing container for quick access.
    virtual void _cache_parameter_by_shortcut( iSingularParameter * );

    /// Inserts fully-qualified name inside hashing container for quick access.
    virtual void _cache_parameter_by_full_name( const std::string &, iSingularParameter * );

    /// Internal procedure that composes access caches.
    virtual void _append_configuration_caches(
                const std::string & nameprefix,
                Configuration * conf
            ) const final;

    /// Sets argument depending on multiplicity.
    static void _set_argument_parameter( iSingularParameter &,
                                         const char *,
                                         std::ostream * );
public:
    /// Ctr expects the `name' here to be an application name and `description'
    /// to be an application description.
    Configuration( const char * name, const char * description );

    ~Configuration();

    /// Copy ctr.
    Configuration( const Configuration & );

    /// Explicit copy creation.
    //Configuration copy() const { return *this; }

    /// Parses command-line arguments.
    void extract( int argc,
                  char * const argv[],
                  bool doConsistencyCheck=true,
                  std::ostream * verbose=nullptr );

    /// Returns certain paramater by its name or full path.
    const iSingularParameter & get_parameter( const char [] ) const;

    /// Constructs a bound insertion proxy instance object.
    InsertionProxy insertion_proxy();

    /// Produces an `usage' instruction text to the stream provided by arg.
    void usage_text( std::ostream &, bool enableASCIIColoring = false );

    /// A wrapper to glibc's wordexp() function.
    static Size tokenize_string( const std::string &, char **& argvTokens );

    /// Cleaner for tokenized string
    static void free_tokens( size_t argcTokens, char ** argvTokens );

    /// Operator shortcut for get_parameter.
    virtual const iSingularParameter & operator[]( const char [] ) const;

    virtual void print_ASCII_tree( std::ostream & ) const;

    virtual void print_ASCII_tree( std::list<std::string> & ) const override;

    friend class Dictionary;
};  // class Configuration

}  // namespace dicts
/** @} */  // end of appParameters group
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

