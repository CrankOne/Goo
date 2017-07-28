# ifndef H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H
# define H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

# include <stack>

# include "goo_exception.hpp"
# include "goo_dict/parameter.tcc"
# include "goo_dict/dict.hpp"
# include "goo_dict/parameters/logic.hpp"
# include "goo_dict/parameters/integral.tcc"
# include "goo_dict/parameters/floating_point.tcc"
# include "goo_dict/parameters/string.hpp"

namespace goo {
namespace dict {

/**@brief Helper class for parameter insertion.
 *
 * The typical lifetime of is instances is following:
 *  1. Instance first is created with `Configuration` class `insertion_proxy()`
 *     method.
 *  2. Then parameters are added using this instance with various appropriate
 *     methods like `p()`, `flag()` and `list()`.
 *  3. Insertion proxy then may be safely deleted.
 *
 * Note that `bgn_sect()` and `end_sect()` methods have to be paired --- i.e.
 * each created sub-section must be closed.
 *
 * Note, that to prevent user code from possible mistakes, positional arguments
 * have to be added with own `Configuration` setter method
 * `positional_arguments()`.
 * */
class InsertionProxy {
private:
    std::stack<Dictionary *> _stack;
protected:
    /// Constructs new insertion proxy insance.
    InsertionProxy( Dictionary * );
public:
    InsertionProxy( const InsertionProxy & o ) : _stack(o._stack) {
        printf( "Goo::XXX --- stack copy.\n" );
    }
    /// Opens new section.
    InsertionProxy & bgn_sect( const char *, const char * );

    /// Closes most recent section and switches to its previous
    /// (or finalizes last). Argument is optional and useful for self-check.
    InsertionProxy & end_sect( const char * = nullptr );

    /// Marks the last inserted parameter as mandatory one.
    InsertionProxy & required_argument();

    /// Marks the last inserted logical argument as mandatory one.
    /// newName may be null if name has to be preserved.
    void insert_copy_of( const iSingularParameter &, const char * newName=nullptr );

    //
    // Parameter inserter
    //

    template<typename ParameterT, class ... Types> InsertionProxy &
    p( Types ... args ) {
        auto * p = new InsertableParameter<ParameterT>( args ... );
        p->_check_initial_validity();
        _stack.top()->insert_parameter( p );
        return *this;
    }

    template<class ... Types> InsertionProxy &
    flag( Types ... args ) {
        Parameter<bool> * newParameterPtr = new Parameter<bool>( args ... );
        newParameterPtr->reset_flag();
        newParameterPtr->_check_initial_validity();
        _stack.top()->insert_parameter( newParameterPtr );
        return *this;
    }

    //
    // List inserters
    //

    template<typename ParameterT> InsertionProxy &
    list( char shortcut,
          const char * name,
          const char * description,
          const std::initializer_list<ParameterT> & dfts ) {
        _stack.top()->insert_parameter(
                new Parameter<std::list<ParameterT> >( dfts, shortcut, name, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( const char * name,
          const char * description,
          const std::initializer_list<ParameterT> & dfts ) {
        _stack.top()->insert_parameter(
                new Parameter<std::list<ParameterT> >( dfts, name, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( char shortcut,
          const char * description,
          const std::initializer_list<ParameterT> & dfts ) {
        _stack.top()->insert_parameter(
                new Parameter<std::list<ParameterT> >( dfts, shortcut, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( char shortcut,
          const char * name,
          const char * description ) {
        _stack.top()->insert_parameter(
                new Parameter<std::list<ParameterT> >( shortcut, name, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( const char * name,
          const char * description ) {
        _stack.top()->insert_parameter(
                new Parameter<std::list<ParameterT> >( name, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( char shortcut,
          const char * description ) {
        _stack.top()->insert_parameter(
                new Parameter<std::list<ParameterT> >( shortcut, description )
            );
        return *this;
    }

    friend class Configuration;
    friend class Dictionary;
};  // class InsertionProxy

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

