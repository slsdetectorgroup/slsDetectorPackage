
mkdir $PREFIX/lib
mkdir $PREFIX/bin
mkdir $PREFIX/include

#No libs for gui?

#Binaries
cp build/bin/gui_client $PREFIX/bin/.
cp build/bin/slsDetectorGui $PREFIX/bin/.


#Which headers do we need for development??

# cp include/some_lib.h $PREFIX/include/.