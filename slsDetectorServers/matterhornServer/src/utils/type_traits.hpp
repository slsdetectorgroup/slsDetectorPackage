#pragma once
#include <type_traits>

namespace sls {

// forward declares
template <bool isStopServer> class MatterhornServer;

template <bool isStopServer> class VirtualMatterhornServer;

template <bool isStopServer> class MatterhornServerImpl;

template <bool isStopServer> class VirtualMatterhornServerImpl;

template <typename DetectorServer> struct implementation_type_trait;

template <bool isStopServer>
struct implementation_type_trait<MatterhornServer<isStopServer>> {
    using ImplType = MatterhornServerImpl<isStopServer>;
};

template <bool isStopServer>
struct implementation_type_trait<VirtualMatterhornServer<isStopServer>> {
    using ImplType = VirtualMatterhornServerImpl<isStopServer>;
};

template <typename DetectorServerImpl>
struct is_stop_server : std::false_type {};

template <>
struct is_stop_server<VirtualMatterhornServerImpl<true>> : std::true_type {};

template <>
struct is_stop_server<MatterhornServerImpl<true>> : std::true_type {};

} // namespace sls
