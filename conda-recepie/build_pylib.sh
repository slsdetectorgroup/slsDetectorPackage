
# mkdir $PREFIX/lib
# mkdir $PREFIX/include


# #Shared and static libraries
# cp build/bin/_sls_detector* $PREFIX/lib/.


# #Binaries
# cp -r build/bin/sls_detector $PREFIX/lib/.

cd python
${PYTHON} setup.py install