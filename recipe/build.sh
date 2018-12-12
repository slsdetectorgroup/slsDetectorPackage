mkdir build
cd build
cmake .. \
      -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
      -DCMAKE_INSTALL_PREFIX=$PREFIX \
      -DUSE_TEXTCLIENT=ON \
      -DUSE_RECEIVER=ON \
      -DUSE_GUI=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DUSE_HDF5=OFF\
     

cmake --build . -- -j8