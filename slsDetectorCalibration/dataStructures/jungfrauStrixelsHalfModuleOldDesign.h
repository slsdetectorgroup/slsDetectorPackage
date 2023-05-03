// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef JUNGFRAUSTRIXELSHALFMODULEOLD_H
#define JUNGFRAUSTRIXELSHALFMODULEOLD_H
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

namespace strixelsOldDesign {
  constexpr int NC_STRIXEL = (1024*3);
  constexpr int NR_TOTAL = (512/3);
  constexpr int NR_STRIXEL = ( (256-4)/3 );
}

typedef struct {
    uint64_t bunchNumber; /**< is the frame number */
    uint64_t pre;         /**< something */

} jf_header; //Aldo's header


using namespace strixelsOldDesign;    
class jungfrauStrixelsHalfModuleOldDesign : public slsDetectorData<uint16_t> {

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
    jungfrauStrixelsHalfModuleOldDesign()
      : slsDetectorData<uint16_t>( 1024*3, 512/3,
                                    512 * 1024 * 2 + sizeof(header) ) {
      std::cout << "Jungfrau strixels old design" << std::endl;
#ifdef ALDO //VH
      std::cout<< "using reduced jf_header" << std::endl; //VH
#endif //VH
      for (int ix = 0; ix != 1024*3; ++ix) {
	for (int iy = 0; iy != 512/3; ++iy) {
	  dataMap[iy][ix] = sizeof(header);//+ ( 1024 * 5 + 300) * 2; //somewhere on the guardring of the LGAD
#ifdef HIGHZ
	  dataMask[iy][ix] = 0x3fff;
#endif
	}
      }

      //remap
      int ix, iy;
      for (int ipx=0; ipx!=1024; ++ipx) {
	for (int ipy=0; ipy!=256-4; ++ipy) {
	  iy=ipy/3;
	  /* //1
	  if (ipy%3==0) ix=ipx*3+2;
	  if (ipy%3==1) ix=ipx*3+1;
	  if (ipy%3==2) ix=ipx*3;
	  */
	  /* //2
	  if (ipy%3==2) ix=ipx*3+2;
	  if (ipy%3==0) ix=ipx*3+1;
	  if (ipy%3==1) ix=ipx*3;
	  */
	  /* //3
	  if (ipy%3==1) ix=ipx*3+2;
	  if (ipy%3==2) ix=ipx*3+1;
	  if (ipy%3==0) ix=ipx*3;
	  */
	  //4 //This seems to be correct //corresponds to looking from the backside of the sensor
	  if (ipy%3==0) ix=ipx*3;
	  if (ipy%3==1) ix=ipx*3+1;
	  if (ipy%3==2) ix=ipx*3+2;
	  
	  /* //5
	  if (ipy%3==2) ix=ipx*3;
	  if (ipy%3==0) ix=ipx*3+1;
	  if (ipy%3==1) ix=ipx*3+2;
	  */
	  /* //6
	  if (ipy%3==1) ix=ipx*3;
	  if (ipy%3==2) ix=ipx*3+1;
	  if (ipy%3==0) ix=ipx*3+2;
	  */

	  if ( ipx!=255 && ipx!=256 && ipx!=511 && ipx!=512 && ipx!=767 && ipx!=768 ) //avoid double pixels
	    // ( !( ipx%256==0 || ipx%256==255 ) || ipx==0 || ipx==1023 )
	    dataMap[iy][ix] = sizeof(header) + (1024 * ipy + ipx) * 2;
	  //  cout << ipx << " " << ipy << " " << ix << " " << iy << endl;
	}
      }
	
      iframe = 0;
      std::cout << "data struct created" << std::endl;
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
      //uint32_t fakePacketNumber = 1000;
      //return fakePacketNumber; //VH //TODO: Keep in mind in case of bugs! //This is definitely bad!
      return 1000;
#else //VH
      return ((header *)buff)->detHeader.packetNumber;
#endif //VH
    };

   

    char *readNextFrame(std::ifstream &filebin) {
        int ff = -1, np = -1;
        return readNextFrame(filebin, ff, np);
    };

    char *readNextFrame(std::ifstream &filebin, int &ff) {
        int np = -1;
        return readNextFrame(filebin, ff, np);
    };

    char *readNextFrame(std::ifstream &filebin, int &ff, int &np) {
        char *data = new char[dataSize];
        char *d = readNextFrame(filebin, ff, np, data);
        if (d == NULL) {
            delete[] data;
            data = NULL;
        }
        return data;
    };

    char *readNextFrame(std::ifstream &filebin, int &ff, int &np,char *data) {
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
