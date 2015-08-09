# ifndef H_GOO_DATA_STREAMS_STREAM_DATA_LAYOUT_MANAGER_H
# define H_GOO_DATA_STREAMS_STREAM_DATA_LAYOUT_MANAGER_H

# include "streams/goo_AbstractStreamBase.hpp"
# include "goo_utility.hpp"

namespace goo {
namespace streams {

template<typename posIdxT>
class iSDLM_inBase {
protected:
    virtual posIdxT _put( Size length, const UByte * data ) = 0;
public:
    posIdxT put( Size length, const UByte * data ) {
        return _put(length, data); }
};


template<typename posIdxT>
class iSDLM_outBase {
protected:
    virtual posIdxT _get( Size length, UByte ** data ) = 0;
public:
    posIdxT get( Size length, UByte ** data ) {
        return _get(length, data); }
};

template<typename posIdxT>
class iSDLM_positionalBase {
protected:
    virtual void _set_position_to( const posIdxT & ) = 0;
    virtual posIdxT _get_position() const = 0;
public:
    void set_position_to( const posIdxT & i ) {
        _set_position_to( i ); }
    posIdxT get_position() const {
        return _get_position(); }
};

template<Direction dirT,
         Reflexivity rflT,
         typename posIdxT>
class iSDLM {
    // empty generic template
    static_assert(true, "Generic Stream Data Layout Manager template instantiated.");
};



template<typename posIdxT>
class iSDLM< in, plain, posIdxT > :
        public iSDLM_inBase<posIdxT> { };

template<typename posIdxT>
class iSDLM< out, plain, posIdxT > :
        public iSDLM_outBase<posIdxT> { };

template<typename posIdxT,
         Reflexivity rflT>
class iSDLM< in, rflT, posIdxT > :
        public iSDLM_inBase<posIdxT>,
        public iSDLM_positionalBase<posIdxT> { };

template<typename posIdxT,
         Reflexivity rflT>
class iSDLM< out, rflT, posIdxT > :
        public iSDLM_outBase<posIdxT>,
        public iSDLM_positionalBase<posIdxT> { };

}  // namespace streams
}  // namespace goo

# endif  // H_GOO_DATA_STREAMS_STREAM_DATA_LAYOUT_MANAGER_H

