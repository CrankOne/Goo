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

# ifndef H_GOO_PATH_H
# define H_GOO_PATH_H

# include "goo_config.h"

// todo: if on unix
# include <sys/stat.h>
# include <dirent.h>

# include <vector>
# include <string>

namespace goo {
namespace filesystem {

/// Rudimentary path API with interpolation support. To be elaborated further.
/// TODO: wildcards
/// TODO: dir content caching
/// TODO: advanced stats: size, timestamps, etc.
/// TODO: concat, relative-to-absolute resolution, relateve between two
//        resolution
/// todo: windows support (minor priority)
class Path : public std::string {
public:
    class Interpolator {
    public:
        virtual std::string interpolate( const std::string & p ) const = 0;
        std::string operator()( const std::string & p ) const
                                                { return interpolate(p); }
    };
protected:
    mutable bool _exists;
    mutable bool _statCacheValid;
    mutable struct stat _statCache;

    mutable bool _interpolatedCacheValid;
    mutable std::string _interpolatedCache;

    Interpolator * _interpolator;

    struct stat & get_stat() const;
public:
    Path( Interpolator * intrp=nullptr );
    Path( const std::string & o, Interpolator * intrp=nullptr );
    Path( const char * p, Interpolator * intrp=nullptr );
    Path( const Path & o );

    virtual bool exists() const;
    virtual bool is_dir() const;
    virtual bool is_file() const;

    // TODO: support wildcards, caching, etc.
    virtual std::vector<std::string> get_dir_entries() const;

    void interpolator( Interpolator * ip );

    virtual const std::string & interpolated() const;

    // TODO: quite primitive
    virtual Path concat( const std::string & );
};  // class Path

}  // namespace filesystem
}  // namespace goo

# endif  // H_STROMA_V_INTERPOLABLE_PATH_H


