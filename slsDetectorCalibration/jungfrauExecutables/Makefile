ROOTSYS=/opt/cern/v6/build

LIBZMQDIR = $(PWD)
LIBZMQ =  -L$(LIBZMQDIR) -lzmq
SHLIB_PATH=/opt/cern/v6/root/lib
CMAKE_PREFIX_PATH=/opt/cern/v6/root
DYLD_LIBRARY_PATH=/opt/cern/v6/root/lib

QT_CXXFLAGS = $(shell pkg-config --cflags Qt5Core Qt5Gui Qt5Widgets 2>/dev/null)
QT_LIBS     = $(shell pkg-config --libs Qt5Core Qt5Gui Qt5Widgets 2>/dev/null)

INCDIR=   -I. -I../dataStructures ../tiffio/src/tiffIO.cpp -I../ -I../interpolations/ -I../../slsSupportLib/include/ -I../../slsReceiverSoftware/include/ -I../../libs/rapidjson/ -I../tiffio/include
LDFLAG= -L/usr/lib64/ -lpthread -lm  -lstdc++  -lzmq -pthread -lrt -ltiff   -O3  -std=c++17 -Wall -Wl,-rpath-link,$(ROOTSYS)/lib


default: onlinedisp_zmq

onlinedisp_zmq: onlinedisp_zmq.cpp onlinedisp_zmq.h ../dataStructures/jungfrauLGADStrixelsData_new.h
	g++ -o onlinedisp_zmq onlinedisp_zmq.cpp $(QT_CXXFLAGS) $(QT_LIBS) -I. -I$(ROOTSYS)/include -Wall -g -lm -L. -lzmq -pthread -lrt -L$(ROOTSYS)/lib -lGui -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -m64 -L../../build/bin/ -lSlsSupportStatic ../../build/_deps/libzmq-build/lib/libzmq.a -lstdc++ -lsodium $(LDFLAG) $(INCDIR)

#-lCint

