
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

// Created by crank on 12.01.18.

# ifndef H_GOO_DICT_TYPES_H
# define H_GOO_DICT_TYPES_H

# include "goo_config.h"

# include <vector>
# include <list>
# include <unordered_map>
# include <unordered_set>
# include <limits>

# include "goo_sys/exception.hpp"

# ifdef FNTHN_ALLOCATORS
    // TODO: use https://github.com/foonathan/memory here
# endif

// Fine configuration for
// - disables dictionaries (overall):
//# define _Goo_m_DISABLE_DICTIONARIES
// - application configuration dictionaries implementation
# define _Goo_m_DISABLE_APP_CONF_DICTIONARIES
// - disables common dictionaries implementation
# define _Goo_m_DISABLE_GENERIC_DICTIONARIES

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

/**@file types.hpp
 */

namespace goo {
namespace dict {



}  // namespace ::goo::dict
}  // namespace ::goo

# endif  // !defined( _Goo_m_DISABLE_DICTIONARIES )

# endif  // H_GOO_DICT_TYPES_H
