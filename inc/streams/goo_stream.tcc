# ifndef H_GOO_STREAMS_STREAM_H
# define H_GOO_STREAMS_STREAM_H

# include <cassert>
# include "goo_resource.tcc"
# include "goo_SDLM.tcc"
# include "goo_storable.hpp"

# ifdef ENABLE_DATASTREAMS

namespace goo {
namespace streaming {

namespace abstract {

class Stream {
private:
    abstract::Resource * _resourcePtr;
    abstract::Layout * _layoutPtr;
protected:
    Layout & _layout() {
        assert(layout_set());
        return *_layoutPtr;
    }
    const Layout & _layout() const {
        assert(layout_set());
        return *_layoutPtr;
    }
    void _layout( abstract::Layout * layoutPtr ) {
        if( layout_set() ) {
            _layout().undock_stream( this );
        }
        _layoutPtr = layoutPtr;
        _layout().dock_stream( this );
    }


    abstract::Resource & _resource() {
        assert(resource_set());
        return *_resourcePtr;
    }
    const abstract::Resource & _resource() const {
        assert(resource_set());
        return *_resourcePtr;
    }
    void _resource( abstract::Resource * resourcePtr ) {
        if( resource_set() ) {
            _resource().undock_stream(this);
        }
        _resourcePtr = resourcePtr;
        _resource().dock_stream(this);
    }
public:
    virtual ~Stream(){}

    /// Returns true, if layout instance is set.
    bool layout_set() const { return _layoutPtr; }
    /// Returns true, if resource instance is set.
    bool resource_set() const { return _resourcePtr; }

    template<typename TargetResourceT>
    TargetResourceT & resource() {
        return streaming_infrastructure_cast<TargetResourceT&>( _resource() );
    }

    template<typename TargetResourceT>
    const TargetResourceT & resource() const {
        return streaming_infrastructure_cast<const TargetResourceT&>( _resource() );
    }

    template<typename TargetLayoutT>
    TargetLayoutT & layout() {
        return streaming_infrastructure_cast<TargetLayoutT&>( _layout() );
    }

    template<typename TargetLayoutT>
    const TargetLayoutT & layout() const {
        return streaming_infrastructure_cast<const TargetLayoutT&>( _layout() );
    }
};

template<Direction dir,
         typename DataSizeT>
class PlainStream;  // generic template instantiation forbidden;

template<typename DataSizeT>
class PlainStream<in, DataSizeT> : public Stream {
public:
    typedef DataSizeT DataSize;
    typedef Stream Parent;
public:
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, void>::type
    write( const T data ) {
        Parent::layout()
            .write_arithmetic( Parent::resource< ResourcePlain<in, DataSize> >(), data );
    }

    template<typename T>
    typename std::enable_if<std::is_base_of<iSerializable, T>::value, void>::type
    write( const T & entity ) {
        Parent::layout()
            .write_serializable( Parent::resource< ResourcePlain<in, DataSize> >(), entity );
    }
    // TODO: (arrays) ...
};  // class PlainStream<in>

# if 0
template<Direction dir,
         typename LayoutT>
class ReflexiveStream;  // generic template instantiation forbidden

template<typename LayoutT>
class ReflexiveStream<in, LayoutT> : public virtual PlainStream<LayoutT> {
public:
    typedef LayoutT Layout;
    typedef typename Layout::Reflection Reflection;
    typedef PlainStream<Layout> Parent;
public:
    template<typename T>
    write( const T & entity ) {
        Reflection rfl;
        Parent::layout().new_reflection( rfl );
        helpers::form_reflection_data_type_descriptor_for<T, Layout>( entity, rfl, Parent::hints_enabled() );

        //PositionIndex rIdx = Parent::layout_manager().get_reflection_position( rfl ),
        //              dIdx = Parent::layout_manager().get_data_position( rfl );
        auto rsrcRef = Parent::template resource< ResourcePlain<in, typename Layout::DataSize> >();
        rsrcRef.reserve(
                Parent::layout_manager().get_reflection_length( rfl ) +
                Parent::layout_manager().get_data_size( rfl )
            );
        //Parent::resource< ResourcePlain<in, SDLM::DataSize> >().write_block(
        //        rfl
        //    );

    }
};
# endif


}  // namespace abstract


}  // namespace streaming
}  // namespace goo

# endif  // ENABLE_DATASTREAMS
# endif  // H_GOO_STREAMS_STREAM_H

