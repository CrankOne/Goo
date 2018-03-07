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

# include "goo_dict/parameters/logic.hpp"

# if !defined(_Goo_m_DISABLE_DICTIONARIES) && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)

# include <regex>

static const std::regex
        _static_logicalTermRegex_True(  "(true|enable|on|yes|1)",   std::regex_constants::icase ),
        _static_logicalTermRegex_False( "(false|disable|off|no|0)", std::regex_constants::icase );

namespace goo {
namespace dict {
namespace aspects {
bool
iStringConvertible::ConversionTraits<bool>::parse_string_expression(const char *stv) {
    // _static_logicalTermRegex_True
    if (std::regex_match(stv, _static_logicalTermRegex_True)) {
        return true;
    } else if (std::regex_match(stv, _static_logicalTermRegex_False)) {
        return false;
    } else {
        emraise(parserFailure, "Could not interpret \"%s\" as a logical option.", stv);
    }
}

std::string
iStringConvertible::ConversionTraits<bool>::to_string_expression(const bool &val) {
    if (val) {
        return "True";
    } else {
        return "False";
    }
}
}  // namespace aspects
}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES) && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)
