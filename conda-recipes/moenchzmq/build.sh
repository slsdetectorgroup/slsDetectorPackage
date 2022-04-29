# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

mkdir build

cd build
cmake .. \
      -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
      -DCMAKE_INSTALL_PREFIX=$PREFIX \
      -DSLS_USE_MOENCH=ON\
      -DSLS_USE_TESTS=ON \
      -DSLS_EXT_BUILD=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DSLS_USE_HDF5=OFF\
     
NCORES=$(getconf _NPROCESSORS_ONLN)
echo "Building using: ${NCORES} cores"
cmake --build . -- -j${NCORES}
cmake --build . --target install

CTEST_OUTPUT_ON_FAILURE=1 ctest -j 2

#Clean up for the next build
# cd ..
# rm -rf build