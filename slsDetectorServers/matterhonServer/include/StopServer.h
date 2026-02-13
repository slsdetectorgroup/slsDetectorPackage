#include <cstdint>

// TODO: should this inherit from MatterhornServer?
class StopServer {
  public:
    StopServer(uint16_t port);

    ~StopServer();
};