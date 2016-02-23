# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_H

# include <goo_types.h>

namespace goo {
namespace dicts {
namespace aux {

class iParameter {
public:
    enum DictEntryTypeDescription {
        # define declare_enumval( code, cnm, gnm, gdsnm ) \
            gnm ## PType = code,
        for_all_atomic_datatypes( declare_enumval )
        # undef declare_enumval
        // ... ?
        string = 0xfe,
        custom = 0xff
    };
protected:
    std::string _name,
                _description,
                _stringValue
                ;
public:
    iParameter( const std::string n, const std::string d ) : _name(n), _description(d) {}
    virtual ~iParameter() {}
    const std::string & name() const { return _name; }
    const std::string & description() const { return _description; }
};  // class iParameter


template<typename T>  // dft impl --- for atomic types
class Parameter : public iParameter {
private:
    T _value;
public:
    T value() const { return _value; }
};  // class Parameter<dft>

template<>
class Parameter<std::string> : public iParameter {
private:
    std::string _value;
public:
    const std::string & value() const { return _value; }
};  // class Parameter<string>

/*
template<>
class Parameter<???> : public iParameter {
private:
    std::string _value;
public:
    const std::string & value() const { return _value; }
};  // class Parameter<string>
*/

}  // namespace aux
}  // namespace dicts
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_H


