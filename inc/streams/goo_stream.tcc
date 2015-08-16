# ifndef H_GOO_STREAMS_STREAM_H
# define H_GOO_STREAMS_STREAM_H

# include <cassert>
# include "goo_resource.tcc"
# include "goo_SDLM.tcc"

# ifdef ENABLE_DATASTREAMS

namespace goo {
namespace streaming {

namespace abstract {

class Stream {
private:
    abstract::Resource * _resourcePtr;
protected:
    abstract::Resource & _resource() {
        assert(resource_set());
        return *_resourcePtr;
    }
    const abstract::Resource & _resource() const {
        assert(resource_set());
        return *_resourcePtr;
    }
    void _resource( abstract::Resource * resourcePtr ) {
        _resourcePtr = resourcePtr;
    }
public:
    bool resource_set() const { return _resourcePtr; }
    virtual ~Stream(){}

    template<typename TargetResourceT>
    TargetResourceT & resource() {
        return streaming_infrastructure_cast<TargetResourceT&>( _resource() );
    }

    template<typename TargetResourceT>
    const TargetResourceT & resource() const {
        return streaming_infrastructure_cast<const TargetResourceT&>( _resource() );
    }
};

template<typename SDLMT>
class ReflexiveStreamBase : public virtual Stream {
public:
    typedef SDLMT SDLM;
private:
    SDLM * _sdlmPtr;
    bool _hintsEnabled;
protected:
    // Note: should be available as public only for in/bidir streams.
    void _enable_hints() { _hintsEnabled = true; }
    void _disable_hints() { _hintsEnabled = false; }
public:
    bool layout_manager_set() const {
        return _sdlmPtr;
    }
    SDLM & layout_manager() {
        assert(layout_manager_set());
        return *_sdlmPtr;
    }
    const SDLM & layout_manager() const {
        assert(layout_manager_set());
        return *_sdlmPtr;
    }
    bool hints_enabled() const { return _hintsEnabled; }
};

template<Direction dir,
         typename SDLMT>
class ReflexiveStream;  // generic template instantiation forbidden

template<typename SDLMT>
class ReflexiveStream<in, SDLMT> : public virtual ReflexiveStreamBase<SDLMT> {
public:
    typedef SDLMT SDLM;
    typedef typename SDLM::Reflection Reflection;
    typedef ReflexiveStreamBase<SDLMT> Parent;
public:
    template<typename T>
    typename std::enable_if<std::is_same<SerialLayout, SDLM>::value, void>::type
    write( const T & entity ) {
        Reflection rfl;
        Parent::layout_manager().new_reflection( rfl );
        helpers::form_reflection_data_type_descriptor_for<T, SDLM>( entity, rfl, Parent::hints_enabled() );

        //PositionIndex rIdx = Parent::layout_manager().get_reflection_position( rfl ),
        //              dIdx = Parent::layout_manager().get_data_position( rfl );
        auto rsrcRef = Parent::template resource< ResourcePlain<in, typename SDLM::DataSize> >();
        rsrcRef.reserve(
                Parent::layout_manager().get_reflection_length( rfl ) +
                Parent::layout_manager().get_data_size( rfl )
            );
        //Parent::resource< ResourcePlain<in, SDLM::DataSize> >().write_block(
        //        rfl
        //    );
        // TODO: what to do with hints?
    }
};


}  // namespace abstract


}  // namespace streaming
}  // namespace goo

# endif  // ENABLE_DATASTREAMS
# endif  // H_GOO_STREAMS_STREAM_H

