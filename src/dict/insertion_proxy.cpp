# include "dict/goo_insertion_proxy.hpp"
# include "dict/goo_dict.hpp"

namespace goo {
namespace dicts {

InsertionProxy::InsertionProxy( Dictionary & d ) : _dict(d) {
}

InsertionProxy
InsertionProxy::bgn_sect(
            const std::string & name,
            const std::string & descr ) {
    _dict.open_subsection( name, descr );
    // ... ?
}


InsertionProxy
InsertionProxy::end_sect(
            const std::string & name,
            const std::string & descr ) {
    _dict.close_subsection( name, descr );
    // ... ?
}

}  // namespace goo
}  // namespace dicts


