// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2025 Contributors to the SLS Detector Package
/************************************************
 * @file test-ReceiverActions.cpp
 * @short test cases to test receiver actions
 ***********************************************/

#include "DataProcessor.h"
#include "GeneralData.h"
#include "catch.hpp"
#include <vector>

namespace sls {

TEST_CASE("cannot set analog/digital and transceiver samples when "
          "reciever is running",
          "[.recieveraction][.chiptestboard]") {}

} // namespace sls