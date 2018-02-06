/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

# include "goo_dict/appCfg/traits.hpp"
# include "goo_dict/appCfg/insertion_proxy.tcc"

namespace goo {
namespace dict {

// generic options container

InsertionProxy<std::string>
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<std::string>::Aspect::insertion_proxy() {
    return InsertionProxy<std::string>(dynamic_cast<InsertionProxy<std::string>::Subsection &>(*this));
}

/*
const Traits<_Goo_m_VART_LIST_APP_CONF>::FeaturedBase *
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<std::string>::Aspect::parameter_ptr( const char * p ) const {
    return static_cast<const Dictionary &>(*this).entry_ptr( p );
}
*/


# if 0
template<typename ... AspectTs>
const TValue<AspectTs...> & retrieve_entry_by_str_path(
        const GenericDictionary<std::string, AspectTs> & D
      , const std::string & strPath ) {
    auto path = dpath( strPath );
    return retrieve_entry_by_path( D, path );
}

template<typename ... AspectTs>
const TValue<AspectTs...> & retrieve_entry_by_path(
        const GenericDictionary<std::string, AspectTs> & D
      , const DictPath & p ) {
    if( p.next ) {
        return retrieve_entry_by_path( D.subsection(p), p.next );
    }
    //return D.value(p.);
}
# endif

// shortcut-only options container

InsertionProxy<char>
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<char>::Aspect::insertion_proxy() {
    return InsertionProxy<char>(dynamic_cast<GenericDictionary<char, _Goo_m_VART_LIST_APP_CONF> &>(*this));
}

}  // namespace dict
}  // namespace goo
