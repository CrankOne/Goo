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

# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_H

# include "goo_dict/dict.hpp"

namespace goo {
namespace dict {

/**
 *
 * This class represents a node in a "Configuration" tree-like structure.
 * Instances (usually named and owned by another Dictionary instance) store
 * a set of named (or shortcut-referenced) parameters and other
 * sub-dictionaries.
 *
 * It is implied that user code will utilize InsertionProxy methods to fill
 * this container instances with particular parameters and sub-dictionaries.
 * */
class DictionaryParameter : public mixins::iDuplicable< AbstractParameter
                                                      , DictionaryParameter
                                                      , AbstractParameter>
                          , public Dictionary {
    friend class DictInsertionProxy;
    friend class Configuration;
public:
    typedef mixins::iDuplicable< AbstractParameter
                               , DictionaryParameter
                               , AbstractParameter> DuplicableParent;
public:
    DictionaryParameter( const char *, const char * );
    DictionaryParameter( const DictionaryParameter & );
    /// Constructs a bound insertion proxy instance object.
    DictInsertionProxy insertion_proxy();

    // ...
};

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_H

