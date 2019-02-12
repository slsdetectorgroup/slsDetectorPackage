
mkdir $PREFIX/lib
mkdir $PREFIX/bin
mkdir $PREFIX/include
mkdir $PREFIX/include/slsDetectorPackage

#Shared and static libraries
cp build/bin/libSlsDetector.so $PREFIX/lib/.
# cp build/bin/libSlsDetector.a $PREFIX/lib/.
cp build/bin/libSlsReceiver.so $PREFIX/lib/.
# cp build/bin/libSlsReceiver.a $PREFIX/lib/.

#Binaries
cp build/bin/sls_detector_acquire $PREFIX/bin/.
cp build/bin/sls_detector_get $PREFIX/bin/.
cp build/bin/sls_detector_put $PREFIX/bin/.
cp build/bin/sls_detector_help $PREFIX/bin/.
cp build/bin/slsReceiver $PREFIX/bin/.
cp build/bin/slsMultiReceiver $PREFIX/bin/.

#Which headers do we need for development??
cp build/install/include/* $PREFIX/include/slsDetectorPackage/
# cp include/some_lib.h $PREFIX/include/.