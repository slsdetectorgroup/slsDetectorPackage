// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCH03v2DATA_H
#define MOENCH03v2DATA_H
//#define MYROOT

#ifndef MYROOT
#include "sls/sls_detector_defs.h"
#endif




#ifdef MYROOT

    typedef struct {
        uint64_t frameNumber;
        uint32_t expLength;
        uint32_t packetNumber;
        uint64_t bunchId;
        uint64_t timestamp;
        uint16_t modId;
        uint16_t row;
        uint16_t column;
        uint16_t reserved;
        uint32_t debug;
        uint16_t roundRNumber;
        uint8_t detType;
        uint8_t version;
    } sls_detector_header;
#define MAX_NUM_PACKETS 512
// using sls_bitset = std::bitset<MAX_NUM_PACKETS>;
//  using bitset_storage = uint8_t[MAX_NUM_PACKETS / 8];
  struct sls_receiver_header {
        sls_detector_header detHeader; /**< is the detector header */
        uint8_t packetsMask[64];        /**< is the packets caught bit mask */
    };
#endif

#include "slsDetectorData.h"
#ifdef RAWDATA
#define DATA_OFFSET sizeof(header)
#endif
#ifndef RAWDATA
#define DATA_OFFSET 0
#endif


class moench03v2Data : public slsDetectorData<uint16_t> {

  private:
    int iframe;
    const int nRows;
    
    

    double ghost[200][25];

    // Single point of definition if we need to customize
#ifndef MYROOT
    using header = sls::defs::sls_receiver_header;
#endif
#ifdef MYROOT
    sls_receiver_header header;
#endif
  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

    */
    moench03v2Data(int nrows = 200)
        : slsDetectorData<uint16_t>(400, nrows*2,2* 400*nrows*2 + DATA_OFFSET),
      nRows(nrows) {
	
	std::cout << "MOENCH width new firmware " << dataSize << std::endl;
	
	int off=DATA_OFFSET;
	    for (int ix = 0; ix < 400; ix++) {
	      for (int iy = 0; iy < nRows*2; iy++) {
		dataMap[iy][ix]=off+2*(iy*400+ix);
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
    double getValue(char *data, int ix, int iy = 0) override {
        uint16_t val = getChannel(data, ix, iy) & 0x3fff;
        return val;
    };

    virtual void calcGhost(char *data, int ix, int iy) {
        double val = 0;
        /* for (int ix=0; ix<25; ix++){ */
        /*   for (int iy=0; iy<200; iy++) { */
        val = 0;
        //	cout << "** ";
        for (int isc = 0; isc < 16; isc++) {
            //  for (int ii=0; ii<2; ii++) {
            val += getChannel(data, ix + 25 * isc, iy);
            //  cout << "(" << isc << "," << val << " " ;
            val += getChannel(data, ix + 25 * isc, 399 - iy);
            // cout << val << " " ;
            // }
        }
        ghost[iy][ix] = val; //-6224;
                             //	cout << " --"<< endl;
                             /*   }  */
                             /* } */
                             // cout << "*" << endl;
    }

    virtual void calcGhost(char *data) {
        for (int ix = 0; ix < 25; ix++) {
            for (int iy = 0; iy < 200; iy++) {
                calcGhost(data, ix, iy);
            }
        }
        // cout << "*" << endl;
    }

    double getGhost(int ix, int iy) {
        if (iy < 200)
            return ghost[iy][ix % 25];
        if (iy < 400)
            return ghost[399 - iy][ix % 25];
        return 0;
    };

    /**

    Returns the frame number for the given dataset. Purely virtual func.
    \param buff pointer to the dataset
    \returns frame number

 */

    int getFrameNumber(char *buff) {
#ifdef RAWDATA
      return ((sls::defs::sls_receiver_header *)buff)->detHeader.frameNumber;
#endif
#ifndef RAWDATA
      return 1;
#endif

    }

    /**

       Returns the packet number for the given dataset. purely virtual func
       \param buff pointer to the dataset
       \returns packet number number



    */
    int getPacketNumber(char *buff) {
#ifdef RAWDATA
      return ((sls::defs::sls_receiver_header  *)buff)->detHeader.packetNumber;
#endif
#ifndef RAWDATA
      return 0;
#endif
      
    }

    char *readNextFrame(std::ifstream &filebin) override {
        int ff = -1, np = -1;
        return readNextFrame(filebin, ff, np);
    }

    // not present in base class
    virtual char *readNextFrame(std::ifstream &filebin, int &ff) {
        int np = -1;
        return readNextFrame(filebin, ff, np);
    };

    // not present in base class
    virtual char *readNextFrame(std::ifstream &filebin, int &ff, int &np) {
        char *data = new char[dataSize];
        char *d = readNextFrame(filebin, ff, np, data);
        if (d == NULL) {
            delete[] data;
            data = NULL;
        }
        return data;
    }

    // not present in base class
    virtual char *readNextFrame(std::ifstream &filebin, int &ff, int &np,
                                char *data) {
        np = 0;
        if (filebin.is_open()) {
            if (filebin.read(data, dataSize)) {
                ff = getFrameNumber(data);
                np = getPacketNumber(data);
	std::cout << "**" << ff << " " << dataSize << " " << ff << " " << np << std::endl;

                return data;
            }
        }
	std::cout << "**" << ff << " " << dataSize << " " << ff << " " << np << std::endl;
        return nullptr;
    }

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
    char *findNextFrame(char *data, int &ndata, int dsize) override {
        if (dsize < dataSize)
            ndata = dsize;
        else
            ndata = dataSize;
        return data;
    }

    // int getPacketNumber(int x, int y) {return dataMap[y][x]/packetSize;};
};

#endif
