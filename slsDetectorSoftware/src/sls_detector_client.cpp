#include "multiSlsDetectorClient.h"
#include "sls_detector_defs.h"
#include "versionAPI.h"

#include <cstring> //strcmp

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (!(strcmp(argv[i], "--version")) || !(strcmp(argv[i], "-v"))) {
            int64_t tempval = APILIB;
            std::cout << argv[0] << " " << GITBRANCH << " (0x" << std::hex
                      << tempval << ")" << std::endl;
            return 0;
        }
    }

#ifdef PUT
    int action = slsDetectorDefs::PUT_ACTION;
#endif

#ifdef GET
    int action = slsDetectorDefs::GET_ACTION;
#endif

#ifdef READOUT
    int action = slsDetectorDefs::READOUT_ACTION;
#endif

#ifdef HELP
    int action = slsDetectorDefs::HELP_ACTION;
#endif

    try {
        multiSlsDetectorClient(argc, argv, action);
    } catch (const sls::RuntimeError &e) {
    }
}
