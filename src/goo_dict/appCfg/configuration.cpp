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

# include "goo_dict/appCfg/configuration.hpp"
# include "goo_dict/appCfg/insertion_proxy.tcc"
# include "goo_exception.hpp"
# include "goo_utility.hpp"
# include "goo_dict/util/conf_help_render.hpp"

# include <algorithm>
# include <cstring>
# include <unistd.h>
# include <getopt.h>
# include <wordexp.h>
# include <list>
# include <cassert>
# include <sstream>
# include <climits>
# include <iostream>

namespace goo {
namespace dict {

Configuration::Configuration( const char * descr_) : DuplicableParent(descr_) {}

Configuration::Configuration( const Configuration & orig ) : DuplicableParent( orig ) {}

Configuration::~Configuration() {}

}  // namespace dict
}  // namespace goo

