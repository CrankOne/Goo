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

# include "goo_dict/parameters/integral.tcc"

namespace goo {
namespace dict {

template<> char
integral_safe_parse<char>( const char * str, int base ) {
    if( '\0' != str[0] && '\0' == str[1] ) {
        return str[0];
    }
    char * end;
    long int r = strtol( str, &end, base );
    if( '\0' == *str ) {
        emraise( badParameter, "Unable to parse empty string as integral number "
            "with base %d.", base );
    }
    if( ERANGE == errno && LONG_MIN == r ) {
        emraise( underflow, "Given string token \"%s\" represents an integer "
            "number which is lesser than upper limit of signed long integer.",
            str );
    }
    if( ERANGE == errno && LONG_MAX == r ) {
        emraise( overflow, "Given string token \"%s\" represents an integer "
            "number which is greater than upper limit of signed long integer.",
            str );
    }
    if( *end != '\0' ) {
        emraise( badParameter, "Unable to parse token %s as integral number "
            "with base %d. Extra symbols on tail: %s.", str, base, end );
    }
    if( r < std::numeric_limits<char>::min() ) {
        emraise( underflow, "Given string token \"%s\" represents an integer "
            "number which is lesser than upper limit of%s integer type of "
            "length %d.", str, (std::numeric_limits<char>::is_signed ?
                            " signed" : "unsigned" ), (int) sizeof(char) );
    }
    if( r > std::numeric_limits<char>::max() ) {
        emraise( overflow, "Given string token \"%s\" represents an integer "
            "number which is greater than upper limit of%s integer type of "
            "length %d.", str, (std::numeric_limits<char>::is_signed ?
                            " signed" : "unsigned" ), (int) sizeof(char) );
    }
    return (char) r;
}

template<> unsigned long
integral_safe_parse<unsigned long>( const char * str, int base ) {
    char * end;
    unsigned long r = strtoul( str, &end, base );
    if( '\0' == *str ) {
        emraise( badParameter, "Unable to parse empty string as integral number "
            "with base %d.", base );
    }
    if( ERANGE == errno && ULONG_MAX == r ) {
        emraise( overflow, "Given string token \"%s\" represents an integer "
            "number which is greater than upper limit of unsigned long integer.",
            str );
    }
    return r;
}

# ifdef TYPES_128BIT_LENGTH
template<> long long int
integral_safe_parse<long long int>( const char * str, int base ) {
    _TODO_  // TODO: strtoll
}

template<> unsigned long long int
integral_safe_parse<unsigned long long int>( const char * str, int base ) {
    _TODO_  // TODO: std::stoull
}
# endif

}  // namespace dict
}  // namespace goo

