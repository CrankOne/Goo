# ifndef H_GOO_STREAMING_REPRESENTATION_INTERFACE_H
# define H_GOO_STREAMING_REPRESENTATION_INTERFACE_H

# include "goo_resource.tcc"

namespace goo {
namespace streaming {

namespace abstract {

class ReflectionBase {
public:
    virtual ~ReflectionBase(){}
};

template<typename ReflectionTraitsT,
         typename BytesT=UByte *>
class ReflexiveLayout {
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

    # define for_all_interface_methods( m, e_type, e_name ) \
        /* Reflection serialization */ \
        m(reflection_to_bytes,,            DataSize, /* */ const Reflection &, Bytes &) \
        m(reflection_from_bytes,,              void, /* */ const Bytes &, const DataSize, Reflection &) \
        \
        /* Position index operations */ \
        m(get_position,,              PositionIndex, /* */ const Reflection &) \
        m(set_position,,                       void, /* */ Reflection &, const PositionIndex &) \
        \
        /* Type identifier operations */ \
        m(get_a_type_id,,                   ATypeID, /* */ const Reflection &) \
        m(set_a_type_id,,                      void, /* */ Reflection &, const ATypeID ) \
        \
        /* Array operations */ \
        m(set_array_dimension,,                void, /* */ Reflection &, const ArrayDimension) \
        m(get_array_dimension,const, ArrayDimension, /* */ const Reflection &) \
        m(get_array_d_size, const,        ArraySize, /* */ const Reflection &, const ArrayDimension) \
        m(set_array_d_size,,                   void, /* */ Reflection &, const ArrayDimension, ArraySize) \
        /* ... */

    //
    // Macro that declares the actual interface
    //

    # define declare_interface_f( name, constQual, retType, ... )           \
        public:            retType        name( __VA_ARGS__ ) constQual;    \
        protected: virtual retType _V_ ## name( __VA_ARGS__ ) constQual = 0;
    # define omit( keyword )
    # define provide( keyword ) keyword

    for_all_interface_methods( declare_interface_f, provide, provide )

    # undef declare_interface_f
    # undef omit

    // ...
};


template<typename ReflectionT,
         typename ResourceT,
         typename DataSizeT>
class Layout {
public:
    typedef ReflectionT Reflection;
    typedef ResourceT   Resource;
    typedef DataSizeT   DataSize;
public:
    void write_reflection( Reflection & rfl, ResourcePlain<in, DataSize> & );
    void write_arithmetic(
                     const UByte * data,
                     Reflection * rfl,
                     ResourcePlain<in, DataSize> & );
    // ...
};  // class Representation

}  // namespace abstract

}  // namespace goo
}  // namespace goo

# endif  // H_GOO_STREAMING_REPRESENTATION_INTERFACE_H

