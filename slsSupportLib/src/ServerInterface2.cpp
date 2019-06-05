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
            write(mess, MAX_STR_LENGTH);
        } else {
            FILE_LOG(logERROR) << "No error message provided for this "
                                  "failure. Will mess up TCP\n";
        }
    } else {
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
        std::ostringstream ss;
        ss << "TCP socket error read " << bytes_read << " bytes instead of "
           << sizeofArg << " bytes";

        throw sls::SocketError(ss.str());
    }
}

} // namespace sls
