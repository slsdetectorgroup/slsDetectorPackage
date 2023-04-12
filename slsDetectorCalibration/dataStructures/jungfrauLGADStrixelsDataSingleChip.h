// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef JUNGFRAULGADSTRIXELSDATASINGLECHIP_H
#define JUNGFRAULGADSTRIXELSDATASINGLECHIP_H
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

namespace strixelSingleChip {
  constexpr int nc_chip = 256;
  constexpr int nr_chip = 256;
  constexpr int gr = 9;

  //Group 1: 25um pitch, groups of 3, 1 column of square pixels
  constexpr int g1_ncols{ (nc_chip-(2*gr)-1)/3 }; //79
  constexpr int g1_nrows{ ( (nr_chip/4)-gr )*3 }; //165
  
  //Group 2: 15um pitch, groups of 5, 3 columns of square pixels
  constexpr int g2_ncols{ (nc_chip-(2*gr)-3)/5 }; //47
  constexpr int g2_nrows{ (nr_chip/4)*5 }; //320
  
  //Group 3: 18.75um pitch, groups of 4, 2 columns of square pixels (double the size of the other groups)
  constexpr int g3_ncols{ (nc_chip-(2*gr)-2)/4 }; //59
  constexpr int g3_nrows{ ( ((nr_chip/4)*2)-gr )*4 }; //476

  constexpr int nc_strixel = 2*gr + 1 + g1_ncols; //group 1 is the "longest" group in x and has one extra square pixel
  constexpr int nr_strixel = 2*gr + g1_nrows + g2_nrows + g3_nrows;
}

typedef struct {
    uint64_t bunchNumber; /**< is the frame number */
    uint64_t pre;         /**< something */

} jf_header; //Aldo's header


using namespace strixelSingleChip;    
class jungfrauLGADStrixelsDataSingleChip : public slsDetectorData<uint16_t> {

  private:
    int iframe;
    int mchip;

    void remapGroup( const int group ) {
      int ix, iy=0;
      int x0, y0, x1, y1, shifty;
      int multiplicator;

      switch (group) {
      default:
      case 1:
	multiplicator = 3;
	break;
      case 2:
	multiplicator = 5;
	break;
      case 3:
	multiplicator = 4;
	break;
      }

      if ( mchip == 1 ) {
	switch (group) {
	default:
	case 1:
	  x0 = 10;
	  x1 = 247;
	  y0 = 10;
	  y1 = 65;
	  shifty = 0;
	  break;
	case 2:
	  x0 = 12;
	  x1 = 247;
	  y0 = 65;
	  y1 = 129;
	  shifty = ( (nr_chip/4)-gr )*3;
	  break;
	case 3:
	  x0 = 11;
	  x1 = 247;
	  y0 = 129;
	  y1 = 248;
	  shifty = ( (nr_chip/4)-gr )*3 + (nr_chip/4)*5;
	  break;
	}
      }

      if ( mchip == 6 ) {
	switch (group) {
	default:
	case 1:
	  x0 = 9;
	  x1 = 246;
	  y0 = 191;
	  y1 = 246;
	  shifty = ( (nr_chip/4)-gr+(nr_chip/4) )*4 + (nr_chip/4)*5;
	  break;
	case 2:
	  x0 = 9;
	  x1 = 244;
	  y0 = 127;
	  y1 = 191;
	  shifty = ( (nr_chip/4)-gr+(nr_chip/4) )*4;
	  break;
	case 3:
	  x0 = 9;
	  x1 = 245;
	  y0 = 8;
	  y1 = 127;
	  shifty = 0;
	  break;
	}
      }
     
      //remapping loop
      for ( int ipx=x0; ipx!=x1; ++ipx ) {
	for ( int ipy=y0; ipy!=y1; ++ipy) {
	  ix = (ipx-x0)/multiplicator;
	  for ( int m=0; m!=multiplicator; ++m ) {
	    if ( (ipx-x0)%multiplicator==m ) iy=(ipy-y0)*multiplicator + m + shifty;
	  }
	  dataMap[iy][ix] = sizeof(header) + (nc_chip * ipy + ipx) * 2;
	}
      } 

    }

  public:

#ifdef ALDO //VH
    using header = jf_header; //VH
#else //VH
    using header = sls::defs::sls_receiver_header;
#endif //VH
    
    jungfrauLGADStrixelsDataSingleChip( const int chip )
      : slsDetectorData<uint16_t>( /*nc_strixel*/nc_chip/3, /*nr_strixel*/ nr_chip*5,
                                    nc_chip * nr_chip * 2 + sizeof(header) ) {
      std::cout << "Jungfrau strixels single chip" << std::endl;
#ifdef ALDO //VH
      std::cout<< "using reduced jf_header" << std::endl; //VH
#endif //VH

      mchip = chip;
      //Fill all strixels with dummy values
      for (int ix = 0; ix != nc_strixel; ++ix) {
	for (int iy = 0; iy != nr_strixel; ++iy) {
	  dataMap[iy][ix] = sizeof(header);
#ifdef HIGHZ
	  dataMask[iy][ix] = 0x3fff;
#endif
	}
      }

      remapGroup(1);
      remapGroup(2);
      remapGroup(3);
	
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

    /**

    Returns the frame number for the given dataset. Purely virtual func.
    \param buff pointer to the dataset
    \returns frame number

    */


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
	  // fnum = ff;

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
