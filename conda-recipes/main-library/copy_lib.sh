# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

mkdir -p $PREFIX/lib
mkdir -p $PREFIX/bin
mkdir -p $PREFIX/include/sls

#Shared and static libraries
cp build/install/lib/* $PREFIX/lib/

#Binaries
cp build/install/bin/sls_detector_acquire $PREFIX/bin/.
cp build/install/bin/sls_detector_acquire_zmq $PREFIX/bin/.
cp build/install/bin/sls_detector_get $PREFIX/bin/.
cp build/install/bin/sls_detector_put $PREFIX/bin/.
cp build/install/bin/sls_detector_help $PREFIX/bin/.
cp build/install/bin/sls_detector $PREFIX/bin/.
cp build/install/bin/slsReceiver $PREFIX/bin/.
cp build/install/bin/slsMultiReceiver $PREFIX/bin/.
cp build/install/bin/slsFrameSynchronizer $PREFIX/bin/.


cp build/install/include/sls/* $PREFIX/include/sls
cp -rv build/install/share $PREFIX
