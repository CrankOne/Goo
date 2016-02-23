# ifndef H_GOO_PARAMETERS_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_H

# include <iostream>

namespace goo {
namespace dicts {

class InsertionProxy;
class DictEntry;

class iParser;

class Dictionary {
public:
    InsertionProxy get_insertion_proxy();  // TODO: inline-scoped?
    int parse_stream( iParser &, std::ostream & );
    void print_usage( std::ostream & );
    const DictEntry & get( const std::string & ) const;

    /// Shortcut for get() method.
    const DictEntry & operator()( const std::string & key ) const {
        return this->get(key); }

protected:
    // Interface for insertion proxy:
    open_subsection()
    close_subsection();
};

}  // namespace dicts
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

