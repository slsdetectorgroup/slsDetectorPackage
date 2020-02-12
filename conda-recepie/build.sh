
echo "|<-------------------- CMAKE setup"
echo "CONDA_PY: $CONDA_PY"
echo "PYTHON_VERSION: $PYTHON_VERSION"
mkdir build
mkdir install
cd build
cmake .. \
      -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
      -DCMAKE_INSTALL_PREFIX=install \
      -DSLS_USE_TEXTCLIENT=ON \
      -DSLS_USE_RECEIVER=ON \
      -DSLS_USE_GUI=ON \
      -DSLS_USE_TESTS=ON \
      -DPYTHON_EXECUTABLE=$CONDA_PREFIX/bin/python
      -DSLS_USE_PYTHON=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DSLS_USE_HDF5=OFF\
     

cmake --build . -- -j10
cmake --build . --target install

CTEST_OUTPUT_ON_FAILURE=1 ctest -j 2