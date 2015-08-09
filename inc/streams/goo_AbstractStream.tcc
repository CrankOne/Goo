# ifndef H_GOO_DATA_STREAMS_ABSTRACT_STREAM_H
# define H_GOO_DATA_STREAMS_ABSTRACT_STREAM_H

# include "streams/goo_iSDLM.tcc"

namespace goo {
namespace streams {

template< Direction dirT,
          Reflexivity rflT,
          typename IdxT>
class AbstractStream {
public:
    typedef AbstractStreamBase Parent;
    typedef IdxT Indexes;
    typedef iSDLM<dirT, rflT, Indexes> iSDLMType;
protected:
    iSDLMType * _layoutManager;
public:
    /// Returns true when layout manager is set.
    bool layout_manager_set() { return _layoutManager; }

    /// Sets the layout manager.
    void layout_manager( iSDLMType & manager ) { _layoutManager = &manager; }

    /// Returns layout manager (raises badState if unset).
    iSDLMType & layout_manager();  // TODO

    /// Const version of layout manager getter.
    const iSDLMType & layout_manager() const;  // TODO
};

template<Direction dirT,
         Reflexivity rflT,
         typename IdxT=Size>
class Stream {
    // empty generic template
    static_assert(true, "Generic Stream template instantiated.");
};


template<typename IdxT>
class Stream<in, plain, IdxT> :
    public AbstractStream<in, plain, IdxT> {
public:
    virtual IdxT put( Size length, UByte * data ) {
        return this->layout_manager().put(length, data); }
};

template<typename IdxT>
class Stream<out, plain, IdxT> :
    public AbstractStream<out, plain, IdxT> {
public:
    virtual IdxT get( Size length, UByte ** data ) {
        return this->layout_manager().get(length, data); }
};

template<typename IdxT>
class Stream<bidir, plain, IdxT> {
    static_assert(true, "Bidirectional streams unimplemented in current revision.");
};

template<Reflexivity rflT,
         typename IdxT>
class Stream<in, rflT, IdxT> {
    // TODO
};

template<Reflexivity rflT,
         typename IdxT>
class Stream<out, rflT, IdxT> {
    // TODO
};

template<Reflexivity rflT,
         typename IdxT>
class Stream<bidir, rflT, IdxT> {
    static_assert(true, "Bidirectional streams unimplemented in current revision.");
};

}  // namespace streams
}  // namespace goo

# endif  // H_GOO_DATA_STREAMS_ABSTRACT_STREAM_H

