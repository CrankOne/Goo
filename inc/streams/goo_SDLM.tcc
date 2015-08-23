# ifndef H_GOO_STREAMS_SERIALIZED_DATA_LAYOUT_MANAGER_H
# define H_GOO_STREAMS_SERIALIZED_DATA_LAYOUT_MANAGER_H

# include "goo_ds_bases.hpp"
# include "goo_resource.tcc"
# include "goo_utility.hpp"
# include "goo_storable.hpp"
# include "goo_resource.tcc"

# ifdef ENABLE_DATASTREAMS
namespace goo {
namespace streaming {

namespace abstract {

class Stream;

class Layout {
public:
    virtual ~Layout(){}
    void dock_stream( Stream * );
    void undock_stream( Stream * );
};

# if 0
template<typename DataSizeT>
class LayoutPlainIn : public LayoutBase {
public:
    typedef DataSizeT DataSize;
public:
    void dock_stream( Stream * );
    void undock_stream( Stream * );

    template<Direction dir> substream();

    template<template<Direction> class StreamT,
             Size SizeT,
             typename T>
    typename std::enable_if<std::is_base_of<iStorable<StreamT, SizeT>, T>::value, void>::type
    write_entity_data(  abstract::ResourcePlainIn<DataSize> & writableResource,
                        const T & entity ) {
        auto ss = obtain_substream<StreamT<in>>( writableResource );
        entity.store( ss );
        parentISS.merge_substream()
    }

    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, void>::type
    write_arithmetic_data(  abstract::ResourcePlainIn<DataSize> & writableResource,
                            const T entity ) {
        DataSize dataLength = arithmetic_data_length( entity );
        writableResource.reserve( dataLength );
        writableResource.write_block(
                arithmetic_as_data( entity ),
                dataLength
            );
    }
};
# endif

}  // namespace abstract

# if 0
namespace abstract {

template<typename DataSizeT,
         typename PositionIndexT,
         typename ReflectionT,
         typename TypeInfoT>
class iDataLayout {
public:
    typedef DataSizeT DataSize;
    typedef PositionIndexT PositionIndex;
    typedef ReflectionT Reflection;
    typedef TypeInfoT TypeInfo;
protected:
    void _V_new_reflection( Reflection & ) const = 0;

    virtual PositionIndex _V_get_reflection_position( const Reflection & ) const = 0;
    virtual PositionIndex _V_get_data_position(  const Reflection & ) const = 0;

    virtual void _V_get_type_info( TypeInfo &, const Reflection & ) const = 0;
    virtual void _V_set_type_info( Reflection &, TypeInfo & ) const = 0;

    virtual DataSize _V_get_reflection_length( const Reflection & ) const = 0;
    virtual void _V_set_reflection_length( Reflection &, DataSize ) const = 0;

    virtual DataSize _V_get_data_size( const Reflection & ) const = 0;
    virtual void _V_set_data_size( Reflection &, DataSize ) const = 0;
public:
    /// Initializes void reflection.
    void new_reflection( Reflection & rfl ) const {
        _V_new_reflection(rfl); }

    PositionIndex get_reflection_position( const Reflection & rfl ) const {
        return _V_get_reflection_position(rfl); }
    PositionIndex get_data_position(  const Reflection & rfl ) const {
        return _V_get_data_position(rfl); }

    void get_type_info( TypeInfo & ti, const Reflection & rfl ) const {
        _V_get_type_info( ti, rfl ); }
    void set_type_info( Reflection & rfl, TypeInfo & ti ) const {
        _V_set_type_info( rfl, ti ); }

    DataSize get_reflection_length( const Reflection & rfl ) const {
        return _V_get_reflection_length(rfl); }
    void set_reflection_length( Reflection & rfl, DataSize rl ) const {
        _V_set_reflection_length( rfl, rl );}

    DataSize get_data_size( const Reflection & rfl ) const {
        return _V_get_data_size(rfl); }
    void set_data_size( Reflection & rfl, DataSize ds ) const {
        _V_set_data_size( rfl, ds );}
};

}  // namespace abstract

namespace helpers {

//template<typename T, typename LayoutT>
//struct ReflectionSpecifier;  // generic template instantiation forbidden

template<typename T, typename LayoutT>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type
form_reflection_data_type_descriptor_for(
                     const T & O,
                     typename LayoutT::Reflection & rfl,
                     bool enableHint) {
    rfl.template set_arithmetic_flag<T>();
    // TODO (make hint if necessary) ...
}

template<typename T, typename LayoutT>
typename std::enable_if<std::is_pointer<T>::value, void>::type
form_reflection_data_type_descriptor_for(
                     const T & O,
                     typename LayoutT::Reflection & rfl,
                     bool enableHint) {
    // TODO (array flags setters) ...
}

// TODO (other serializables) ...

}  // namespace helpers

class SerialLayout;
class PrescriptiveLayout;

# endif

} // namespace streaming
} // namespace goo

# endif  // ENABLE_DATASTREAMS
# endif  // H_GOO_STREAMS_SERIALIZED_DATA_LAYOUT_MANAGER_H

