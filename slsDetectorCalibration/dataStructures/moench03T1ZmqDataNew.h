// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCH03T1ZMQDATANEW_H
#define MOENCH03T1ZMQDATANEW_H
#include "slsDetectorData.h"

class moench03T1ZmqDataNew : public slsDetectorData<uint16_t> {

  private:
    // int iframe;
    int nadc;
    int sc_width;
    int sc_height;
    const int nSamples;
    const int offset;

    double ghost[200][25];
    double xtalk;

  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

    */
    // moench03T1ZmqDataNew(int ns=5000): slsDetectorData<uint16_t>(400, 400,
    // ns*32*2+sizeof(int)), nSamples(ns), offset(sizeof(int)), xtalk(0.00021) {
    moench03T1ZmqDataNew(int ns = 5000, int oo = 2 * 2)
        : slsDetectorData<uint16_t>(400, 400, ns * 32 * 2 + oo), nSamples(ns),
          offset(oo), xtalk(0.00021) {
        std::cout << "M0.3" << std::endl;
        int nadc = 32;
        int sc_width = 25;
        int sc_height = 200;

        int adc_nr[32] = {300, 325, 350, 375, 300, 325, 350, 375, 200, 225, 250,
                          275, 200, 225, 250, 275, 100, 125, 150, 175, 100, 125,
                          150, 175, 0,   25,  50,  75,  0,   25,  50,  75};

        /*   int adc_nr[32]={350,375,150,175,350,375,150,175,	\
                       300,325,100,125,300,325,100,125,\
                       250,275,50,75,250,275,50,75,\
                       200,225,0,25,200,225,0,25};
        */
        int row, col;

        int isample;
        int iadc;
        int ix, iy;

        // int npackets=40;
        int i;
        int adc4(0);

        // for (int ip=0; ip<npackets; ip++) {
        //  for (int is=0; is<128; is++) {
        for (i = 0; i < nSamples; i++) {
            for (iadc = 0; iadc < nadc; iadc++) {
                // i=128*ip+is;
                adc4 = (int)iadc / 4;
                if (i < sc_width * sc_height) {
                    //  for (int i=0; i<sc_width*sc_height; i++) {
                    col = adc_nr[iadc] + (i % sc_width);
                    if (adc4 % 2 == 0) {
                        row = 199 - i / sc_width;
                    } else {
                        row = 200 + i / sc_width;
                    }
                    dataMap[row][col] =
                        (nadc * i + iadc) * 2 + offset; //+16*(ip+1);
                    if (dataMap[row][col] < 0 || dataMap[row][col] >= dataSize)
                        std::cout << "Error: pointer " << dataMap[row][col]
                             << " out of range " << std::endl;
                }
            }
        }
        // }

        int ii = 0;

        for (i = 0; i < dataSize; i++) {
            if (i < offset) {
                // header! */
                xmap[i] = -1;
                ymap[i] = -1;
            } else {
                // ii=ibyte+128*32*ipacket;
                isample = ii / nadc;
                if (isample < nSamples) {
                    iadc = ii % nadc;
                    adc4 = (int)iadc / 4;
                    ix = isample % sc_width;
                    iy = isample / sc_width;
                    if (adc4 % 2 == 0) {
                        xmap[i] = adc_nr[iadc] + ix;
                        ymap[i] = ny / 2 - 1 - iy;
                    } else {
                        xmap[i] = adc_nr[iadc] + ix;
                        ymap[i] = ny / 2 + iy;
                    }
                }

                ii++;
            }
        }

        for (int ix = 0; ix < 25; ix++)
            for (int iy = 0; iy < 200; iy++)
                ghost[iy][ix] = 0.;

        // iframe=0;
        //  cout << "data struct created" << endl;
    };

    double getXTalk() { return xtalk; };
    void setXTalk(double g) { xtalk = g; };

    /**
       Returns the value of the selected channel for the given dataset as
       double. \param data pointer to the dataset (including headers etc) \param
       ix pixel number in the x direction \param iy pixel number in the y
       direction \returns data for the selected channel, with inversion if
       required as double

    */
    virtual double getValue(char *data, int ix, int iy = 0) {
        /* cout << " x "<< ix << " y"<< iy << " val " << getChannel(data, ix,
         * iy)<< endl;*/
        /* double val=0, vout=getChannel(data, ix, iy); */
        /* int x1=ix%25; */
        /* for (int ix=0; ix<16; ix++) { */
        /*   for (int ii=0; ii<2; ii++) { */
        /* 	val+=getChannel(data,x1+25*ix,iy); */
        /* 	val+=getChannel(data,x1+25*ix,399-iy); */
        /*   } */
        /* } */
        /* vout+=0.0008*val-6224; */
        /* return vout; //(double)getChannel(data, ix, iy);
         */
        // cout << ix << " "<< iy << " " << dataMap[iy][ix] << endl;
        return ((double)getChannel(data, ix, iy)) + xtalk * getGhost(iy, iy);
    };

    virtual void calcGhost(char *data, int ix, int iy) {
        double val = 0;
        /* for (int ix=0; ix<25; ix++){ */
        /*   for (int iy=0; iy<200; iy++) { */
        val = 0;
        for (int isc = 0; isc < 16; isc++) {
            for (int ii = 0; ii < 2; ii++) {
                val += getChannel(data, ix + 25 * isc, iy);
                //  cout << val << " " ;
                val += getChannel(data, ix + 25 * isc, 399 - iy);
                //  cout << val << " " ;
            }
        }
        ghost[iy][ix] = val; //-6224;
                             //	cout << endl;
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

    /* class jfrau_packet_header_t { */
    /*  public: */
    /* 	unsigned char reserved[4]; */
    /* 	unsigned char packetNumber[1]; */
    /* 	unsigned char frameNumber[3]; */
    /* 	unsigned char bunchid[8]; */
    /* }; */

    int getFrameNumber(char *buff) {
        return *((int *)buff);
    }; //*((int*)(buff+5))&0xffffff;};

    /**

       Returns the packet number for the given dataset. purely virtual func
       \param buff pointer to the dataset
       \returns packet number number



    */
    int getPacketNumber(char *buff) {
        return 0;
    } //((*(((int*)(buff+4))))&0xff)+1;};

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

    virtual char *readNextFrame(std::ifstream &filebin) {
        int ff = -1, np = -1;
        return readNextFrame(filebin, ff, np);
    };

    virtual char *readNextFrame(std::ifstream &filebin, int &ff) {
        int np = -1;
        return readNextFrame(filebin, ff, np);
    };

    virtual char *readNextFrame(std::ifstream &filebin, int &ff, int &np) {
        char *data = new char[32 * 2 * nSamples];
        char *d = readNextFrame(filebin, ff, np, data);
        if (d == NULL) {
            delete[] data;
            data = NULL;
        }
        return data;
    }

    virtual char *readNextFrame(std::ifstream &filebin, int &ff, int &np,
                                char *data) {
        // char *retval=0;
        //	  int  nd;
        // int fnum = -1;
        np = 0;
        //	  int  pn;

        //  if (ff>=0)
        //  fnum=ff;

        if (filebin.is_open()) {
            if (filebin.read(data, 32 * 2 * nSamples)) {
                // iframe++;
                // ff=iframe;
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
        if (dsize < 32 * 2 * nSamples)
            ndata = dsize;
        else
            ndata = 32 * 2 * nSamples;
        return data;
    }

    // virtual int setFrameNumber(int ff){iframe=ff};

    int getPacketNumber(int x, int y) { return 0; };
};

#endif
