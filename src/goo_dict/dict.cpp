# include "goo_dict/dict.hpp"
# include "goo_dict/insertion_proxy.tcc"
# include "goo_exception.hpp"

# include <cstring>

# if 0

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

size_t
Dictionary::_V_string_length() const {
    _TODO_  // TODO
}

void
Dictionary::_V_to_string( char * ) const {
    _TODO_  // TODO
}

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

//
//

||||||| merged common ancestors
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

//
//


void
command_line_argument_to_tokens( int argc,
                                 char * argv[],
                                 struct Configuration::Tokens & tokens ) {
    if( argc < 2 ) {
        tokens.positionalValues.clear();
        tokens.options.clear();
        return;
    }
    // argv[0]  --- omit name of the application.
    for( int i = 1; i < argc; ++i ) {
        const char * const & argument = argv[i];
        const size_t argLength = strlen( argument );
        if( !argLength ) {
            emraise(malformedArguments, "Got an empty argument at %d-th position.", i);
        }
        if( '-' == argument[0] ) {  // guess, this is an option
            // ...
        } else {  // this is an argument
        }
    }
}

InsertionProxy
Configuration::insertion_proxy() {
    return InsertionProxy( this );
}

Configuration::Configuration( const char * name, const char * descr ) : Dictionary(name, descr) {
    _TODO_  // TODO
}

Configuration::~Configuration() {
    _TODO_  // TODO
}

void
Configuration::usage_text( std::ostream & os,
                           bool enableASCIIColoring ) {
    os << name() << " --- " << description() << std::endl
       << "Usage:" << std::endl;
}

}  // namespace dict
}  // namespace dict

# endif

