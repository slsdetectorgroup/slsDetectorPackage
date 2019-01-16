#include "gitInfoLib.h"
#include "multiSlsDetectorClient.h"
#include <cstdlib>

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (!(strcmp(argv[i], "--version")) || !(strcmp(argv[i], "-v"))) {
            int64_t tempval = GITDATE;
            std::cout << argv[0] << " " << GITBRANCH << " (0x" << std::hex << tempval << ")" << std::endl;
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

    if (argc > 1)
        argv++;
    multiSlsDetectorClient(argc - 1, argv, action);
}
