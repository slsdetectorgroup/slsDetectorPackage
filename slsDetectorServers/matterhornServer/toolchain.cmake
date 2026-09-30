#Usage: cmake .. -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -DSLS_BUILD_ONLY_MATTERHORN=ON

set(SLS_ARM_COMPILER "/psi.ch/group/detector/firmware/arm64_linux/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/" CACHE PATH "Path to the ARM cross compiler")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64) 

set(CMAKE_C_COMPILER "${SLS_ARM_COMPILER}/bin/aarch64-none-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${SLS_ARM_COMPILER}/bin/aarch64-none-linux-gnu-g++")

set(CMAKE_AR "${SLS_ARM_COMPILER}/bin/aarch64-none-linux-gnu-ar")
set(CMAKE_RANLIB "${SLS_ARM_COMPILER}/bin/aarch64-none-linux-gnu-ranlib")

set(CMAKE_FIND_ROOT_PATH "${SLS_ARM_COMPILER}")
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

