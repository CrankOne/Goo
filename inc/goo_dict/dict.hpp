# ifndef H_GOO_PARAMETERS_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_H

# include <map>

# include <iostream>
# include "goo_dict/insertion_proxy.hpp"
# include "goo_dict/parameter.tcc"

namespace goo {
namespace dicts {

class Dictionary : public iAbstractParameter {
private:
    std::map<std::string, iAbstractParameter *> _parameters;
    std::map<std::string, Dictionary *> _dictionaries;
protected:
    virtual void _V_from_string( const char * ) override;
    virtual void _V_deserialize( const UByte * ) override;
    virtual Size _V_serialized_length() const override;
    virtual void _V_serialize( UByte * ) const override;
    virtual size_t _V_string_length() const override;
    virtual void _V_to_string( char * ) const override;
public:
    void insert_parameter( iAbstractParameter * );
    void insert_section( Dictionary * );
};  // class Dictionary


class Configuration : public Dictionary {
public:
    Configuration();
    ~Configuration();

    void extract( int argc, char * const argv[] );

    const iParameter & get_parameter( const std::string & ) const;
};  // class Configuration

}  // namespace dicts
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_H

