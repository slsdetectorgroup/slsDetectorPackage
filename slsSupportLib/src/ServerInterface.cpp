// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/ServerInterface.h"
#include "sls/logger.h"
#include <cassert>
#include <cstring>
#include <sstream>
namespace sls {

int ServerInterface::sendResult(int ret, void *retval, int retvalSize,
                                char *mess) {

    write(&ret, sizeof(ret));
    if (ret == defs::FAIL) {
        if (mess != nullptr) {
            write(mess, MAX_STR_LENGTH);
        } else {
            LOG(logERROR) << "No error message provided for this "
                             "failure. Will mess up TCP\n";
        }
    } else {
        write(retval, retvalSize);
    }
    return ret;
}

} // namespace sls
