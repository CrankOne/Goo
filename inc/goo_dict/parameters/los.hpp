# ifndef H_GOO_DICT_LOS_H
# define H_GOO_DICT_LOS_H

/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# include "goo_dict/parameter.tcc"
# include "goo_dict/parameters/pointer_parameter.tcc"

namespace goo {
namespace dict {

/**@class LOS
 * @brief List-of-structures parameter is an AoS-like data structure.
 *
 * List-of-structures is a list with arbitrary-typed elements. It is not
 * generally supposed to be an initial application parameter structure, but may
 * appear at some sophisticated configuration schemes.
 */
template<>
class Parameter< List<iAbstractParameter*> > : public mixins::iDuplicable< iAbstractParameter
                                                                         , Parameter< List<iAbstractParameter*> >
                                                                         , InsertableParameter<iAbstractParameter*>
                                                                         > {
public:
    typedef List<iAbstractParameter*> Array;
    typedef mixins::iDuplicable< iAbstractParameter
                               , Parameter<Array>
                               , InsertableParameter<iAbstractParameter*> > DuplicableParent;
public:
    Parameter( const Parameter & );
    Parameter( const char *
             , const char * );
};  // class LOS

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_DICT_LOS_H
