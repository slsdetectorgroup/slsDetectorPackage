
mkdir $PREFIX/lib
mkdir $PREFIX/bin
mkdir $PREFIX/include
mkdir $PREFIX/include/slsDetectorPackage

#Shared and static libraries
# cp build/bin/libSlsDetector.so $PREFIX/lib/.
# cp build/bin/libSlsReceiver.so $PREFIX/lib/.
# cp build/bin/libSlsSupport.so $PREFIX/lib/.

cp build/install/lib/* $PREFIX/lib/

#Binaries
cp build/install/bin/sls_detector_acquire $PREFIX/bin/.
cp build/install/bin/sls_detector_get $PREFIX/bin/.
cp build/install/bin/sls_detector_put $PREFIX/bin/.
cp build/install/bin/sls_detector_help $PREFIX/bin/.
cp build/install/bin/slsReceiver $PREFIX/bin/.
cp build/install/bin/slsMultiReceiver $PREFIX/bin/.


cp build/install/include/* $PREFIX/include/
cp -r build/install/share/ $PREFIX/share
