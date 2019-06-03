#include "ServerInterface2.h"
#include <cassert>
#include <cstring>
#include <sstream>
namespace sls {

int ServerInterface2::sendResult(int ret, void *retval, int retvalSize,
                                 char *mess) {

    write(&ret, sizeof(ret));
    if (ret == defs::FAIL) {
        if (mess != nullptr) {
            sendData(mess, MAX_STR_LENGTH);
        } else {
            FILE_LOG(logERROR) << "No error message provided for this "
                                  "failure. Will mess up TCP\n";
        }
    }else{
        write(retval, retvalSize);
    }
    return ret;
}

int ServerInterface2::receiveArg(void *arg, int sizeofArg) {
    assert(sizeofArg > 0);
    int bytes_read = read(arg, sizeofArg);
    if (bytes_read == sizeofArg) {
        return defs::OK;
    } else {
        // We did not read the expected number of bytes
        int ret = defs::FAIL;
        char errormsg[MAX_STR_LENGTH]{};
        std::ostringstream ss;
        if (bytes_read == -1) {
            ss << "TCP socket read returned -1, possible socket crash";
            // TODO! test this!
        } else {
            ss << "TCP socket error read " << bytes_read << " bytes instead of "
               << sizeofArg << " bytes";
        }
        strcpy(errormsg, ss.str().c_str());
        sendData(&ret, sizeof(ret));
        sendData(errormsg, sizeof(errormsg));
        throw sls::SocketError(ss.str());
        return defs::FAIL;
    }
}

} // namespace sls
