# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

if [ ! -d "build" ]; then
    mkdir build
fi
if [ ! -d "install" ]; then
    mkdir install
fi
cd build
cmake .. -G Ninja \
      -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
      -DCMAKE_INSTALL_PREFIX=install \
      -DSLS_USE_TEXTCLIENT=ON \
      -DSLS_USE_RECEIVER=ON \
      -DSLS_USE_GUI=ON \
      -DSLS_USE_MOENCH=ON\
      -DSLS_USE_TESTS=ON \
      -DSLS_USE_PYTHON=OFF \
      -DCMAKE_BUILD_TYPE=Release \
      -DSLS_USE_HDF5=OFF \
     
NCORES=$(getconf _NPROCESSORS_ONLN)
echo "Building using: ${NCORES} cores"
cmake --build . -- -j${NCORES}
cmake --build . --target install

CTEST_OUTPUT_ON_FAILURE=1 ctest -j 1
