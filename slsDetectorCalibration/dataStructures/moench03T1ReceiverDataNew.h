// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCH03T1RECDATANEW_H
#define MOENCH03T1RECDATANEW_H
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

class moench03T1ReceiverDataNew : public slsDetectorData<uint16_t> {

  private:
    int iframe;
    int nadc;
    int sc_width;
    int sc_height;
    const int nSamples;

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
    moench03T1ReceiverDataNew(int ns = 5000)
        : slsDetectorData<uint16_t>(400, 400, ns * 2 * 32 + sizeof(header)),
          nSamples(ns) {

        int nadc = 32;
        int sc_width = 25;
        int sc_height = 200;

        int adc_nr[32] = {300, 325, 350, 375, 300, 325, 350, 375, 200, 225, 250,
                          275, 200, 225, 250, 275, 100, 125, 150, 175, 100, 125,
                          150, 175, 0,   25,  50,  75,  0,   25,  50,  75};

        int row, col;

        int isample;
        int iadc;
        int ix, iy;

        int npackets = 40;
        int i;
        int adc4(0);
	int off=sizeof(header);
        for (int ip = 0; ip < npackets; ip++) {
            for (int is = 0; is < 128; is++) {

                for (iadc = 0; iadc < nadc; iadc++) {
                    i = 128 * ip + is;
                    adc4 = (int)iadc / 4;
                    if (i < sc_width * sc_height) {
                        //  for (int i=0; i<sc_width*sc_height; i++) {
                        col = adc_nr[iadc] + (i % sc_width);
                        if (adc4 % 2 == 0) {
                            row = 199 - i / sc_width;
                        } else {
                            row = 200 + i / sc_width;
                        }
                        dataMap[row][col] = off +
                                            (nadc * i + iadc) * 2; //+16*(ip+1);
#ifdef HIGHZ
                        dataMask[row][col] = 0x3fff; // invert data
#endif
                        if (dataMap[row][col] < 0 ||
                            dataMap[row][col] >= off + nSamples * 2 * 32)
                            std::cout << "Error: pointer " << dataMap[row][col]
                                      << " out of range " << std::endl;
                    }
                }
            }
        }
        // double ghost[200][25];

        for (int ix = 0; ix < 25; ix++)
            for (int iy = 0; iy < 200; iy++)
                ghost[iy][ix] = 0.;

        int ipacket;
        uint ibyte;
        int ii = 0;
        for (ibyte = 0; ibyte < sizeof(header) / 2; ibyte++) {
            xmap[ibyte] = -1;
            ymap[ibyte] = -1;
        }
        off = sizeof(header) / 2;
        for (ipacket = 0; ipacket < npackets; ipacket++) {
            for (ibyte = 0; ibyte < 8192 / 2; ibyte++) {
                i = ipacket * 8208 / 2 + ibyte;
                isample = ii / nadc;
                if (isample < nSamples) {
                    iadc = ii % nadc;
                    adc4 = (int)iadc / 4;
                    ix = isample % sc_width;
                    iy = isample / sc_width;
                    if (adc4 % 2 == 0) {
                        xmap[i + off] = adc_nr[iadc] + ix;
                        ymap[i + off] = ny / 2 - 1 - iy;
                    } else {
                        xmap[i + off] = adc_nr[iadc] + ix;
                        ymap[i + off] = ny / 2 + iy;
                    }
                }
                ii++;
                //	}
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
      return ((sls::defs::sls_receiver_header *)buff)->detHeader.frameNumber;
    }

    /**

       Returns the packet number for the given dataset. purely virtual func
       \param buff pointer to the dataset
       \returns packet number number



    */
    int getPacketNumber(char *buff) {
      return ((sls::defs::sls_receiver_header  *)buff)->detHeader.packetNumber;
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
                return data;
            }
        }
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
