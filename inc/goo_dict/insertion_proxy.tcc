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

    template<typename ParameterT, class ... Types> InsertionProxy &
    p( Types ... args ) {
        _stack.top()->insert_parameter(
                new Parameter<ParameterT>( args ... )
            );
        return *this;
    }

    template<typename ParameterT, class ... Types> InsertionProxy &
    list( const std::initializer_list<ParameterT> & dft, Types ... args ) {
        _stack.top()->insert_parameter(
                new Parameter<std::list<ParameterT> >( dft, args ... )
            );
        return *this;
    }

    template<typename ParameterT, class ... Types> InsertionProxy &
    list( Types ... args ) {
        _stack.top()->insert_parameter(
                new Parameter<std::list<ParameterT> >( args ... )
            );
        return *this;
    }

    friend class Configuration;
};  // class InsertionProxy

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

