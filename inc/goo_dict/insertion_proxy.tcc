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

    /// Marks the last inserted parameter as mandatory one.
    InsertionProxy & required();

    //
    // Parameter inserter
    //

    template<typename ParameterT, class ... Types> InsertionProxy &
    p( Types ... args ) {
        _stack.top()->insert_parameter(
                new Parameter<ParameterT>( args ... )
            );
        return *this;
    }

    template<typename ParameterT, class ... Types> InsertionProxy &
    rq( Types ... args ) {
        _stack.top()->insert_parameter(
                new Parameter<ParameterT>( args ... )
            );
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
};  // class InsertionProxy

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

