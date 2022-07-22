// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef JUNGFRAULGADSTRIXELDATA_H
#define JUNGFRAULGADSTRIXELDATA_H
#include "sls/sls_detector_defs.h"
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

} jf_header;


using namespace std;    
class jungfrauLGADStrixelsData : public slsDetectorData<uint16_t> {

  private:
    int iframe;

    using header = sls::defs::sls_receiver_header;
  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

    */
    jungfrauLGADStrixelsData()
        : slsDetectorData<uint16_t>(1024/5, 512*5,
                                    512 * 1024 * 2 + sizeof(header)) {
      cout << "aaa" << endl;
        for (int ix = 0; ix < 1024/5; ix++) {
            for (int iy = 0; iy < 512*5; iy++) {
	      dataMap[iy][ix] = sizeof(header);//+ ( 1024 * 5 + 300) * 2; //somewhere on the guardring of the LGAD
#ifdef HIGHZ
                dataMask[iy][ix] = 0x3fff;
#endif
            }
        }
	int x0=256+10, x1=256+246;
	int y0=10, y1=256-10;
	int ix,iy;
	int ox=0, oy=0, ooy=0;
	ox=0;
	cout << "G0" << endl;

	//chip1

	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0; ipy<y0+54; ipy++) {
	    ix=(ipx-x0+ox)/3;
	    iy=(ipx-x0+ox)%3+(ipy-y0+oy)*3+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	    
	    //  cout << ipx << " " << ipy << " " << ix << " " << iy << endl;


	  }
	}
	cout << "G1" << endl;
	oy=-54;
	ooy=54*3;
	ox=3;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0+54; ipy<y0+64+54; ipy++) {
	    ix=(ipx-x0+ox)/5;
	    iy=(ipx-x0+ox)%5+(ipy-y0+oy)*5+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	  }
	}
	
	cout << "G2" << endl;
	oy=-54-64;
	ooy=54*3+64*5;	
	ox=3;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0+64+54; ipy<y0+64*2+54*2; ipy++) {
	    ix=(ipx-x0+ox)/4;
	    iy=(ipx-x0+ox)%4+(ipy-y0+oy)*4+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	  }
	}
	
	//chip 6

	x0=256*2+10;
	y0=256+10;
	x1=256*2+246;
	ooy=256*5;
	oy=0;
	ox=1;
	cout << "G0" << endl;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0; ipy<y0+54+64; ipy++) {
	    ix=(ipx-x0+ox)/4;
	    iy=(ipx-x0+ox)%4+(ipy-y0+oy)*4+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	    if (ipx==x0)
	      cout << ipx << " " << ipy << " " << ix << " " << iy << endl;


	  }
	}
	cout << "G1" << endl;
	oy=-54-64;
	ooy+=(54+64)*4;
	ox=1;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0+54+64; ipy<y0+64*2+54; ipy++) {
	    ix=(ipx-x0+ox)/5;
	    iy=(ipx-x0+ox)%5+(ipy-y0+oy)*5+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	  }
	}
	
	cout << "G2" << endl;
	oy=-54-64*2;
	ooy+=64*5;	
	ox=1;
	for (int ipx=x0; ipx<x1; ipx++) {
	  for (int ipy=y0+64*2+54; ipy<y0+64*2+54*2; ipy++) {
	    ix=(ipx-x0+ox)/3;
	    iy=(ipx-x0+ox)%3+(ipy-y0+oy)*3+ooy;
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
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
        return ((header *)buff)->detHeader.frameNumber;
    }

    /**

       Returns the packet number for the given dataset. purely virtual func
       \param buff pointer to the dataset
       \returns packet number number



    */
    int getPacketNumber(char *buff) {
        return ((header *)buff)->detHeader.packetNumber;
    }

    /* int getFrameNumber(char *buff) { */
      
    /*   //  return ((jf_header *)buff)->bunchNumber; */
    /* };  */

    /* /\** */

    /*    Returns the packet number for the given dataset. purely virtual func */
    /*    \param buff pointer to the dataset */
    /*    \returns packet number number */



    /* *\/ */
    /* int getPacketNumber(char *buff) { */
    /*     return 0; */
    /* } ; */

    /*    /\** */

    /*      Loops over a memory slot until a complete frame is found (i.e. all
     * packets 0 to nPackets, same frame number). purely virtual func */
    /*      \param data pointer to the memory to be analyzed */
    /*      \param ndata reference to the amount of data found for the frame, in
     * case the frame is incomplete at the end of the memory slot */
    /*      \param dsize size of the memory slot to be analyzed */
    /*      \returns pointer to the beginning of the last good frame (might be
     * incomplete if ndata smaller than dataSize), or NULL if no frame is found
     */

    /*   *\/ */
    /*     virtual  char *findNextFrame(char *data, int &ndata, int
     * dsize){ndata=dsize; setDataSize(dsize);  return data;}; */

    /*    /\** */

    /*      Loops over a file stream until a complete frame is found (i.e. all
     * packets 0 to nPackets, same frame number). Can be overloaded for
     * different kind of detectors!  */
    /*      \param filebin input file stream (binary) */
    /*      \returns pointer to the begin of the last good frame, NULL if no
     * frame is found or last frame is incomplete */

    /* */
       /* char *readNextFrame(ifstream &filebin){ */
       /*    //	int afifo_length=0; */
       /*    uint16_t *afifo_cont; */
       /*    int ib=0; */
       /*    if (filebin.is_open()) { */
       /* 	afifo_cont=new uint16_t[dataSize/2]; */
       /* 	while (filebin.read(((char*)afifo_cont)+ib,2)) { */
       /* 	  ib+=2; */
       /* 	  if (ib==dataSize) break; */
       /* 	} */
       /* 	if (ib>0) { */
       /* 	  iframe++; */
       /* 	  // cout << ib << "-" << endl; */
       /* 	  return (char*)afifo_cont; */
       /* 	} else { */
       /* 	  delete [] afifo_cont; */
       /* 	  return NULL; */
       /* 	} */
       /*    } */
       /*    return NULL; */
       /*  }; */

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

    /*  /**

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
