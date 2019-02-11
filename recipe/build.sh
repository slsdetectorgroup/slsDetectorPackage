mkdir build
mkdir install
cd build
cmake .. \
      -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
      -DCMAKE_INSTALL_PREFIX=install \
      -DSLS_USE_TEXTCLIENT=ON \
      -DSLS_USE_RECEIVER=ON \
      -DSLS_USE_GUI=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DSLS_USE_HDF5=OFF\
     

cmake --build . -- -j10
cmake --build . --target install