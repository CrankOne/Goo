# ifndef H_GOO_PARAMETERS_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_H

# include <map>
# include <vector>

# include <iostream>
//# include "goo_dict/insertion_proxy.hpp"
# include "goo_dict/parameter.tcc"

namespace goo {
namespace dict {

class InsertionProxy;

class Dictionary : public iAbstractParameter {
private:
    std::map<std::string, iAbstractParameter *> _parameters;
    std::map<std::string, Dictionary *> _dictionaries;
protected:
    /// Inserts parameter instance created by insertion proxy.
    void insert_parameter( iAbstractParameter * );

    /// Inserts dictionary instance created by insertion proxy.
    void insert_section( Dictionary * );

    Dictionary( const char *, const char * );
    ~Dictionary();

    friend class InsertionProxy;
};  // class Dictionary


class Configuration : public Dictionary {
private:
    // getopt_long() aux caches:
    mutable void * _longOptionsPtr;
    mutable char * _shortOptionsPtr;
protected:
    const void * _form_long_options() const;
    const char * _create_short_opt_string() const;
public:
    /// Ctr expects the `name' here to be an application name and `description'
    /// to be an application description.
    Configuration( const char * name, const char * description );

    ~Configuration();

    /// Parses command-line arguments.
    void extract( int argc, char * const argv[] );

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

