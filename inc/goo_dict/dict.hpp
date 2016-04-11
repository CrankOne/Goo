# ifndef H_GOO_PARAMETERS_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_H

# include <map>
# include <unordered_map>
# include <vector>

# include <iostream>
# include <queue>
# include "goo_dict/parameter.tcc"

namespace goo {
namespace dict {

class InsertionProxy;

class Dictionary : public iAbstractParameter {
protected:
    typedef std::queue<char>    ShortOptString;
    typedef std::queue<void*>   LongOptionEntries;  // ptrs are malloc()'d
private:
    std::map<std::string, iAbstractParameter *> _parameters;
    std::map<char, iAbstractParameter *>        _byShortcutIndexed;
    std::map<std::string, Dictionary *>         _dictionaries;

    /// Aux insertion method for long options (reentrant routine).
    static void _insert_long_option( const std::string &,
                                     Dictionary::LongOptionEntries &,
                                     const iAbstractParameter & );
protected:
    /// Inserts parameter instance created by insertion proxy.
    virtual void insert_parameter( iAbstractParameter * );

    /// Inserts dictionary instance created by insertion proxy.
    virtual void insert_section( Dictionary * );

    Dictionary( const char *, const char * );
    ~Dictionary();

    /// Internal procedure --- composes short & long options data structures.
    virtual void _append_options( const std::string & nameprefix,
                                  ShortOptString &,
                                  LongOptionEntries & ) const;

    friend class InsertionProxy;
    friend class Configuration;
};  // class Dictionary


class Configuration : public Dictionary {
private:
    // getopt_long() aux caches:
    mutable void * _cache_longOptionsPtr;
    mutable char * _cache_shortOptionsPtr;
    mutable bool _getoptCachesValid;

    std::unordered_map<char, iAbstractParameter *>      _shortcuts;
    std::unordered_map<std::string, iAbstractParameter> _longOptions;

    void _free_caches_if_need() const;
protected:
    /// Recursively iterates through all the options and section producing getopt()-strings.
    void _recache_getopt_arguments() const;

    /// Inserts parameter instance created by insertion proxy with caches invalidation.
    virtual void insert_parameter( iAbstractParameter * ) override;

    /// Inserts dictionary instance created by insertion proxy with caches invalidation.
    virtual void insert_section( Dictionary * ) override;

    /// Inserts shortcut inside hashing container for quick access.
    virtual void _insert_shortened_parameter( iAbstractParameter * );

    /// Inserts fully-qualified name inside hashing container for quick access.
    virtual void _insert_fully_qualified_parameter( const std::string &, iAbstractParameter * );
public:
    /// Ctr expects the `name' here to be an application name and `description'
    /// to be an application description.
    Configuration( const char * name, const char * description );

    ~Configuration();

    /// Parses command-line arguments.
    void extract( int argc, char * const argv[], std::ostream * verbose=nullptr );

    /// Returns certain paramater by its name or full path.
    const iAbstractParameter & get_parameter( const std::string & ) const;

    /// Constructs a bound insertion proxy instance object.
    InsertionProxy insertion_proxy();

    /// Produces an `usage' instruction text to the stream provided by arg.
    void usage_text( std::ostream &, bool enableASCIIColoring = false );

    /// A wrapper to glibc's wordexp() function.
    static Size tokenize_string( const std::string &, char **& argvTokens );

    /// Cleaner for tokenized string
    static void free_tokens( size_t argcTokens, char ** argvTokens );
};  // class Configuration

}  // namespace dicts
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

