# ifndef H_GOO_STREAMS_STREAM_H
# define H_GOO_STREAMS_STREAM_H

# include "goo_resource.tcc"

# ifdef ENABLE_DATASTREAMS

namespace goo {
namespace streaming {

namespace mixins {

template<typename DestStreamT>
class InStream : virtual public abstract::StreamBase {
public:
    typedef DestStreamT DestStream;
public:
    void write_data_block( typename DestStream::DSize length, const UByte * data ) {
        DestStream::get_resource()
            .write_block( length, data );
    }
};

template<typename DestStreamT>
class ReflexiveStream : virtual public abstract::StreamBase {
public:
    typedef DestStreamT DestStream;
public:
    // State transitions.


};

}  // namespace abstract

template<Direction dir,
         Reflexivity rfl,
         typename PositionT,
         typename SizeT>
class Stream {
    // todo: forbid generic template instantiation
};

//
//
//

template<typename PositionT,
         typename SizeT>
class Stream<in, plain, PositionT, SizeT> :
    virtual public mixins::InStream< Stream<in, plain, PositionT, SizeT> >
    {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
public:
    iResource<in, serial> & get_resource() {
        // Note: random‐access resource can be downcasted to serial.
        return *streaming_infrastructure_cast<iResource<in, serial> *>(
                abstract::StreamBase::_get_abstract_resource_ptr() );
    }
};

template<typename PositionT,
         typename SizeT>
class Stream<in, reflexive, PositionT, SizeT> :
    virtual public mixins::InStream< Stream<in, plain, PositionT, SizeT> >
    {
public:
    typedef PositionT Position;
    typedef SizeT DSize;
public:
    iResource<in, serial> & get_resource() {
        // Note: random‐access resource can be downcasted to serial.
        return *streaming_infrastructure_cast<iResource<in, serial> *>(
                abstract::StreamBase::_get_abstract_resource_ptr() );
    }
};

# if 0
template<typename T>
    typename std::enable_if<
            std::is_arithmetic<T>::value, Stream<in, plain>
        >::type & 
operator<<( Stream<in, plain> & os, const T & obj ) {
    os.write_data_block( sizeof(T), reinterpret_cast<const UByte *>(&obj) );
    return os;
}
# endif

// todo: serializable stuff

//
//
//




}  // namespace streaming
}  // namespace goo

# endif  // ENABLE_DATASTREAMS
# endif  // H_GOO_STREAMS_STREAM_H

