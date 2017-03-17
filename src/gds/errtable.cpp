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

# include "gds/errtable.h"

# include <cstdlib>
# include <map>
# include <goo_exception.hpp>

namespace goo {
namespace gds {

static std::map<std::pair<void(*)(), int>, std::string> *
_static_GDSErrorsDescriptionsPtr = NULL;

}  // namespace gds
}  // namespace goo

# ifdef ENABLE_GDS
# ifdef __cplusplus
extern "C" {
# endif  /* __cplusplus */

void
gds_insert_error_description( void(*fPtr)(), int result, const char * descr ) {
    if( !goo::gds::_static_GDSErrorsDescriptionsPtr ) {
        goo::gds::_static_GDSErrorsDescriptionsPtr
                = new std::map<std::pair<void(*)(), int>, std::string>();
    }

    auto ir = goo::gds::_static_GDSErrorsDescriptionsPtr->emplace(
                                        std::make_pair(fPtr, result), descr );
    if( !ir.second ) {
        emraise( nonUniq, "GDS error table: %d --- \"%s\" for %p.",
            result, descr, fPtr )
    }
}

const char *
gds_get_error_description_for( void(*fPtr)(), int result ) {
    if( ! goo::gds::_static_GDSErrorsDescriptionsPtr ) {
        return NULL;
    }
    auto it = goo::gds::_static_GDSErrorsDescriptionsPtr->find(
            std::make_pair(fPtr, result) );
    if( goo::gds::_static_GDSErrorsDescriptionsPtr->end() == it ) {
        return NULL;
    }
    return it->second.c_str();
}

# ifdef __cplusplus
}
# endif  /* __cplusplus */
# endif  /* ENABLE_GDS */


