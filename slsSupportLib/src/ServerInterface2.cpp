#include "ServerInterface2.h"

namespace sls {

int ServerInterface2::sendResult(bool update, int ret, void *retval,
                                 int retvalSize, char *mess) {

    // if (update && ret == defs::OK && server_->DifferentClients()) {
    //     ret = defs::FORCE_UPDATE;
    // }
    sendData(&ret, sizeof(ret));
    if (ret == defs::FAIL) {
        // send error message
        if (mess)
            sendData(mess, MAX_STR_LENGTH);
        // debugging feature. should not happen.
        else
            FILE_LOG(logERROR) << "No error message provided for this "
                                  "failure. Will mess up TCP\n";
    }
    sendData(retval, retvalSize);
    return ret;
}

int ServerInterface2::receiveArg(int &ret, char *mess, void *arg,
                                 int sizeofArg) {
    if (sizeofArg && receiveData(arg, sizeofArg) < 0)
        return defs::FAIL;
    return defs::OK;
}

} // namespace sls
