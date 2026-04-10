#include "MatterhornServer.h"
#include <cstdint>

namespace sls {

// TODO: should this inherit from MatterhornServer or a base class - depending on virtual or not 
class StopServer : public MatterhornServer {
  public:
    StopServer(uint16_t port);

    ~StopServer() = default;
};

} // namespace sls