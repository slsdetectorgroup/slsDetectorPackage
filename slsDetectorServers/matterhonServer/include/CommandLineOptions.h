#include "MatterhornServer.h"
#include "sls/sls_detector_defs.h"
#include <array>
#include <cstdint>
#include <getopt.h>
#include <string>

namespace sls {

struct DetectorServerOptions {
    uint16_t port{DEFAULT_TCP_CNTRL_PORTNO};
    bool isControlServer{true};
    bool debugflag{false};
    bool updateFlag{false};
    bool checkModuleFlag{true};
    bool versionRequested{false};
    bool helpRequested{false};
};

// TODO should be a general server specific class or even shared with
// CommandLIneOptions in Receiver
class CommandLineOptions {

  public:
    CommandLineOptions() = default;

    ~CommandLineOptions() = default;

    DetectorServerOptions parse(int argc, char *argv[]);

    std::string getHelpMessage(const std::string &executable) const;

    std::string getVersion() const;

    uint16_t parsePort(const char *optarg) const;

  private:
    static constexpr std::array<option, 6> options{
        {{"help", no_argument, nullptr, 'h'},
         {"version", no_argument, nullptr, 'v'},
         {"port", required_argument, nullptr, 'p'},
         {"devel", no_argument, nullptr, 'd'},
         {"update", no_argument, nullptr, 'u'},
         {nullptr, 0, nullptr, 0}}};

    inline static const std::string optstring{"hvp:du"};
};

} // namespace sls