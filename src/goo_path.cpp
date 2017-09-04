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

# include "goo_path.hpp"

# include "goo_exception.hpp"

# include <set>

# include <cerrno>
# include <cstring>

namespace goo {
namespace filesystem {

Path::Path( Interpolator * intrp ) :    std::string(),
                                        _statCacheValid(false),
                                        _interpolatedCacheValid(false),
                                        _interpolator( intrp ) {
}

Path::Path( const std::string & o, Interpolator * intrp )
                                : std::string(o),
                                _statCacheValid(false),
                                _interpolatedCacheValid(false),
                                _interpolator(intrp) {
}

Path::Path( const char * p, Interpolator * intrp )
                                : std::string(p),
                                _statCacheValid(false),
                                _interpolatedCacheValid(false),
                                _interpolator(intrp) {
}

Path::Path( const Path & o )
                                : std::string( o ),
                                _statCacheValid(false),
                                _interpolatedCacheValid(false),
                                _interpolator(o._interpolator) {
}

struct stat &
Path::get_stat() const {
    const std::string & path = interpolated();
    if( !_statCacheValid ) {
        int rs = ::stat( path.c_str(), &_statCache );
        _exists = true;
        if( -1 == rs ) {
            if( errno == ENOENT ) {
                _exists = false;
                _statCacheValid = true;
            } else {
                emraise( ioError, "Unable to retrieve stat() for path \"%s\": %s",
                    path.c_str(), strerror( errno ) );
            }
        }
        _statCacheValid = true;
    }
    return _statCache;
}

bool
Path::exists() const {
    if( !_statCacheValid ) {
        get_stat();
    }
    return _exists;
}

bool
Path::is_dir() const {
    return get_stat().st_mode & S_IFDIR;
}

bool
Path::is_file() const {
    return get_stat().st_mode & S_IFREG;
}

void
Path::interpolator( Interpolator * ip ) const {
    _interpolatedCacheValid = _statCacheValid = false;  // TODO: invalidate_caches();
    _interpolator = ip;
}

const std::string &
Path::interpolated() const {
    if( !_interpolatedCacheValid ) {
        if( _interpolator ) {
            _interpolatedCache = _interpolator->interpolate(*this);
            _interpolatedCacheValid = true;
        } else {
            // Trivial interpolation (identity).
            _interpolatedCache = *this;
        }
    }
    return _interpolatedCache;
}


std::vector<std::string>
Path::get_dir_entries() const {
    std::set<std::string> content;
    DIR * dp;
    struct dirent * dirPtr;
    if( !is_dir() ) {
        emraise( ioError, "Is not a directory: \"%s\".",
            interpolated().c_str() );
    }
    if( (dp  = opendir(interpolated().c_str())) == NULL ) {
        emraise( ioError, "Unable to open dir \"%s\": %s.",
            interpolated().c_str(), strerror(errno) );
    }
    while( (dirPtr = readdir(dp)) != NULL ) {
        content.insert( dirPtr->d_name );
    }
    if( dp ) {
        closedir( dp );
    }
    return std::vector<std::string>( content.begin(), content.end() );
}

Path
Path::concat( const std::string & postfix ) const {
    return Path(interpolated() + "/" + postfix );
}

}  // namespace filesystem
}  // namespace goo

