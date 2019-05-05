# include "goo_dataflow/processor.hpp"

namespace goo {
namespace dataflow {

PortInfo::PortInfo( const std::type_info & ti
                  , size_t size_
                  , bool isI, bool isO ) : _typeInfoPtr(&ti)
                                         , _features(size_ << 2) {
    if( isI ) _features |= flag_inputPort;
    if( isO ) _features |= flag_outputPort;
}

void
ValuesMap::_add_value_entry( const std::string & nm
                           , ValueEntry ve ) {
    _values.emplace( nm, ve );
}

}  // namespace goo::dataflow
}  // namespace goo

