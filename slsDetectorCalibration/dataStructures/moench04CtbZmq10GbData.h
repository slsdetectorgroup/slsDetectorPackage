// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCH04ZMQ10GBDATA_H
#define MOENCH04ZMQ10GBDATA_H
#include "slsDetectorData.h"
#include "sls/sls_detector_defs.h"

#define CTB

using namespace std;
class moench04CtbZmq10GbData : public slsDetectorData<uint16_t> {

  private:
    int iframe;
    int nadc;
    int sc_width;
    int sc_height;
    const int aSamples;
    const int dSamples;
    int off;

    // Single point of definition if we need to customize
    using header = sls::defs::sls_receiver_header;

  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

    */
    // moench04CtbZmq10GbData(int nas=5000, int nds=0):
    // slsDetectorData<uint16_t>(400, 400, nas*2*32+nds*8), aSamples(nas),
    // dSamples(nds), nadc(32), sc_width(25), sc_height(200) {
 
        moench04CtbZmq10GbData(int nas = 5000, int nds = 0, int oo = 2 * 2)
            : slsDetectorData<uint16_t>(400, 400,
#ifdef RAWDATA
                                        sizeof(slsDetectorDefs::sls_receiver_header) +
#endif
                                            ((nas > 0) && (nds > 0)
                                                 ? max(nas, nds) * (32 * 2 + 8)
                                                 : nas * 32 * 2 + nds * 8)),
      nadc(32), sc_width(25), sc_height(200), aSamples(nas), dSamples(nds), off(oo) {
#ifdef RAWDATA
      off=sizeof(slsDetectorDefs::sls_receiver_header);
#endif
#ifndef RAWDATA
#ifndef CTB
      off=sizeof(int);
#endif
#ifdef CTB
      off=0;
#endif
#endif

    cout << "off is " << off << endl;
	  
	if (off>0)
	    cout << "M04 RAW DATA NEW " << endl;
	else
	    cout << "M04 ZMQ DATA NEW " << endl;
	    
    int adc_nr[32] = {9,  8,  11, 10, 13, 12, 15, 14, 1,  0,  3,
                        2,  5,  4,  7,  6,  23, 22, 21, 20, 19, 18,
                        17, 16, 31, 30, 29, 28, 27, 26, 25, 24};

    

    int row, col;

    // int isample;
    int iadc;
    // int ix, iy;

    // int npackets=40;
    int i;
    // int adc4(0);

            for (int is = 0; is < aSamples; is++) {

                for (iadc = 0; iadc < nadc; iadc++) {
                    i = is;
                    //	  adc4=(int)iadc/4;
                    if (i < sc_width * sc_height) {
                        //  for (int i=0; i<sc_width*sc_height; i++) {
                        col = (adc_nr[iadc] % 16) * sc_width + (i % sc_width);
                        // if (adc4%2==0) {
                        if (iadc < 16) {
                            row = 199 - i / sc_width;
                        } else {
                            row = 200 + i / sc_width;
                        }
			dataMap[row][col] =
			  ((nadc ) * i + iadc) * 2 + off; 
                        if (dataMap[row][col] < 0 ||
                            dataMap[row][col] >= aSamples * 2 * 32 +  off)
                            cout << "Error: pointer " << dataMap[row][col]
                                 << " out of range " << endl;
                    }
                }
            }


            iframe = 0;
            //  cout << "data struct created" << endl;
      }
   

        int getGain(char *data, int x, int y) {
            // int aoff=aSamples*2*32;
            int aoff=off+aSamples*2*32;
            int irow;
            int isc = x / sc_width;
            int icol = x % sc_width;
            if (y < 200)
                irow = sc_height - 1 - y;
            else {
                irow = y - sc_height;
                isc++;
            }
            int ibit[32] = {-1, -1, -1, -1, -1, -1, 1,  3,  5,  7,  -1,
                            -1, -1, -1, -1, -1, 62, 60, 58, 56, 54, 52,
                            50, 48, 63, 61, 59, 57, 55, 53, 51, 49};
            int isample = irow * sc_width + icol;

            uint64_t sample;
            char *ptr;
            if (isc < 0 || isc >= 32)
                return 0;
            if (ibit[isc] < 0 || ibit[isc] >= 64)
                return 0;
            if (dSamples > isample) {
	      ptr = data + aoff + 8 * isample;
              //  ptr = data + 32 * (isample + 1) + 8 * isample;
                sample = *((uint64_t *)ptr);
                // cout << isc << " " << ibit[isc] << " " << isample << hex <<
                // sample << dec << endl;
                if (sample & (1 << ibit[isc]))
                    return 1;
                else
                    return 0;
            } else
                return 0;
        }

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
#ifdef RAWDATA
	  return ((slsDetectorDefs::sls_receiver_header *)buff)->detHeader.frameNumber;
#endif
	    return (int)(*buff);
        }; 

        /**

           Returns the packet number for the given dataset. purely virtual func
           \param buff pointer to the dataset
           \returns packet number number



        */
        int getPacketNumber(char *buff) {
#ifdef RAWDATA
	  return ((slsDetectorDefs::sls_receiver_header *)buff)->detHeader.packetNumber;
#endif
	    return 0;
	    
        }


 
        virtual char *readNextFrame(ifstream & filebin) {
            int ff = -1, np = -1;
            return readNextFrame(filebin, ff, np);
        };

        virtual char *readNextFrame(ifstream & filebin, int &ff) {
            int np = -1;
            return readNextFrame(filebin, ff, np);
        };

        virtual char *readNextFrame(ifstream & filebin, int &ff, int &np) {
            char *data = new char[dataSize];
            char *d = readNextFrame(filebin, ff, np, data);
            if (d == NULL) {
                delete[] data;
                data = NULL;
            }
            return data;
        }


        virtual char *readNextFrame(ifstream & filebin, int &ff, int &np,
                                    char *data) {
            // char *retval=0;
#ifndef RAWDATA   

            // int  nd;
            // int fnum = -1;
            np = 0;
            // int  pn;

            //  cout << dataSize << endl;
            if (ff >= 0)
                // fnum=ff;

                if (filebin.is_open()) {
                    if (filebin.read(data, dataSize)) {
                        ff = getFrameNumber(data);
                        //     np=getPacketNumber(data);
                        return data;
                    }
                }
            return NULL;
#endif

#ifdef RAWDATA
            //int nd;
            int fnum = -1;
            np = 0;
            //int pn;

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

#endif
        };

        /**

           Loops over a memory slot until a complete frame is found (i.e. all
           packets 0 to nPackets, same frame number). purely virtual func \param
           data pointer to the memory to be analyzed \param ndata reference to
           the amount of data found for the frame, in case the frame is
           incomplete at the end of the memory slot \param dsize size of the
           memory slot to be analyzed \returns pointer to the beginning of the
           last good frame (might be incomplete if ndata smaller than dataSize),
           or NULL if no frame is found

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

