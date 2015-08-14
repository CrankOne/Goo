# ifndef H_GOO_STREAMS_SERIALIZED_DATA_LAYOUT_MANAGER_H
# define H_GOO_STREAMS_SERIALIZED_DATA_LAYOUT_MANAGER_H

# include "goo_ds_bases.hpp"

# ifdef ENABLE_DATASTREAMS
namespace goo {
namespace streaming {

template<typename PositionT, typename SizeT>
class SDLMSerial {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
protected:
    abstract::ResourceBase * _resourcePtr;
public:
    class Iterator {
    public:
        typedef SDLMSerial<Position, DSize> OwningSDMLT;
    private:
        OwningSDMLT * _sdlmPtr;
        Position _pos;
    protected:
        Iterator( OwningSDMLT * parPtr, const Position & pos ) :
                _sdlmPtr(parPtr), _pos(pos) {}
    public:
        Iterator( const Iterator & );
        Iterator() : _sdlmPtr(nullptr), _pos(0) {}
        // ...
        friend class SDLMSerial<Position, DSize>;
    };
public:
    Iterator get_iterator();
    void iterator_increment_n( Iterator  & it, DSize );
    Position get_iterator_position( const Iterator & );

    abstract::ResourceBase * get_resource() {
        return _resourcePtr; }
};


template<typename PositionT, typename SizeT>
class SDLMPositional : public SDLMSerial<PositionT, SizeT> {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
    typedef SDLMSerial<PositionT, SizeT> ParentSerial;
public:
    class Iterator : public ParentSerial::Iterator {
    public:
        typedef SDLMSerial<Position, DSize> OwningSDMLT;
        typedef typename ParentSerial::Iterator ParentIteratorSerial;
    private:
        OwningSDMLT * _sdlmPtr;
        Position _pos;
    protected:
        Iterator( OwningSDMLT * parPtr, const Position & pos ) :
                _sdlmPtr(parPtr), _pos(pos) {}
    public:
        Iterator( const Iterator & );
        Iterator() : _sdlmPtr(nullptr), _pos(0) {}
        // ...
        friend class SDLMPositional<Position, DSize>;
    };
public:
    Iterator get_iterator(); // shadowing?

    void iterator_decrement_n( Iterator  & it, DSize );

    void set_iterator_position( const Iterator &, const Position & );
};


} // namespace streaming
} // namespace goo

# endif  // ENABLE_DATASTREAMS
# endif  // H_GOO_STREAMS_SERIALIZED_DATA_LAYOUT_MANAGER_H

