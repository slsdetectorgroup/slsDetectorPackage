// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef IMAGEZMQ16BIT_H
#define IMAGEZMQ16BIT_H
#include "slsDetectorData.h"

class imageZmq16bit : public slsDetectorData<uint16_t> {

  private:
    // int iframe;
  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

    */
    // moench03T1ZmqDataNew(int ns=5000): slsDetectorData<uint16_t>(400, 400,
    // ns*32*2+sizeof(int)), nSamples(ns), offset(sizeof(int)), xtalk(0.00021) {
    imageZmq16bit(int nnx = 400, int nny = 400)
        : slsDetectorData<uint16_t>(nnx, nny, 2 * nnx * nny) {
        cout << "* " << nx << " " << ny << endl;
        int is = 0;
        for (int row = 0; row < ny; row++) {
            for (int col = 0; col < nx; col++) {
                dataMap[row][col] = is * 2;
                is++;
            }
        }
    };

    virtual double getValue(char *data, int ix, int iy = 0) {

        cout << ix << " " << ix << dataMap[iy][ix] << 2 * nx * ny << " "
             << endl;
        uint16_t val = getChannel(data, ix, iy) & 0x3fff;
        return val;
    };

    int getFrameNumber(char *buff) {
        return *((int *)buff);
    }; //*((int*)(buff+5))&0xffffff;};

    int getPacketNumber(char *buff) {
        return 0;
    } //((*(((int*)(buff+4))))&0xff)+1;};

    virtual char *readNextFrame(ifstream &filebin) {
        int ff = -1, np = -1;
        return readNextFrame(filebin, ff, np);
    };

    virtual char *readNextFrame(ifstream &filebin, int &ff) {
        int np = -1;
        return readNextFrame(filebin, ff, np);
    };

    virtual char *readNextFrame(ifstream &filebin, int &ff, int &np) {
        char *data = new char[2 * nx * ny];
        char *d = readNextFrame(filebin, ff, np, data);
        if (d == NULL) {
            delete[] data;
            data = NULL;
        }
        return data;
    }

    virtual char *readNextFrame(ifstream &filebin, int &ff, int &np,
                                char *data) {
        // char *retval=0;
        //	  int  nd;
        // int fnum = -1;
        np = 0;
        //	  int  pn;

        //  if (ff>=0)
        //  fnum=ff;

        if (filebin.is_open()) {
            if (filebin.read(data, 2 * nx * ny)) {
                // iframe++;
                // ff=iframe;
                return data;
            }
        }
        return NULL;
    };

    virtual char *findNextFrame(char *data, int &ndata, int dsize) {
        if (dsize < 2 * nx * ny)
            ndata = dsize;
        else
            ndata = 2 * nx * ny;
        return data;
    }

    // virtual int setFrameNumber(int ff){iframe=ff};

    int getPacketNumber(int x, int y) { return 0; };
};

#endif
