# ifndef H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H
# define H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

# include <stack>

# include "goo_exception.hpp"
# include "goo_dict/parameter.tcc"
# include "goo_dict/dict.hpp"

namespace goo {
namespace dict {

/**@brief Helper class for parameter insertion.
 *
 * TODO
 * */
class InsertionProxy {
private:
    std::stack<Dictionary *> _stack;
protected:
    /// Constructs new insertion proxy insance.
    InsertionProxy( Dictionary * );
public:
    /// Opens new section.
    InsertionProxy & bgn_sect( const char *, const char * );

    /// Closes most recent section and switches to its previous (or finalizes last).
    InsertionProxy & end_sect( const char * = nullptr );

    template<typename ParameterT> InsertionProxy &
    p( const char * name,
       const char * description );

    template<typename ParameterT> InsertionProxy &
    p( const char * name,
       const char * description,
       ParameterT && defaultValue );

    friend class Configuration;
};  // class InsertionProxy


template<typename ParameterT> InsertionProxy &
InsertionProxy::p( const char * name,
                   const char * description ) {
    _stack.top()->insert_parameter(
            new Parameter<ParameterT>( name, description )
        );
    return *this;
}

template<typename ParameterT> InsertionProxy &
InsertionProxy::p( const char * name,
                   const char * description,
                   ParameterT && defaultValue ) {
    _stack.top()->insert_parameter(
            new Parameter<ParameterT>( name, description, defaultValue )
        );
    return *this;
}

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

