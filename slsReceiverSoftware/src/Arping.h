// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/**
 *@short creates/destroys an ARPing thread to arping the interfaces slsReceiver
is listening to.
 */

#include "ThreadObject.h"

class Arping : private virtual slsDetectorDefs, public ThreadObject {

  public:
    Arping(int ind);
    ~Arping();
    void ClearIpsAndInterfaces();
    void AddInterfacesAndIps(std::string interface, std::string ip);

  private:
    /**
     * Thread Execution for Arping Class
     * Arping interfaces and wait 60 seconds
     */
    void ThreadExecution() override;
    void ExecuteCommands();

    static const std::string ThreadType;
    std::vector<std::string> arpingCommands;
};
