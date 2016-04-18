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
class Configuration;

class Dictionary : public iAbstractParameter {
protected:
    typedef std::queue<char>    ShortOptString;
    typedef std::queue<void*>   LongOptionEntries;  // ptrs are malloc()'d
private:
    std::map<std::string, iSingularParameter *> _parameters;
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

    /// Parses command-line arguments.
    void extract( int argc,
                  char * const argv[],
                  bool doConsistencyCheck=true,
                  std::ostream * verbose=nullptr );

    /// Returns certain paramater by its name or full path.
    const iSingularParameter & get_parameter( const std::string & ) const;

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

    friend class Dictionary;
};  // class Configuration

}  // namespace dicts
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

