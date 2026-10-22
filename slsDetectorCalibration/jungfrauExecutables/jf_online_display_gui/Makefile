# ===========================================================================
# jf_online_display_gui -- single multithreaded program merging
#   jf_event_finder_cli / jf_event_finder_gui  (ZMQ gate)
#   onlinedisp_zmq                             (Qt5 + ROOT display)
#
# Usage:
#   make                        -> ./jf_online_display_gui            (normal)
#                                  ./jf_online_display_gui_JFSTRX     (strip-xel)
#   make normal | make jfstrx    single variant (aliases)
#   ./jf_online_display_gui <rx_host> [rx_port] [gate_threshold] [min_pixels] [pede_file]
#                    [--pub PORT] [--contig] [--no-gate] [--pede-frames N]
#
# Deps: system ROOT (root-config), Qt5 (pkg-config), the prebuilt static
# libzmq from the package build tree, and rapidjson headers.
# ===========================================================================
QT_CXXFLAGS = $(shell pkg-config --cflags Qt5Core Qt5Gui Qt5Widgets 2>/dev/null)
QT_LIBS     = $(shell pkg-config --libs   Qt5Core Qt5Gui Qt5Widgets 2>/dev/null)
ROOT_CFLAGS = $(shell root-config --cflags 2>/dev/null)
ROOT_LIBS   = $(shell root-config --libs   2>/dev/null)

# paths relative to this directory (jf_online_display_gui/)
PKG = ../../..
CAL = ../..

ZMQ_STATIC = $(PKG)/build/_deps/libzmq-build/lib/libzmq.a

# HDF5 C++ headers are NOT in the default include path on every distro (Debian/
# Ubuntu put them in /usr/include/hdf5/serial).  Discover the include/library
# directories with pkg-config (the .pc may be "hdf5" or "hdf5-serial"), then the
# h5c++ wrapper, and let the user override both explicitly:
#   make HDF5_INC=-I/usr/include/hdf5/serial \
#        HDF5_LIB=-L/usr/lib/x86_64-linux-gnu/hdf5/serial
HDF5_PC  := $(shell pkg-config --exists hdf5 && echo hdf5 || (pkg-config --exists hdf5-serial && echo hdf5-serial))
HDF5_INC ?= $(shell pkg-config --cflags $(HDF5_PC) 2>/dev/null) \
            $(shell h5c++ -show 2>/dev/null | tr ' ' '\n' | grep '^-I' | tr '\n' ' ')
HDF5_LIB ?= $(shell pkg-config --libs $(HDF5_PC) 2>/dev/null) \
            $(shell h5c++ -show 2>/dev/null | tr ' ' '\n' | grep '^-L' | tr '\n' ' ')
HDF5_LIBS  = $(HDF5_LIB) -lhdf5_cpp -lhdf5

INCDIR = -I. \
         $(HDF5_INC) \
         -I$(CAL)/dataStructures \
         -I$(CAL)/interpolations \
         -I$(CAL)/tiffio/include \
         -I$(PKG)/slsSupportLib/include \
         -I$(PKG)/slsReceiverSoftware/include \
         -I$(PKG)/libs/rapidjson \
         -I$(PKG)/build/_deps/libzmq-src/include

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -Wall -pthread

LIBS = $(QT_LIBS) $(ROOT_LIBS) $(ZMQ_STATIC) $(HDF5_LIBS) -lsodium -lpthread -lrt -lm

OBJS           = online_jf_core.o online_jf_root.o online_jf_gui.o online_jf_h5.o
# The strip-xel variant compiles the same sources with -DJFSTRX into its own
# objects: the two builds have different geometry and struct layouts, so they
# must never share object files (mixing them is a hard crash).
JFSTRX_OBJS    = online_jf_core.JFSTRX.o online_jf_root.JFSTRX.o \
                 online_jf_gui.JFSTRX.o online_jf_h5.JFSTRX.o

BIN            = jf_online_display_gui
BIN_JFSTRX     = jf_online_display_gui_JFSTRX

ALL_CXXFLAGS = $(CXXFLAGS) $(ROOT_CFLAGS) $(QT_CXXFLAGS) $(INCDIR)

# `make` builds BOTH executables: the normal one and the LGAD strip-xel one
# (the decoder is header-only, no extra library).
all: $(BIN) $(BIN_JFSTRX)

# the strip-xel decoder header is header-only and lives outside this directory:
# touch it and the variant that uses it must be rebuilt
STRIXEL_HDR = $(CAL)/dataStructures/jungfrauLGADStrixelsData_new.h

%.JFSTRX.o: %.cpp online_jf.h $(STRIXEL_HDR)
	$(CXX) $(ALL_CXXFLAGS) -DJFSTRX -c $< -o $@

%.o: %.cpp online_jf.h
	$(CXX) $(ALL_CXXFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

$(BIN_JFSTRX): $(JFSTRX_OBJS)
	$(CXX) -o $@ $(JFSTRX_OBJS) $(LIBS)

# kept so the old habits still work
normal: $(BIN)
jfstrx: $(BIN_JFSTRX)

# Synthetic slsReceiver stand-in used to exercise the program without
# hardware (see test/jf_feeder.cpp).
feeder: test/jf_feeder

test/jf_feeder: test/jf_feeder.cpp
	$(CXX) -std=c++17 -O2 -pthread $(INCDIR) -o $@ $< $(ZMQ_STATIC) -lsodium -lpthread -lrt -lm

clean:
	rm -f $(OBJS) $(JFSTRX_OBJS) $(BIN) $(BIN_JFSTRX) test/jf_feeder

FORCE:

.PHONY: all clean jfstrx normal feeder
