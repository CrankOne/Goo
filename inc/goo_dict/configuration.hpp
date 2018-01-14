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
 *
 * Warning: subsections have no back-populate effect, so modifying them after
 * they were added to Configuration instance will not cause chaches to be
 * invalidated. Thus, it is mandatory to manually invalidate caches with
 * `invalidate_getopt_caches()` just before `extract()` invokation if inserted
 * sub-sections were modified.
 * */
class Configuration : public DictionaryParameter {
public:
    typedef std::list<char> ShortOptString;
    typedef std::list<void*> LongOptionEntries;
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
    /// Temporary cache storage for all shortcuts in config.
    mutable std::unordered_map<char, std::string> _cache_shortcutPaths;
    /// Indicates whether `getopt_long()` caches are valid.
    mutable bool _getoptCachesValid;

    /// Controls, whether to automatically generate -h|--help [subsect] interface.
    bool _dftHelpIFace;

    /// Access cache for short options. This index includes recursively all the
    /// shortcuts provided in sub-sections. Filled by overrided
    /// insert_parameter() method.
    mutable std::unordered_map<char, iSingularParameter *>      _confShortcuts;
    /// Access cache for long options (?).
    //mutable std::unordered_map<std::string, iSingularParameter *>   _longOptions;

    /// Positional parameters. Ptr may be null if positional argument is
    /// disallowed.
    iSingularParameter * _positionalArgument;

    void _free_caches_if_need() const;
protected:
    /// Recursively iterates through all the options and section producing getopt()-strings.
    void _recache_getopt_arguments() const;

    /// Inserts parameter instance created by insertion proxy with caches invalidation.
    virtual void acquire_parameter_ptr( iSingularParameter * ) override;

    /// Inserts dictionary instance created by insertion proxy with caches invalidation.
    virtual void acquire_subsection_ptr( DictionaryParameter * ) override;

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
                                    const DictionaryParameter &,
                                    const std::string & nameprefix,
                                    ShortOptString &,
                                    std::unordered_map<char, std::string> &,
                                    LongOptionEntries & );

    /// Aux insertion method for long options (reentrant routine).
    static void _cache_insert_long_option( const std::string &,
                                     LongOptionEntries &,
                                     const iSingularParameter & );

    ///  Collects shortcutted flags and required options.
    static void _collect_first_level_options(
                        const DictionaryParameter & d,
                        const std::string & nameprefix,
                        std::unordered_map<std::string, iSingularParameter *> & rqs,
                        std::unordered_map<char, iSingularParameter *> & shrt );

    /// Used for usage/help print.
    static void _print_dict_usage( const DictionaryParameter & d,
                                   const std::string & omitShortcuts );

    /// Helper function setting/appending given token as a positional argument.
    void _append_positional_arg( const char * );

public:
    /// Ctr expects the `name' here to be an application name and `description'
    /// to be an application description.
    Configuration( const char * name,
                   const char * description,
                   bool defaultHelpIFace=true );

    ~Configuration();

    /// Copy ctr.
    Configuration( const Configuration & );

    /// Explicit copy creation.
    //Configuration copy() const { return *this; }

    /// Parses command-line arguments. Returns -1 if immediate exit is required.
    int extract( int argc,
                  char * const argv[],
                  bool doConsistencyCheck=true,
                  std::ostream * verbose=nullptr );

    /// Produces an `usage' instruction text to the stream provided by arg.
    void usage_text( std::ostream &, const char * );

    /// Produces subsection reference.
    void subsection_reference( std::ostream &, const char * );

    /// A wrapper to glibc's wordexp() function.
    static Size tokenize_string( const std::string &, char **& argvTokens );

    /// Cleaner for tokenized string
    static void free_tokens( size_t argcTokens, char ** argvTokens );

    /// Extendeds parent version with positional argument resolution.
    //virtual const iSingularParameter & parameter( const char path[] ) const override;

    /// Invalidates getopt's caches. Must be called if any containee topology
    /// has changed.
    virtual void invalidate_getopt_caches() const
        { _getoptCachesValid = false; }

    /// _getoptCachesValid getter indicating whether getopt() option caches are
    /// valid.
    bool is_getopt_cache_valid() const { return _getoptCachesValid; }

    /// Extended version of Dictionary's method printing additional decorations
    /// around root node.
    //virtual void print_ASCII_tree( std::list<std::string> & ) const override;

    /// Argument-overloaded method that user will probably wish to invoke to
    /// get an ASCII-tree in output stream.
    //virtual void print_ASCII_tree( std::ostream & ) const;

    /// Inserts dictionary instance created by third-party code (causes caches)
    /// invalidation. Note, that copy of dictionary will be dynamically
    /// allocated on heap.
    virtual void append_section( const DictionaryParameter & dPtr );

    /// Makes configuration able to acquire single (only) positional argument.
    /// This is not an incremental procedure and has to be used rarely: if
    /// user's code has to treat multiple positional arguments, the
    /// `positional_arguments()` has to be used instead.
    template<typename T> iParameter<T> &
    single_positional_argument( const char name[], const char description[] ) {
        auto * p = new InsertableParameter<T>( name, description );
        p->_check_initial_validity();
        _positionalArgument = p;
        return *p;
    }

    /// Makes configuration instace be able to acquire a list of positional
    /// arguments of the specific type.
    template<typename T> Parameter<Array<T> > &
    positional_arguments( const char name[], const char description[] ) {
        auto p = new InsertableParameter<Array<T> >( name, description );
        _positionalArgument = p;
        return *p;
    }

    /// Returns forwarded arguments (if they were set).
    const Array<std::string> & forwarded_argv() const;

    friend class DictionaryParameter;
};  // class Configuration

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_CONFIGURATION_H

