# ifndef H_GOO_STREAMS_SERIALIZABLE_INTERFACE_H
# define H_GOO_STREAMS_SERIALIZABLE_INTERFACE_H

# include "goo_resource.tcc"

namespace goo {
namespace streaming {

template<template<Direction> class StreamT,
         Size SizeT>
class iStorable {
public:
    typedef StreamT<in>  StreamIn;
    typedef StreamT<out> StreamOut;
protected:
    virtual void _V_stored_size() const = 0;
    virtual void _V_store( StreamOut & ) const = 0;
    virtual void _V_restore( StreamIn & ) = 0;
public:
    void stored_size() const {
        return _V_stored_size(); }
    void serialize( StreamOut & oss ) const {
        _V_store(oss); }
    void deserialize( StreamIn & iss ) {
        _V_restore(iss); }
};

template<template<Direction> class StreamT,
         Size SizeT>
class iUniformLengthStorable : public iStorable<StreamT, SizeT> {
public:
    static constexpr Size TypeSize = SizeT;
protected:
    virtual void _V_serialized_size() const final { return SizeT; }
};

template<template<Direction> class StreamT,
         Size SizeT>
class iVariadicLengthStorable : public iStorable<StreamT, SizeT> {
};

}  // streaming
}  // goo

# endif  // H_GOO_STREAMS_SERIALIZABLE_INTERFACE_H

