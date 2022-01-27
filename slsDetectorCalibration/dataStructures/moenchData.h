// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCHDATA_H
#define MOENCHDATA_H
#include "slsDetectorData.h"
#include "sls/sls_detector_defs.h"

using namespace std;
class moenchData : public slsDetectorData<uint16_t> {

  private:
    int iframe;
    int nadc;
    int sc_width;
    int sc_height;
    const int aSamples;
    const int dSamples;
    int off;
    int *adc_nr;
    int *ibit;

  public:
    /**
       Implements the slsReceiverData structure for the moench02 prototype read
       out by a module i.e. using the slsReceiver (160x160 pixels, 40 packets
       1286 large etc.) \param c crosstalk parameter for the output buffer

    */
  
 
 moenchData(int off, int nadc_i, int *adc_nr_in, int *ibit_in, int nas = 5000, int nds = 0, int nx_i=400, int ny_i=400)
   : nadc(nadc_i), slsDetectorData<uint16_t>(nx_i, ny_i,off+((nas > 0) && (nds > 0)
                                                 ? max(nas, nds) * (nadc_i * 2 + 8)
                                                 : nas * nadc_i * 2 + nds * 8)),
       sc_width(25), sc_height(200), aSamples(nas), dSamples(nds) {
      
      
      adc_nr=new int[nadc];
      if (ibit_in)
	ibit=new int[nadc];
      else
	ibit=NULL;
      for (iadc=0; iadc<nadc; iadc++) {
	adc_nr[iadc]=adc_nr_in[iadc];
	if (ibit)
	  ibit[iadc]=ibit_in[iadc];
      }

	  

            int row, col;
            int iadc;
            int i;

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
                        if (nds > 0)
                            dataMap[row][col] =
                                ((nadc + 4) * i + iadc) * 2 + off; //+16*(ip+1);
                        else
                            dataMap[row][col] =
                                (nadc * i + iadc) * 2 + off; //+16*(ip+1);
                        if (dataMap[row][col] < 0 ||
                            dataMap[row][col] >= aSamples * 2 * 32 + off)
                            cout << "Error: pointer " << dataMap[row][col]
                                 << " out of range " << endl;
                    }
                }
            }
            iframe = 0;
      }
   

        virtual int getGain(char *data, int x, int y) {
            // int aoff=aSamples*2*32;

	  if (ibit) {
            int irow;
            int isc = x / sc_width;
            int icol = x % sc_width;
            if (y < 200)
                irow = sc_height - 1 - y;
            else {
                irow = y - sc_height;
                isc++;
            }
            int isample = irow * sc_width + icol;

            uint64_t sample;
            char *ptr;
            if (isc < 0 || isc >= 32)
                return 0;
            if (ibit[isc] < 0 || ibit[isc] >= 64)
                return 0;
            if (dSamples > isample) {
                ptr = data + 32 * (isample + 1) + 8 * isample;
                sample = *((uint64_t *)ptr);
                if (sample & (1 << ibit[isc]))
		  return 1;
            } 
	  }
	  return 0;
        }

        /**

        Returns the frame number for the given dataset. Purely virtual func.
        \param buff pointer to the dataset
        \returns frame number

     */

     

        int getFrameNumber(char *buff) {
	  return (int* buff)[0];
        }; 

        /**

           Returns the packet number for the given dataset. purely virtual func
           \param buff pointer to the dataset
           \returns packet number number



        */
        int getPacketNumber(char *buff) {
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
          
            char *retval = 0;
            int nd;
            int fnum = -1;
            np = 0;
            int pn;

            //  cout << dataSize << endl;
            //if (ff >= 0)
            //    fnum = ff;

            if (filebin.is_open()) {
                if (filebin.read(data, dataSize)) {
                    ff = getFrameNumber(data);
                    np = getPacketNumber(data);
                    return data;
                }
            }
            return NULL;
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



    };

#endif

