// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef JUNGFRAULGADSTRIXELDATA_H
#define JUNGFRAULGADSTRIXELDATA_H
#ifdef CINT
#include "sls/sls_detector_defs_CINT.h"
#else
#include "sls/sls_detector_defs.h"
#endif
#include "slsDetectorData.h"

/*
/afs/psi.ch/project/mythen/Anna/slsDetectorPackageDeveloperMpc2011/slsDetectorCalibration/jungfrauExecutables 
make -f Makefile.rawdataprocess jungfrauRawDataProcessStrx
 ../dataStructures/jungfrauLGADStrixelsData.h
*/
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

} jf_header; //Aldo's header


using namespace std;    
class jungfrauLGADStrixelsData : public slsDetectorData<uint16_t> {

  private:
    int iframe;

#ifdef ALDO //VH
    using header = jf_header; //VH
#else //VH
    using header = sls::defs::sls_receiver_header;
#endif //VH
  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

    */

    int groupmap[512*5][1024/5];

 
    jungfrauLGADStrixelsData()
        : slsDetectorData<uint16_t>(1024/5, 512*5,
                                    512 * 1024 * 2 + sizeof(header)) {
      //      cout << "aaa" << endl;
#ifdef ALDO //VH
      cout<< "using reduced jf_header" << endl; //VH
#endif //VH
        for (int ix = 0; ix < 1024/5; ix++) {
            for (int iy = 0; iy < 512*5; iy++) {
	      dataMap[iy][ix] = sizeof(header);//+ ( 1024 * 5 + 300) * 2; //somewhere on the guardring of the LGAD
	      groupmap[iy][ix]=-1;
#ifdef HIGHZ
                dataMask[iy][ix] = 0x3fff;
#endif
            }
        }
        
	int x0=256+10, x1=256+246;
	int y0=10, y1=256-10;
	int ix,iy;
	int ox=0, oy=0, ooy=0;
	ox=2;//0; //original value is 0
	cout << "G0" << endl;

	//chip1
	/*
	 * TL;DR comments: y0 is too high by 1, group 1 and group 2 are by one row too short
	 * group34 by 2 rows, x is by one column too short
	 * NOTE: If x0, x1, y0 are changed, likely also ox (and oy and ooy) will be affected!
	 */

	cout << "G0" << endl; //chip coordinates of chip1 group1: x=255+10 to x=255+246, y=10 to y=64
	//9 pixels guard ring, bonding shift by one pixel in y, one square pixel in x on the left

	int x0=256+10, x1=256+246; //excludes first column (in chip coordinates)
	int y0=10, y1=256-10; //y1 does nothing
	int ix,iy;
	int ox=0, oy=0, ooy=0;
	ox=0;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0; ipy<y0+54; ipy++) { //y0+54 excludes the last row (in chip coordinates), should be y0+55 to include all rows
	    iy=(ipx-x0+ox)%3+(ipy-y0+oy)*3+ooy;
	    ix=(ipx-x0+ox)/3;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	    groupmap[iy][ix]=0;
	    //  cout << ipx << " " << ipy << " " << ix << " " << iy << endl;
	  }
	}

	cout << "G1" << endl; //chip coordinates of chip1 group2: x=255+12 to x=255+246, y=65 to y=128
	//3 square pixels in x on the left
	oy=-54;
	ooy=54*3;
	ox=2;//; //original value is 3
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0+54; ipy<y0+64+54; ipy++) { //I think y0+54 catches the last row of group1! Should be y0+55 if we want to include all rows? And y0+55+64
	    ix=(ipx-x0+ox)/5;
	    iy=(ipx-x0+ox)%5+(ipy-y0+oy)*5+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	    groupmap[iy][ix]=1;
	  }
	}
	
	cout << "G2" << endl; //chip coordinates of chip1 group34: x=255+11 to x=255+246, y=129 to y=247
	//2 square pixels on the left
	oy=-54-64;
	ooy=54*3+64*5;	
	ox=3;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0+64+54; ipy<y0+64*2+54*2; ipy++) { //Same as above, I think it should be y0+55+64 and y0+55*2+64*2 to include all rows
	    ix=(ipx-x0+ox)/4;
	    iy=(ipx-x0+ox)%4+(ipy-y0+oy)*4+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	    groupmap[iy][ix]=2;
	  }
	}
	
	//chip 6
	/*
	 * TL;DR comments: y0 is too high by 3, group34 and group 1 are by one row too short
	 * x is by two columns too short
	 * NOTE: If x0, x1, y0 are changed, likely also ox (and oy and ooy) will be affected!
	 */

	cout << "G0" << endl; //chip coordinates of chip6 group34: x=255+256+9 to x=255+256+244, y=255+8 to y=255+126
	//9 pixels guard ring, bonding shift by one pixel in -y, 2 square pixels in x on the right
	x0=256*2+10;
	y0=256+10;
	x1=256*2+246;
	ooy=256*5;
	oy=0;
	ox=1;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0; ipy<y0+54+64; ipy++) { //shifted by 3 rows because of y0, if y0 is corrected it should be y0+55+64 to include all rows
	    ix=(ipx-x0+ox)/4;
	    iy=(ipx-x0+ox)%4+(ipy-y0+oy)*4+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	    //if (ipx==x0)    cout << ipx << " " << ipy << " " << ix << " " << iy << endl;
	    groupmap[iy][ix]=2;
	  }
	}

	cout << "G1" << endl; //chip coordinates of chip6 group2: x=255+256+9 to x=255+256+243, y=255+127 to y=255+190
	//3 square pixels in x on the right
	oy=-54-64;
	ooy+=(54+64)*4;
	ox=1;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0+54+64; ipy<y0+64*2+54; ipy++) { //shifted by 3 rows because of y0, if y0 is corrected it should be y0+55+64 and y0+55+64*2 to include all rows
	    ix=(ipx-x0+ox)/5;
	    iy=(ipx-x0+ox)%5+(ipy-y0+oy)*5+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	    groupmap[iy][ix]=1;
	  }
	}
	
	cout << "G2" << endl; //chip coordinates of chip6 group1: x=255+256+9 to x=255+256+245, y=255+191 to y=255+245
	//one square pixel in x on the right
	oy=-54-64*2;
	ooy+=64*5;	
	ox=1;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0+64*2+54; ipy<y0+64*2+54*2; ipy++) { //shifted by 3 rows because of y0, if y0 is corrected it should be y0+55+64*2 and y0+55*2+64*2
	    ix=(ipx-x0+ox)/3;
	    iy=(ipx-x0+ox)%3+(ipy-y0+oy)*3+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	    groupmap[iy][ix]=0;
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
        return val;
    };

    /* virtual void calcGhost(char *data, int ix, int iy) { */
    /*   double val=0; */
    /*   ghost[iy][ix]=0; */

    /* } */

    /* virtual void calcGhost(char *data) { */
    /*   for (int ix=0; ix<25; ix++){ */
    /*     for (int iy=0; iy<200; iy++) { */
    /* 	calcGhost(data, ix,iy); */
    /*     }  */
    /*   } */
    /*   // cout << "*" << endl; */
    /* } */

    /* double getGhost(int ix, int iy) { */
    /*   return 0; */
    /* }; */

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
#ifdef ALDO //VH
      return ((header *)buff)->bunchNumber; //VH
