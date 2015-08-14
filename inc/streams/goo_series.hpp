# ifndef H_GOO_STREAMS_SERIALIZABLE_INTERFACE_H
# define H_GOO_STREAMS_SERIALIZABLE_INTERFACE_H

# include "goo_resource.tcc"

namespace goo {
namespace streaming {

class iSerializable {
protected:
    virtual void _V_serialized_size() const = 0;
    virtual void _V_serialize( abstract::OutputStreamingIterator & ) const = 0;
    virtual void _V_deserialize( abstract::InputStreamingIterator & ) = 0;
public:
    void serialized_size() const {
        return _V_serialized_size(); }
    void serialize( abstract::OutputStreamingIterator & oit ) const {
        _V_serialize(out); }
    void deserialize( abstract::InputStreamingIterator & iit ) {
        _V_deserialize(iit); }
};

template<Size SizeT>
class iSerializableUniformSize : public iSerializable {
public:
    static constexpr Size TypeSize = SizeT;
protected:
    virtual void _V_serialized_size() const final { return SizeT; }
};

}  // streaming
}  // goo

# endif  // H_GOO_STREAMS_SERIALIZABLE_INTERFACE_H

