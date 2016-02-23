# ifndef H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H
# define H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

# include <iostream>

namespace goo {
namespace dicts {

class Dictionary;

class InsertionProxy {
private:
    Dictionary & _dict;
public:
    InsertionProxy( Dictionary & );

    /// Inserts a scalar mandatory parameter entry.
    template<typename T> InsertionProxy p(
            const std::string &,
            const std::string & );

    /// Inserts a scalar parameter entry with default value.
    template<typename T> InsertionProxy p(
            const std::string &,
            const std::string &,
            const T & defaultValue );

    /// Inserts an entry which holds a homogeneous array.
    template<typename T> InsertionProxy arr(
            const std::string &,
            const std::string &,
            size_t n=0 );

    /// Inserts an entry which holds a homogeneous array.
    template<typename T> InsertionProxy arr(
            const std::string &,
            const std::string &,
            T dftVal[],
            size_t n=0 );

    // TODO ... ?

    /// Opens new sub-section.
    InsertionProxy bgn_sect( const std::string &, const std::string & );

    /// Closes previously opened sub-section.
    InsertionProxy end_sect( const std::string &, const std::string & );
};


}  // namespace dicts
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H


