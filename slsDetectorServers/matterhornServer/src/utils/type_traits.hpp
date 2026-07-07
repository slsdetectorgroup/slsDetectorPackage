#include "../MatterhornServerImpl.hpp"
#include "../VirtualMatterhornServerImpl.hpp"

namespace sls {

// forward declares
class MatterhornServer;
class VirtualMatterhornServer;
class MatterhornServerImpl;
class VirtualMatterhornServerImpl;

template <typename DetectorServer> struct implementation_typetrait;

template <> struct implementation_typetrait<MatterhornServer> {
    using ImplType = MatterhornServerImpl;
};

template <> struct implementation_typetrait<VirtualMatterhornServer> {
    using ImplType = VirtualMatterhornServerImpl;
};

} // namespace sls
