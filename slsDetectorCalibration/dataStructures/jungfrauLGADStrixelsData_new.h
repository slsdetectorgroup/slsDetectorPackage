// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef JUNGFRAULGADSTRIXELSDATA_H
#define JUNGFRAULGADSTRIXELSDATA_H
#ifdef CINT
#include "sls/sls_detector_defs_CINT.h"
#else
#include "sls/sls_detector_defs.h"
#endif
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

namespace strixelSingleChip {
  constexpr int nc_rawimg = 1024; //for full images  //256;
  constexpr int nr_rawimg = 512; 
  constexpr int nr_chip = 256;
  constexpr int gr = 9;

  //Group 1: 25um pitch, groups of 3, 1 column of square pixels
  constexpr int g1_ncols{ (nc_rawimg-(2*gr)-1)/3 }; //79
  constexpr int g1_nrows{ ( (nr_chip/4)-gr )*3 }; //165
  
  //Group 2: 15um pitch, groups of 5, 3 columns of square pixels
  constexpr int g2_ncols{ (nc_rawimg-(2*gr)-3)/5 }; //47
  constexpr int g2_nrows{ (nr_chip/4)*5 }; //320
  
  //Group 3: 18.75um pitch, groups of 4, 2 columns of square pixels (double the size of the other groups)
  constexpr int g3_ncols{ (nc_rawimg-(2*gr)-2)/4 }; //59
  constexpr int g3_nrows{ ( ((nr_chip/4)*2)-gr )*4 }; //476

  constexpr int nc_strixel = 2*gr + 1 + g1_ncols; //group 1 is the "longest" group in x and has one extra square pixel
  constexpr int nr_strixel = 2*gr + g1_nrows + g2_nrows + g3_nrows;



}

typedef struct {
    uint64_t bunchNumber; /**< is the frame number */
    uint64_t pre;         /**< something */

} jf_header; //Aldo's header


using namespace strixelSingleChip; 
   
class jungfrauLGADStrixelsData : public slsDetectorData<uint16_t> {

  private:
    int iframe;
    int mchip;
    int chip_x0;
    int chip_y0;

    void remapGroup( const int group ) {
      int ix, iy=0;
      int x0, y0, x1, y1, shifty;
      int multiplicator;
      int shiftx;
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
        
	chip_x0=256;
	chip_y0=1;  //because of bump bonding issues(+1 row)  on M408 
	
	switch (group) {
	default:
	case 1:
	  x0 = 10+chip_x0;  //9 gr + 1 sq pixel 
	  x1 = 246+chip_x0;
	  y0 = 9+chip_y0;
	  y1 = 64+chip_y0;
	  shifty = 0;
	  break;
	case 2:
	  x0 = 12+chip_x0;
	  x1 = 247+chip_x0;
	  y0 = 64+chip_y0;
	  y1 = 128+chip_y0;
	  shifty = g1_nrows;
	  break;
	case 3:
	  x0 = 11+chip_x0;
	  x1 = 247+chip_x0;
	  y0 = 128+chip_y0;
	  y1 = 247+chip_y0;
	  shifty = g2_nrows+g1_nrows;
	  break;
	}
      }

      if ( mchip == 6 ) {

	chip_x0=512;
	chip_y0=255;  //should be 256 but is 255 because of bump bonding issues (+1 row) on M408
	
	switch (group) {
	default:
	case 1:

	  x0 = 9+chip_x0;  //9 gr sq pixel 
	  x1 = 246+chip_x0;
	  y0 = 192+chip_y0;
	  y1 = 244+chip_y0;

	  shifty = g1_nrows+2*g2_nrows+2*g3_nrows;
	  break;

	case 2:
	  
	  x0 = 9+chip_x0;
	  x1 = 244+chip_x0;
	  y0 = 128+chip_y0;
	  y1 = 191+chip_y0;
	  

	  shifty = g1_nrows+g2_nrows+2*g3_nrows;;
	  break;
	case 3:

	  x0 = 9+chip_x0;
	  x1 = 244+chip_x0;
	  y0 = 9+chip_y0;
	  y1 = 127+chip_y0;
	  shifty =g1_nrows+g2_nrows+g3_nrows;

	  break;
	}
      }
     
      //remapping loop
      for ( int ipy=y0; ipy<=y1;ipy++) {
	for ( int ipx=x0; ipx<=x1; ipx++ ) {

	  ix = int ((ipx-x0)/multiplicator);
	  for ( int m=0; m<multiplicator;m++ ) {
	    if ( (ipx-x0)%multiplicator==m ) iy=(ipy-y0)*multiplicator +m + shifty;
	  

	  }
	  
	  //	  if (iy< 40)	  cout << iy << "  " << ix <<endl;
	  	  dataMap[iy][ix] = sizeof(header) + (nc_rawimg * ipy + ipx) * 2;
	  	  groupmap[iy][ix]=group-1;
	}
      } 

    }

  public:

    int groupmap[512*5][1024/3];

    using header = sls::defs::sls_receiver_header;

    
    jungfrauLGADStrixelsData()
      : slsDetectorData<uint16_t>( /*nc_strixel*/g1_ncols, /*nr_strixel*/ 2*g1_nrows+2*g2_nrows+2*g3_nrows,
				   g1_ncols*  (2*g1_nrows+2*g2_nrows+2*g3_nrows) * 2 + sizeof(header) ) {
      std::cout << "Jungfrau strixels 2X single chip with full module data " << std::endl;

      

      
      //Fill all strixels with dummy values
      for (int ix = 0; ix != g1_ncols; ++ix) {
	for (int iy = 0; iy != 2*g1_nrows+2*g2_nrows+2*g3_nrows; ++iy) {
	  dataMap[iy][ix] = sizeof(header);

	}
      }

      cout << "sizeofheader = "<<sizeof(header)<<endl;
std::cout << "Jungfrau strixels 2X single chip with full module data " << std::endl;

      mchip = 1;
      remapGroup(1);
  

      remapGroup(2);
      remapGroup(3);

      mchip = 6;
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
