# include "goo_dataflow/processor.hpp"

namespace goo {
namespace dataflow {

void
Link::maps( const std::string & fn
          , const std::string & tn ) {
    fromName = fromName;
    toName = toName;
}

void
ValuesMap::_add_value_entry( const std::string & nm
                           , ValueEntry ve ) {
    _values.emplace( nm, ve );
}

}  // namespace goo::dataflow
}  // namespace goo

