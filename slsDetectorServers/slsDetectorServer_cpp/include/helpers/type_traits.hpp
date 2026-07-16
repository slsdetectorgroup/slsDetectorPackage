#pragma once
#include <type_traits>

namespace sls {

// forward declares
template <bool isStopServer> class MatterhornServer;

template <bool isStopServer> class VirtualMatterhornServer;

template <bool isStopServer> class MatterhornServerImpl;

template <bool isStopServer> class VirtualMatterhornServerImpl;

template <typename DerivedServer> class BaseMatterhornServer;

// type trait to get implementation type

template <typename DetectorServer> struct implementation_type_trait;

template <bool isStopServer>
struct implementation_type_trait<
    BaseMatterhornServer<MatterhornServer<isStopServer>>> {
    using ImplType = MatterhornServerImpl<isStopServer>;
};

template <bool isStopServer>
struct implementation_type_trait<
    BaseMatterhornServer<VirtualMatterhornServer<isStopServer>>> {
    using ImplType = VirtualMatterhornServerImpl<isStopServer>;
};

// type trait to get stop server flag from Detector Server

template <typename DetectorServerImpl>
struct is_stop_server : std::false_type {};

template <>
struct is_stop_server<VirtualMatterhornServerImpl<true>> : std::true_type {};

template <>
struct is_stop_server<MatterhornServerImpl<true>> : std::true_type {};

template <>
struct is_stop_server<VirtualMatterhornServer<true>> : std::true_type {};

template <> struct is_stop_server<MatterhornServer<true>> : std::true_type {};

template <>
struct is_stop_server<BaseMatterhornServer<VirtualMatterhornServer<true>>>
    : std::true_type {};

template <>
struct is_stop_server<BaseMatterhornServer<MatterhornServer<true>>>
    : std::true_type {};

} // namespace sls
