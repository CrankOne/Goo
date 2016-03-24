# include "goo_dict/dict.hpp"
# include "goo_exception.hpp"

namespace goo {
namespace dict {

Dictionary::Dictionary( const char * name_,
                        const char * description_ ) :
            iAbstractParameter(name_,
                               description_,
                               false ) {
}

void
Dictionary::insert_parameter( iAbstractParameter * instPtr ) {
    _parameters.emplace( instPtr->name(),
                         instPtr );
}

void
Dictionary::insert_section( Dictionary * instPtr ) {
    _dictionaries.emplace( instPtr->name(),
                           instPtr );
}

// iAbstractParameter interface implementation
//////////////////////////////////////////////

void
Dictionary::_V_from_string( const char * ) {
    _TODO_  // TODO: need for parser to be specified
}

void
Dictionary::_V_deserialize( const UByte * ) {
    _TODO_  // TODO:
}

Size
Dictionary::_V_serialized_length() const {
    _TODO_  // TODO
}

void
Dictionary::_V_serialize( UByte * ) const {
    _TODO_  // TODO
}

size_t
Dictionary::_V_string_length() const {
    _TODO_  // TODO
}

void
Dictionary::_V_to_string( char * ) const {
    _TODO_  // TODO
}

}  // namespace dict
}  // namespace dict

