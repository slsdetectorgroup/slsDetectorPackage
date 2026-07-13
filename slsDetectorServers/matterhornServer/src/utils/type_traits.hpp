#include "../MatterhornServerImpl.hpp"
#include "../VirtualMatterhornServerImpl.hpp"

namespace sls {

// forward declares
class MatterhornServer;
class VirtualMatterhornServer;
class MatterhornServerImpl;
class VirtualMatterhornServerImpl;

template <typename DetectorServer> struct implementation_type_trait;

template <> struct implementation_type_trait<MatterhornServer> {
    using ImplType = MatterhornServerImpl;
};

template <> struct implementation_type_trait<VirtualMatterhornServer> {
    using ImplType = VirtualMatterhornServerImpl;
};

} // namespace sls
