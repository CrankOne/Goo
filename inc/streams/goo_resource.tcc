# ifndef H_GOO_STREAMS_RESOURCE_H
# define H_GOO_STREAMS_RESOURCE_H

# include "goo_SDLM.tcc"

# ifdef ENABLE_DATASTREAMS
namespace goo {
namespace streaming {

namespace abstract {

template<typename PositionT, typename SizeT>
class ResourceIn : public virtual ResourceBase {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
protected:
    virtual Position _V_write_block( DSize, const UByte * ) = 0;
};

//

template<typename PositionT, typename SizeT>
class ResourceOut : public virtual ResourceBase {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
protected:
    virtual Position _V_read_block( DSize, UByte *& ) = 0;
};

//

template<typename PositionT, typename SizeT>
class ResourceSerial : public virtual ResourceBase {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
private:
    PositionT _cPosition;
protected:
    virtual void _V_increase_position(Position &, Position) = 0;
    PositionT & get_current_position_ref() { return _cPosition; }
public:
    virtual void increase_current_position(Position np) {
        _V_increase_position( _cPosition, np ); }

    PositionT current_position() const { return _cPosition; }
};

//

template<typename PositionT, typename SizeT>
class ResourceRandom : public ResourceSerial<PositionT, SizeT> {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
protected:
    void _V_set_position(Position &, Position) = 0;
public:
    virtual void set_current_position(Position np) {
        _V_set_position( ResourceSerial<PositionT, SizeT>::get_current_position_ref(), np ); }
};

}  // namespace abstract


//
//
//


template<Direction dirT,
         Layout layoutT,
         typename PositionT=GOO_STREAMS_DEFAULT_POSITION,
         typename SizeT=GOO_STREAMS_DEFAULT_SIZE>
class iResource {
    // static_assert<true>( "Default resource base instantiation." );
};

//
//
//

template<typename PositionT,
         typename SizeT>
class iResource<in, serial,
                PositionT,
                SizeT> : public abstract::ResourceIn<PositionT, SizeT>,
                                    public abstract::ResourceSerial<PositionT, SizeT> {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
    typedef SDLMSerial<Position, DSize> SDLM;

    class Iterator : public SDLM::Iterator {
    public:
        void write_block( DSize, const UByte * );
        // ...
    };
public:
    virtual Position write_block( Position place, DSize lBlock, const UByte * blockPtr ) {
        _V_write_block( lBlock, blockPtr );
        _V_increase_position( lBlock );
    }
};

// Note: always can be downcasted to serial!
template<typename PositionT,
         typename SizeT>
class iResource<in, streaming::random,
                PositionT, SizeT> : public abstract::ResourceIn<PositionT, SizeT>,
                                    public abstract::ResourceRandom<PositionT, SizeT> {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
    typedef SDLMPositional<Position, DSize> SDLM;

    class Iterator : public SDLM::Iterator {
    public:
        void read_block( DSize, UByte *& );
        // ...
    };
public:
    virtual Position write_block( Position place, DSize lBlock, const UByte * blockPtr ) {
        _V_write_block( lBlock, blockPtr );
        _V_increase_position( lBlock );
    }
};

} // namespace streaming
} // namespace goo

# endif  // ENABLE_DATASTREAMS
# endif  // H_GOO_STREAMS_RESOURCE_H

