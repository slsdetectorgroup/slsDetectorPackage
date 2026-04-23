#include "MatterhornServer.h"
#include "sls/sls_detector_defs.h"
#include <array>
#include <cstdint>
#include <getopt.h>
#include <string>

namespace sls {

struct DetectorServerOptions {
    // TODO: careful changed for other detectors
    uint16_t port{DEFAULT_TCP_CNTRL_PORTNO};
    /// @brief ignore firmware version compatibility
    bool ignoreFirmwareCompatibility{false};
    /// @brief safe startup - skip initial detector setup and checks
    bool safeStartup{false};
    bool versionRequested{false};
    bool helpRequested{false};
};

template <typename Server>
struct SpecificDetectorServerOptions : DetectorServerOptions {};

// template specialization
// template <>
// struct SpecificDetectorServerOptions<BaseMatterhornServer> {};

// TODO should be a general server specific class or even shared with
// CommandLIneOptions in Receiver
class CommandLineOptions {

  public:
    CommandLineOptions() = default;

    ~CommandLineOptions() = default;

    DetectorServerOptions parse(int argc, char *argv[]);

    std::string printOptions() const;

  private:
    std::string getHelpMessage(const std::string &executable) const;

    uint16_t parsePort(const char *optarg) const;

    void parse_deprecated(const int &opt, char *argv[]);

    DetectorServerOptions detectorserveroptions{};

    static constexpr std::array<option, 8> options{
        {{"help", no_argument, nullptr, 'h'},
         {"version", no_argument, nullptr, 'v'},
         {"port", required_argument, nullptr, 'p'},
         {"ignore_fw_compatibility", no_argument, nullptr,
          'f'}, // ignore firmware compatibility check
         {"safe_startup", no_argument, nullptr, 's'}, // safe startup
         // deprecated options for backward compatibility
         {"devel", no_argument, nullptr, 'd'},  // safe_startup mode
         {"update", no_argument, nullptr, 'u'}, // firmware compatibility check

         {nullptr, 0, nullptr, 0}}};

    inline static const char optstring[] = "hvp:fs"
                                           "du"; // second part is deprecated
};

} // namespace sls