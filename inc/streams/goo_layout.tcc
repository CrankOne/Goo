# ifndef H_GOO_STREAMING_REPRESENTATION_INTERFACE_H
# define H_GOO_STREAMING_REPRESENTATION_INTERFACE_H

# include "goo_resource.tcc"

namespace goo {
namespace streaming {

namespace abstract {

template<typename ReflectionT,
         typename TypeInfoT,
         typename ATypeIDT,
         typename PositionIndexT=Size,
         typename DataSizeT=Size,
         typename ArrayDimensionT=UByte,
         typename ArraySizeT=Size>
struct ReflectionTraits {
    typedef ReflectionT      Reflection;
    typedef PositionIndexT   PositionIndex;
    typedef TypeInfoT        TypeInfo;
    typedef DataSizeT        DataSize;
    typedef ATypeIDT         ATypeID;
    typedef ArrayDimensionT  ArrayDimension;
    typedef ArraySizeT       ArraySize;
};

class Layout {
public:
    void dock_stream( Stream * );
    void undock_stream( Stream * );
    virtual ~Layout(){}
};

template<typename ReflectionTraitsT,
         typename BytesT=UByte *>
class ReflexiveLayout : public virtual Layout {
public:
    typedef ReflectionTraitsT ReflectionTraits;
    typedef typename ReflectionTraits::Reflection       Reflection;
    typedef typename ReflectionTraits::PositionIndex    PositionIndex;
    typedef typename ReflectionTraits::TypeInfo         TypeInfo;
    typedef typename ReflectionTraits::DataSize         DataSize;
    typedef typename ReflectionTraits::ATypeID          ATypeID;
    typedef typename ReflectionTraits::ArrayDimension   ArrayDimension;
    typedef typename ReflectionTraits::ArraySize        ArraySize;
    typedef BytesT Bytes;
protected:
public:

    /// Tests reflection layout
    void self_test();

    # define for_all_interface_methods( m, T, N ) \
        /* Reflection serialization */ \
        m(reflection_to_bytes,,            DataSize, /* */ T(const Reflection &) N(rfl), T(Bytes &) N(sRef) ) \
        m(reflection_from_bytes,,              void, /* */ T(const Bytes &) N(s), T(const DataSize) N(sz), T(Reflection &) N(rfl)) \
        \
        /* Position index operations */ \
        m(get_position,,              PositionIndex, /* */ T(const Reflection &) N(rfl)) \
        m(set_position,,                       void, /* */ T(Reflection &) N(rfl), T(const PositionIndex &) N(pIdx)) \
        \
        /* Type identifier operations */ \
        m(get_a_type_id,,                   ATypeID, /* */ T(const Reflection &) N(rfl)) \
        m(set_a_type_id,,                      void, /* */ T(Reflection &) N(rfl), T(const ATypeID) N(tid)) \
        \
        /* Array operations */ \
        m(set_array_dimension,,                void, /* */ T(Reflection &) N(rfl), T(const ArrayDimension) N(nd)) \
        m(get_array_dimension,const, ArrayDimension, /* */ T(const Reflection &) N(rfl)) \
        m(get_array_d_size, const,        ArraySize, /* */ T(const Reflection &) N(rfl), T(const ArrayDimension) N(nd)) \
        m(set_array_d_size,,                   void, /* */ T(Reflection &) N(rfl), T(const ArrayDimension) N(nd), T(ArraySize) N(dsz)) \
        /* ... */

    //
    // Macro that declares the actual interface
    //

    # define declare_interface_f( name, constQual, retType, ... )           \
        protected: virtual retType _V_ ## name( __VA_ARGS__ ) constQual = 0;\
        public:            retType        name( __VA_ARGS__ ) constQual;
    # define omit( keyword )
    # define provide( keyword ) keyword

    for_all_interface_methods( declare_interface_f, provide, provide )

    # undef declare_interface_f
    # undef omit

    // ...
};

}  // namespace abstract

}  // namespace goo
}  // namespace goo

# endif  // H_GOO_STREAMING_REPRESENTATION_INTERFACE_H

