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


# ifndef H_GOO_COLORS_H
# define H_GOO_COLORS_H

# include "palette.h"

namespace goo {

#define MIN3(x,y,z)  ((y) <= (z) ? \
                         ((x) <= (y) ? (x) : (y)) \
                     : \
                         ((x) <= (z) ? (x) : (z)))
#define MAX3(x,y,z)  ((y) >= (z) ? \
                         ((x) >= (y) ? (x) : (y)) \
                     : \
                         ((x) >= (z) ? (x) : (z)))

# if 1

template<typename ChannelT>
class ColorT {
public:
    enum StoragingType { sRGB, sHSV, sHSL, };
private:
    union {
        ChannelT _channel[3];
        struct { ChannelT r, g, b; } _rgb;
        struct { ChannelT h, s, v; } _hsv;
        struct { ChannelT h, s, l; } _hsl;
    } _channels;
    StoragingType _sType;
protected:
    virtual _get_red        () const;
    virtual _get_green      () const;
    virtual _get_blue       () const;
    virtual _get_hue        () const;
    virtual _get_saturation () const;
    virtual _get_lightness  () const;
    virtual _get_value      () const;

    virtual _set_red        ( ChannelT );
    virtual _set_green      ( ChannelT );
    virtual _set_blue       ( ChannelT );
    virtual _set_hue        ( ChannelT );
    virtual _set_saturation ( ChannelT );
    virtual _set_lightness  ( ChannelT );
    virtual _set_value      ( ChannelT );
public:
    ColorT() : _channels = {0, 0, 0}, _sType(sRGB) {}
    ColorT( StoragingType st ) : _channels = {0, 0, 0}, _sType(st) {}
    virtual ~ColorT() {}

    inline ChannelT red  () const { return _get_red  (); }
    inline ChannelT green() const { return _get_green(); }
    inline ChannelT blue () const { return _get_blue (); }

    inline void red  ( ChannelT val ) { _set_red  (val); }
    inline void green( ChannelT val ) { _set_green(val); }
    inline void blue ( ChannelT val ) { _set_blue (val); }

    inline ChannelT hue       () const { return _get_hue       (); }
    inline ChannelT saturation() const { return _get_saturation(); }
    inline ChannelT lightness () const { return _get_lightness (); }
    inline ChannelT value     () const { return _get_value     (); }

    inline void hue       ( ChannelT val ) { _set_hue       (val); }
    inline void saturation( ChannelT val ) { _set_saturation(val); }
    inline void lightness ( ChannelT val ) { _set_lightness (val); }
    inline void value     ( ChannelT val ) { _set_value     (val); }
};

template<typename ChannelT>
ChannelT ColorT::_get_red() {
    if( _sType == sRGB ) {
        return _channels[0];
    }
}

# endif

}  // namespace goo

# endif  // H_GOO_COLORS_H

