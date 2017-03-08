/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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

# ifndef H_GOO_PARAMETERS_CONFIGURATION_H
# define H_GOO_PARAMETERS_CONFIGURATION_H

# include "goo_dict/dict.hpp"
# include "goo_dict/insertion_proxy.tcc"

namespace goo {
namespace dict {

/**@brief Class representing dictionaries root instance.
 * @class Configuration
 *
 * This class designed mostly for maintaining the application configuration
 * with strong emphasis on command-line arguments parsing. The native system's
 * `getopt_long()` function will perform actual parsing in `extract()` method
 * providing GNU-extended POSIX compatibility of command-line arguments
 * syntax.
 * */
class Configuration : public Dictionary {
protected:
    /// Indicator of long option with mandatory argument.
    static const int longOptNoShortcutRequiresArgument;
    /// Indicator of a kind of long option (logical parameter without argument).
    static const int longOptKey;
private:
    /// `getopt_long()` aux cache for long options structure.
    mutable void * _cache_longOptionsPtr;
    /// `getopt_long()` aux cache for short options structure (just a string).
    mutable char * _cache_shortOptionsPtr;
    /// Indicates whether `getopt_long()` caches are valid.
    mutable bool _getoptCachesValid;

    /// Access cache for short options. This index includes recursively all the
    /// shortcuts provided in sub-sections. Filled by overrided
    /// insert_parameter() method.
    mutable std::unordered_map<char, iSingularParameter *>      _confShortcuts;
    /// Access cache for long options (?).
    //mutable std::unordered_map<std::string, iSingularParameter *>   _longOptions;

    void _free_caches_if_need() const;
protected:
    /// Recursively iterates through all the options and section producing getopt()-strings.
    void _recache_getopt_arguments() const;

    /// Inserts parameter instance created by insertion proxy with caches invalidation.
    virtual void insert_parameter( iSingularParameter * ) override;

    /// Inserts dictionary instance created by insertion proxy with caches invalidation.
    virtual void insert_section( Dictionary * ) override;

    // Inserts shortcut inside hashing container for quick access.
    virtual void _cache_parameter_by_shortcut( iSingularParameter * );

    /// Inserts fully-qualified name inside hashing container for quick access.
    virtual void _cache_parameter_by_full_name( const std::string &, iSingularParameter * );

    /// Internal procedure that composes access caches.
    //virtual void _append_configuration_caches(
    //            const std::string & nameprefix,
    //            Configuration * conf
    //        ) const final;

    /// Sets argument depending on multiplicity.
    static void _set_argument_parameter( iSingularParameter &,
                                         const char *,
                                         std::ostream * );

    /// Internal procedure --- composes short & long options data structures.
    static void _cache_append_options(
                                    const Dictionary &,
                                    const std::string & nameprefix,
                                    ShortOptString &,
                                    LongOptionEntries & );

    /// Aux insertion method for long options (reentrant routine).
    static void _cache_insert_long_option( const std::string &,
                                     Dictionary::LongOptionEntries &,
                                     const iSingularParameter & );

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

    /// Invalidates getopt's caches. Must be called if any containee topology
    /// has changed.
    virtual void invalidate_getopt_caches() const
        { _getoptCachesValid = false; }

    /// _getoptCachesValid getter indicating whether getopt() option caches are
    /// valid.
    bool is_getopt_cache_valid() const { return _getoptCachesValid; }

    /// Extended version of Dictionary's method printing additional decorations
    /// around root node.
    virtual void print_ASCII_tree( std::list<std::string> & ) const override;

    /// Argument-overloaded method that user will probably wish to invoke to
    /// get an ASCII-tree in output stream.
    virtual void print_ASCII_tree( std::ostream & ) const;

    friend class Dictionary;
};  // class Configuration

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_CONFIGURATION_H

