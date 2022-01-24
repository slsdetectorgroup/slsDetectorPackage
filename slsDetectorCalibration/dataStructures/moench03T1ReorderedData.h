// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCH03T1REORDERED_H
#define MOENCH03T1REORDERED_H
#include "slsDetectorData.h"

class moench03T1ReorderedData : public slsDetectorData<uint16_t> {

  private:
  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

          fwrite(&ff, 8, 1,of);//write detector frame number
          fwrite(&ifr, 8, 1,of);//write datset frame number
          fwrite(data,2,NX*NY,of);//write reordered data
    */
    moench03T1ReorderedData()
        : slsDetectorData<uint16_t>(400, 400, 2 * 400 * 400 + 2 * 8) {
        for (int iy = 0; iy < 400; iy++)
            for (int ix = 0; ix < 400; ix++)
                dataMap[iy][ix] = 2 * 8 + 2 * (iy * 400 + ix);

        int ibyte;
        for (ibyte = 0; ibyte < 8; ibyte++) {
            xmap[ibyte] = -1;
            ymap[ibyte] = -1;
        }
        for (ibyte = 0; ibyte < 400 * 400; ibyte++) {
            xmap[ibyte + 8] = ibyte % 400;
            ymap[ibyte + 8] = ibyte / 400;
        }

        //  cout << "data struct created" << endl;
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

    int getFrameNumber(char *buff) { return *((int *)buff); };

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

    /*   *\/ */
    /*     virtual char *readNextFrame(ifstream &filebin){ */
    /*       //	int afifo_length=0;   */
    /*       uint16_t *afifo_cont;  */
    /*       int ib=0; */
    /*       if (filebin.is_open()) { */
    /* 	afifo_cont=new uint16_t[dataSize/2]; */
    /*  	while (filebin.read(((char*)afifo_cont)+ib,2)) { */
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
    /*       }      */
    /*       return NULL; */
    /*     }; */

    virtual char *readNextFrame(ifstream &filebin) {
        int ff = -1, np = -1;
        return readNextFrame(filebin, ff, np);
    };

    virtual char *readNextFrame(ifstream &filebin, int &ff) {
        int np = -1;
        return readNextFrame(filebin, ff, np);
    };

    virtual char *readNextFrame(ifstream &filebin, int &ff, int &np) {
        char *data = new char[dataSize];
        char *d = readNextFrame(filebin, ff, np, data);
        if (d == NULL) {
            delete[] data;
            data = NULL;
        }
        return data;
    }

    virtual char *readNextFrame(ifstream &filebin, int &ff, int &np,
                                char *data) {
        char *retval = 0;
        int nd;
        int fnum = -1;
        np = 0;
        int pn;

        if (ff >= 0)
            fnum = ff;

        if (filebin.is_open()) {
            if (filebin.read(data, dataSize)) {
                ff = getFrameNumber(data);
                np = 40;
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
    }

    // int getPacketNumber(int x, int y) {return dataMap[y][x]/packetSize;};
};

#endif
