# include "goo_dataflow/processor.hpp"

namespace goo {
namespace dataflow {

void
ValuesMap::_add_value_entry( const std::string & nm
                           , ValueEntry ve ) {
    _values.emplace( nm, ve );
}

}  // namespace goo::dataflow
}  // namespace goo

