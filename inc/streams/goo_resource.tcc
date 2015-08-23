# ifndef H_GOO_STREAMS_RESOURCE_H
# define H_GOO_STREAMS_RESOURCE_H

# include "goo_ds_bases.hpp"

# ifdef ENABLE_DATASTREAMS
namespace goo {
namespace streaming {

namespace abstract {

class Stream;

class Resource {
protected:
    virtual void _V_dock_stream( Stream * ) = 0;
    virtual void _V_undock_stream( Stream * ) = 0;
public:
    void dock_stream( Stream * str ) {
        _V_dock_stream(str); }
    void undock_stream( Stream * str ) {
        _V_undock_stream(str); }
    virtual ~Resource(){}
};


template<typename DataSizeT>
class ResourcePlainIn : public virtual Resource {
public:
    typedef DataSizeT DataSize;
protected:
    virtual void _V_write_block( const UByte *, DataSize ) = 0;
public:
    virtual ~ResourcePlainIn() {}
    void write_block( const UByte * bs, DataSize sz ) {
        assert(sz);
        _V_write_block( bs, sz );
    }
};

template<typename DataSizeT>
class ResourcePlainOut : public virtual Resource {
public:
    typedef DataSizeT DataSize;
protected:
    virtual void _V_read_block( const UByte *&, DataSize ) = 0;
public:
    virtual ~ResourcePlainOut() {}
    void read_block( UByte *& bs, DataSize sz ) {
        assert(sz);
        _V_write_block( bs, sz );
    }
};


template<typename DataSizeT,
         Direction dir>
class iResourceRandom;  // generic template instantiation forbidden

template<typename DataSizeT>
class iResourceRandom<DataSizeT, in> : public virtual Resource {
public:
    typedef DataSizeT DataSize;
protected:
    virtual void _V_reserve( DataSize ) = 0;
public:
    void reserve( DataSize desiredSize ) {
        _V_reserved( desiredSize );
    }
};

template<typename DataSizeT>
class iResourceRandom<DataSizeT, out> : public virtual Resource {
public:
    typedef DataSizeT DataSize;
protected:
    virtual DataSize _V_used_size( ) const = 0;
public:
    DataSize used_size( DataSize desiredSize ) const {
        return _V_used_size( );
    }
};

template<typename DataSizeT>
class iResourceRandom<DataSizeT, bidir> :
        public iResourceRandom<DataSizeT, in>,
        public iResourceRandom<DataSizeT, out> { };

}  // namespace abstract

//
//
//

template<Direction dir,
         typename DataSizeT>
class ResourcePlain;  // generic template instantiation forbidden

template<typename DataSizeT>
class ResourcePlain<in, DataSizeT> : public abstract::ResourcePlainIn<DataSizeT> {
public:
    static constexpr Direction dir = in;
};

template<typename DataSizeT>
class ResourcePlain<out, DataSizeT> : public abstract::ResourcePlainOut<DataSizeT> {
public:
    static constexpr Direction dir = out;
};

template<typename DataSizeT>
class ResourcePlain<bidir, DataSizeT> : public abstract::ResourcePlainIn<DataSizeT>,
                                        public abstract::ResourcePlainOut<DataSizeT> {
public:
    static constexpr Direction dir = bidir;
};


template<Direction dir,
         typename DataSizeT,
         typename PositionIndexT,
         typename PositionIteratorT>
class ResourceRandom : public ResourcePlain<dir, DataSizeT>,
                       public abstract::iResourceRandom<DataSizeT, dir> {
public:
    typedef ResourcePlain<dir, DataSizeT> ParentPlain;
    typedef typename ParentPlain::DataSize DataSize;
    typedef PositionIndexT PositionIndex;
    typedef PositionIteratorT Iterator;
protected:
    virtual void _V_set_position( Iterator &, PositionIndex ) = 0;
public:
    virtual void set_position( Iterator & pIt, PositionIndex pIdx ) {
        _V_set_position( pIt, pIdx );
    }
};


} // namespace streaming
} // namespace goo

# endif  // ENABLE_DATASTREAMS
# endif  // H_GOO_STREAMS_RESOURCE_H

