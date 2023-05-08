// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef JUNGFRAUMODULEDATA_H
#define JUNGFRAUMODULEDATA_H
#include <cstdint>
#include "slsDetectorData.h"

//#define VERSION_V2
/**
    @short  structure for a Detector Packet or Image Header
    @li frameNumber is the frame number
    @li expLength is the subframe number (32 bit eiger) or real time exposure
   time in 100ns (others)
    @li packetNumber is the packet number
    @li bunchId is the bunch id from beamline
    @li timestamp is the time stamp with 10 MHz clock
    @li modId is the unique module id (unique even for left, right, top, bottom)
    @li xCoord is the x coordinate in the complete detector system
    @li yCoord is the y coordinate in the complete detector system
    @li zCoord is the z coordinate in the complete detector system
    @li debug is for debugging purposes
    @li roundRNumber is the round robin set number
    @li detType is the detector type see :: detectorType
    @li version is the version number of this structure format
*/
typedef struct {
    uint64_t bunchNumber; /**< is the frame number */
    uint64_t pre;         /**< something */

} jf_header;

using namespace std;  
class jungfrauModuleData : public slsDetectorData<uint16_t> {

  private:
    int iframe;

  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

   */
#ifndef ZMQ
#define off sizeof(jf_header)
#endif
#ifdef ZMQ
#define off 0
#endif


    jungfrauModuleData()
        : slsDetectorData<uint16_t>(1024, 512,
                                    1024* 512 * 2 + off) {
  
        for (int ix = 0; ix < 1024; ix++) {
            for (int iy = 0; iy < 512; iy++) {
                dataMap[iy][ix] = off + (1024 * iy + ix) * 2;
#ifdef HIGHZ
                dataMask[iy][ix] = 0x3fff;
#endif
            }
        }




        iframe = 0;
        //  cout << "data struct created" << endl;
    };

    /**
       Returns the value of the selected channel for the given dataset as
       double. \param data pointer to the dataset (including headers etc) \param
       ix pixel number in the x direction \param iy pixel number in the y
       direction \returns data for the selected channel, with inversion if
       required as double

    */
    virtual double getValue(char *data, int ix, int iy = 0) {

        uint16_t val = getChannel(data, ix, iy) & 0x3fff;
	/* if (ix==0 && iy==0) */
	/*   cout << val << endl; */
        return val;
    };

   

    /**

    Returns the frame number for the given dataset. Purely virtual func.
    \param buff pointer to the dataset
    \returns frame number

 */

    /* class jfrau_packet_header_t { */
    /*  public: */
    /* 	unsigned char reserved[4]; */
    /* 	unsigned char packetNumber[1]; */
    /* 	unsigned char frameNumber[3]; */
    /* 	unsigned char bunchid[8]; */
    /* }; */

    int getFrameNumber(char *buff) {
        return ((jf_header *)buff)->bunchNumber;
    };
  
    /**

       Returns the packet number for the given dataset. purely virtual func
       \param buff pointer to the dataset
       \returns packet number number



    */
    int getPacketNumber(char *buff) {
        return 0;
    };

   
    char *readNextFrame(ifstream &filebin) {
        int ff = -1, np = -1;
        return readNextFrame(filebin, ff, np);
    };

    char *readNextFrame(ifstream &filebin, int &ff) {
        int np = -1;
        return readNextFrame(filebin, ff, np);
    };

    char *readNextFrame(ifstream &filebin, int &ff, int &np) {
        char *data = new char[dataSize];
        char *d = readNextFrame(filebin, ff, np, data);
        if (d == NULL) {
            delete[] data;
            data = NULL;
        }
        return data;
    };

    char *readNextFrame(ifstream &filebin, int &ff, int &np,char *data) {
      //char *retval = 0;
      //int nd;
      //int fnum = -1;
        np = 0;
        //int pn;

        //  cout << dataSize << endl;
        if (ff >= 0)
	  //fnum = ff;

        if (filebin.is_open()) {
            if (filebin.read(data, dataSize)) {
                ff = getFrameNumber(data);
                np = getPacketNumber(data);
                return data;
            }
        }
        return NULL;
    };

    /**

       Loops over a memory slot until a complete frame is found (i.e. all
       packets 0 to nPackets, same frame number). purely virtual func \param
       data pointer to the memory to be analyzed \param ndata reference to the
       amount of data found for the frame, in case the frame is incomplete at
       the end of the memory slot \param dsize size of the memory slot to be
       analyzed \returns pointer to the beginning of the last good frame (might
       be incomplete if ndata smaller than dataSize), or NULL if no frame is
       found

    */
    virtual char *findNextFrame(char *data, int &ndata, int dsize) {
        if (dsize < dataSize)
            ndata = dsize;
        else
            ndata = dataSize;
        return data;
    };

    // int getPacketNumber(int x, int y) {return dataMap[y][x]/packetSize;};
};

#endif
