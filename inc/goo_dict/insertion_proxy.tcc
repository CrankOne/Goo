# ifndef H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H
# define H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

# include <stack>

# include "goo_exception.hpp"
# include "goo_dict/parameter.tcc"
# include "goo_dict/dict.hpp"
# include "goo_dict/parameters/logic.hpp"
# include "goo_dict/parameters/integral.tcc"
# include "goo_dict/parameters/floating_point.tcc"

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
    InsertionProxy & required_argument();

    // Marks the last inserted logical argument as mandatory one.
    //InsertionProxy & as_flag();

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

    //template<typename ParameterT, class ... Types> InsertionProxy &
    //rq( Types ... args ) {  // TODO?
    //    _stack.top()->insert_parameter(
    //            new Parameter<ParameterT>( args ... )
    //        );
    //    return *this;
    //}

    //
    // List inserters
    //

    template<typename ParameterT> InsertionProxy &
    list( char shortcut,
          const char * name,
          const char * description,
          const std::initializer_list<ParameterT> & dfts ) {
        _stack.top()->insert_parameter(
                new InsertableParameter<std::list<ParameterT> >( dfts, shortcut, name, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( const char * name,
          const char * description,
          const std::initializer_list<ParameterT> & dfts ) {
        _stack.top()->insert_parameter(
                new InsertableParameter<std::list<ParameterT> >( dfts, name, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( char shortcut,
          const char * description,
          const std::initializer_list<ParameterT> & dfts ) {
        _stack.top()->insert_parameter(
                new InsertableParameter<std::list<ParameterT> >( dfts, shortcut, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( char shortcut,
          const char * name,
          const char * description ) {
        _stack.top()->insert_parameter(
                new InsertableParameter<std::list<ParameterT> >( shortcut, name, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( const char * name,
          const char * description ) {
        _stack.top()->insert_parameter(
                new InsertableParameter<std::list<ParameterT> >( name, description )
            );
        return *this;
    }

    template<typename ParameterT> InsertionProxy &
    list( char shortcut,
          const char * description ) {
        _stack.top()->insert_parameter(
                new InsertableParameter<std::list<ParameterT> >( shortcut, description )
            );
        return *this;
    }

    friend class Configuration;
};  // class InsertionProxy

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

