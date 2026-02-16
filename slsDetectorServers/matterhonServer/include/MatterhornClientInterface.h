#include "ClientInterface.h"

namespace sls {

class MatterhornClientInterface : public ClientInterface {

  public:
    MatterhornClientInterface(
        const uint16_t portNumber = DEFAULT_TCP_CNTRL_PORTNO);

    ~MatterhornClientInterface() = default;

  private:
    ReturnCode get_version(ServerInterface &socket);

    ReturnCode get_detector_type(ServerInterface &socket);

    static std::string getMatterhornServerVersion();
};

} // namespace sls