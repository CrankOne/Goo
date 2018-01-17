# include "goo_dict/parameter_dict.hpp"
# include "goo_dict/insertion_proxy.tcc"

namespace goo {
namespace dict {

DictionaryParameter::DictionaryParameter( const char * name
                                      , const char * description ) :
        AbstractParameter( name
                        , description
                        , 0x0  ) {}

//DictionaryParameter::DictionaryParameter(const DictionaryParameter & o) {
//    // xxx?
//}

}  // namespace dict
}  // namespace goo
