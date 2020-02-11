
# mkdir $PREFIX/lib
# mkdir $PREFIX/include


# #Shared and static libraries
# cp build/bin/_sls_detector* $PREFIX/lib/.


# #Binaries
# cp -r build/bin/sls_detector $PREFIX/lib/.

#copy shared lib 
echo "|<-------- starting python build"
cp build/bin/_sls_detector* python/.

cd python
python setup.py install
# $PYTHON setup.py install 