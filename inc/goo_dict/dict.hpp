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
public:
    struct Tokens {
        std::vector<std::string>                positionalValues;
        std::multimap<std::string, std::string> options;
    };
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

    void command_line_argument_to_tokens( int argc, char * argv[], struct Tokens & tokens );
};  // class Configuration

}  // namespace dicts
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

