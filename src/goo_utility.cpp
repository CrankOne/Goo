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

# include <sstream>
# include <cstdarg>
# include <memory>
# include <iomanip>
# include <cassert>
# include <cstring>
# include "goo_utility.hpp"
# include "goo_exception.hpp"

namespace stdE {

center_helper<std::string::value_type, std::string::traits_type> centered(const std::string& str) {
    return center_helper<char>(str);
}

template<typename charT, typename traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, const center_helper<charT, traits>& c) {
    size_t w = s.width();
    if(w > c.str_.length()) {
        std::streamsize left = (w + c.str_.length()) / 2;
        s.width(left);
        s << c.str_;
        s.width(w - left);
        s << "";
    } else {
        s << c.str_;
    }
    return s;
}

}  // namespace stdE

namespace goo {

std::string concat_block_strings( const std::string & table1,
                                  const std::string & table2 ) {
    Size mxLen = 0;
    std::istringstream streamTableT(table1);
    while( !streamTableT.eof() ) {
        std::string s1;
        getline(streamTableT, s1);
        if( s1.size() > mxLen ) {
            mxLen = s1.size();
        }
    }
    std::ostringstream ss;
    std::istringstream streamTable1(table1);
    std::istringstream streamTable2(table2);
    while( !streamTable1.eof() || !streamTable2.eof() ) {
        std::string s1;
        getline(streamTable1, s1);
        while( s1.size() < mxLen )
            s1 += " ";
        std::string s2;
        getline(streamTable2, s2);
        ss << s1 << s2 << std::endl;
    }
    return ss.str();
}

void
cf_write( FILE * f, Size len, const void * bf) {
    Size nwrote =
        fwrite( bf, 1, len, f );
    if( nwrote != len ) {
        emraise( ioError, "fwrite() wrote only " GOO_SIZE_FMT " bytes instead of " GOO_SIZE_FMT,
                nwrote, len );
    }
}

void
cf_read( FILE * f, Size len, void * bf) {
    Size nread =
        fread( bf, 1, len, f );
    if( nread != len ) {
        emraise( ioError, "fread() read only " GOO_SIZE_FMT " bytes instead of " GOO_SIZE_FMT,
                nread, len );
    }
}

std::string
strfmt( const char * fmt_str, ... ) {
    int final_n,
        n = strlen(fmt_str) * 2; // reserve 2 times as much as the length of the fmt_str
    std::string str;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while(1) {
        formatted.reset(new char[n]); // wrap the plain char array into the unique_ptr
        strcpy(&formatted[0], fmt_str);
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str, ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}

//
// ParallelLog
//

/**@class ParallelStream
 *
 * A simple std::stringstream-based class for logging multithreaded
 * applications.
 */

void
ParStream::push( unsigned char ncol, const std::string & msg ) {
    auto tm = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lck(_mtx);
    if( ncol <  _nCols ) {
        if( _nMaxChars[ncol] < msg.size() ) { _nMaxChars[ncol] = msg.size(); }
        _log.insert( std::pair<TP, std::pair<UByte, std::string> >(
            tm,
            std::pair<UByte, std::string>(ncol, msg)
        ) );
    } else {
        emraise( malformedArguments, "Wrong thread number: %d > %d",
                (int) ncol,
                (int) _nCols+1);
    }
}

void
ParStream::dump( std::ostream & os ) {
    std::lock_guard<std::mutex> lck(_mtx);
    unsigned short tablelength = 0;
    for( unsigned char i = 1; i < _nCols; ++i ) {
        tablelength += _nMaxChars[i];
    }
    const std::string noMsg = ".";
    const std::string ** columns = new const std::string * [_nCols-1];
    for( auto it = _log.cbegin(); it != _log.cend(); ++it ) {
        UByte nCol = it->second.first;
        const std::string & msg = it->second.second;
        if(!nCol) {
            os << "|>>-" << std::setfill('-') << std::setw(tablelength-5)
               << stdE::centered(msg) << std::setfill(' ') << "-<<|" << std::endl;
            if(msg.size()>1) continue;
        }

        for( UByte i = 0; i < _nCols-1; ++i ) {
            columns[i] = &noMsg;
        }
        columns[it->second.first-1] = &msg;
        auto iit = it; 
        for( ++iit; iit->first == it->first &&
                    iit->second.first &&
                    iit->second.first != it->second.first; ++iit ) {
            columns[iit->second.first-1] = &(it->second.second);
        }

        for( unsigned char i = 1; i < _nCols; ++i ) {
            os << std::setw( _nMaxChars[i] );
            if( &noMsg != columns[i-1] ) {
                os << std::setfill('.');
            }
            os << stdE::centered(*columns[i-1])
               << std::setfill(' ') << "|";
        }
        os << std::endl;
    }
    delete [] columns;
}

}  // namespace goo

