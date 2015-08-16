# ifndef H_GOO_STREAMS_BASES_H
# define H_GOO_STREAMS_BASES_H

# include "goo_types.h"
# include "goo_utility.hpp"

# ifdef ENABLE_DATASTREAMS

# ifdef GOO_NO_DEBUG_STREAMING_INFRASTRUCTURE
#   define streaming_infrastructure_cast static_cast
# else
#   define streaming_infrastructure_cast dynamic_cast
# endif

namespace goo {
namespace streaming {

enum Direction : UByte {
       in = 1,
      out = 2,
    bidir = 3,
};

// ft: Resource; determines data layout strategy (which SDLM to inherit)
enum Layout : UByte {
     serial = 1,
     random = 2,
};

// ft: Stream; determines introspection data placement strategy
enum Reflexivity : UByte {
          plain = 1,
     positional = 2,  // xxx?
      reflexive = 3,
};

namespace abstract {

class ResourceBase {
public:
    virtual ~ResourceBase(){}
};

//class InputStreamingIterator;
//class OutputStreamingIterator;

class StreamBase {
private:
    ResourceBase * _resourcePtr;
    
protected:
    void _set_abstract_resource_ptr( ResourceBase * ptr ) { _resourcePtr = ptr; }
    ResourceBase * _get_abstract_resource_ptr() { return _resourcePtr; }
    const ResourceBase * _get_abstract_resource_ptr() const { return _resourcePtr; }
public:
    virtual ~StreamBase(){}
};

}  // namespace abstract

}  // namespace streaming
}  // namespace goo

# endif  // ENABLE_DATASTREAMS
# endif  // H_GOO_STREAMS_BASES_H