#else //VH
      return ((header *)buff)->detHeader.frameNumber;
#endif //VH
    };

    /**

       Returns the packet number for the given dataset. purely virtual func
       \param buff pointer to the dataset
       \returns packet number number



    */
    int getPacketNumber(char *buff) {
#ifdef ALDO //VH
      return -1; //VH //TODO: Keep in mind in case of bugs!
#else //VH
      return ((header *)buff)->detHeader.packetNumber;
#endif //VH
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
        char *retval = 0;
        int nd;
        int fnum = -1;
        np = 0;
        int pn;

        //  cout << dataSize << endl;
        if (ff >= 0)
            fnum = ff;

        if (filebin.is_open()) {
            if (filebin.read(data, dataSize)) {
                ff = getFrameNumber(data);
                np = getPacketNumber(data);
                return data;
            }
        }
        return NULL;
    };

    /*    Loops over a memory slot until a complete frame is found (i.e. all */
    /*    packets 0 to nPackets, same frame number). purely virtual func \param */
    /*    data pointer to the memory to be analyzed \param ndata reference to the */
    /*    amount of data found for the frame, in case the frame is incomplete at */
    /*    the end of the memory slot \param dsize size of the memory slot to be */
    /*    analyzed \returns pointer to the beginning of the last good frame (might */
    /*    be incomplete if ndata smaller than dataSize), or NULL if no frame is */
    /*    found */

    /* *\/ */
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
