# ifndef H_GOO_DATA_STREAMS_ABSTRACT_STREAM_BASE_H
# define H_GOO_DATA_STREAMS_ABSTRACT_STREAM_BASE_H

# include "goo_types.h"

namespace goo {
namespace streams {

typedef UByte StreamFeatures;
enum Direction : StreamFeatures {
       in = 0x1,
      out = 0x2,
    bidir = 0x3, // todo?
};
enum Reflexivity : StreamFeatures {
        plain = 0x1,
   positional = 0x2,
    reflexive = 0x3,
};

class AbstractStreamBase {
public:
    # define GOO_STREAM_BASE_FLAGS( dir, rfl ) \
        ( dir | ( 0x1 << (0x2 + rfl) ) )
protected:
    StreamFeatures _flags;
public:
    AbstractStreamBase( Direction dir, Reflexivity rfl ) :
        _flags(GOO_STREAM_BASE_FLAGS(dir, rfl))  {}
    virtual ~AbstractStreamBase() {}

    /// Returns basic stream flags.
    virtual StreamFeatures stream_base_flags() const { return _flags; }
};

}  // namespace streams
}  // namespace goo

# endif  // H_GOO_DATA_STREAMS_ABSTRACT_STREAM_BASE_H

