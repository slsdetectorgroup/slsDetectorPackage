// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef DESERIALIZER_H
#define DESERIALIZER_H
#include "slsDetectorData.h"
#include <vector>

class deserializer : public slsDetectorData<int> {

  public:
    deserializer(std::vector<int> dbl, int nch = 64 * 3, int dr = 24,
                 int off = 2)
        : slsDetectorData<int>(nch, 1, nch * dr * 8 + off * 8, NULL, NULL,
                               NULL),
          dynamicRange(dr), serialOffset(off), frameNumber(0),
          numberOfCounters(nch), dbitlist(dbl){};

    deserializer(std::vector<int> dbl, int nch, int dr, int off, int ds)
        : slsDetectorData<int>(nch, 1, ds, NULL, NULL, NULL), dynamicRange(dr),
          serialOffset(off), frameNumber(0), numberOfCounters(nch),
          dbitlist(dbl){};

    virtual void getPixel(int ip, int &x, int &y) {
        x = -1;
        y = -1;
    };

    virtual int getChannel(char *data, int ix, int iy = 0) {
        int ret = -1;
        if (ix >= 0 && ix < numberOfCounters) {
            int *val = deserializeAll(data, dbitlist, dynamicRange,
                                      numberOfCounters, serialOffset);
            ret = val[ix];
            delete[] val;
        }
        return ret;
    };

    virtual int getFrameNumber(char *buff) { return frameNumber; };

    virtual char *findNextFrame(char *data, int &ndata, int dsize) {
        ndata = dsize;
        return data;
    }

    virtual char *readNextFrame(ifstream &filebin) {
        char *data = NULL;
        if (filebin.is_open()) {
            data = new char[dataSize];
            filebin.read(data, dataSize);
        }
        return data;
    }

    virtual int **getData(char *ptr, int dsize = -1) {
        int **val;
        val = new int *[1];
        val[0] = deserializeAll(ptr, dbitlist, dynamicRange, nx, serialOffset);
        return val;
    }

    static int *deserializeAll(char *ptr, std::vector<int> dbl, int dr = 24,
                               int nch = 64 * 3, int off = 5) {
        // off=0;
        // int iarg;

        cout << "** deserializer: " << endl;
        cout << "** Number of chans:\t" << nch << endl;
        cout << "** Serial Offset:\t" << off << endl;
        cout << "** Dynamic range:\t" << dr << endl;
        int64_t word, *wp, ww, one = 1, bit;
        int *val = new int[nch];
        int ioff = 0;
        int idr = 0;
        int ib = 0;
        int iw = 0;
        int ii = 0;
        int ich;
        int nb = dbl.size();
        idr = 0;
        for (ib = 0; ib < nch; ib++) {
            val[ib] = 0;
        }
        wp = (int64_t *)ptr;

        int nw = nch / nb;

        cout << "** Number of bits:\t" << nb << endl;
        cout << "** Samples:\t" << nw << endl;

        for (ib = 0; ib < nb; ib++) {
            cout << dbl[ib] << " ";
        }
        cout << endl;

        for (ib = 0; ib < nch; ib++) {
            val[ib] = 0;
        }

        for (iw = 0; iw < nw; iw) {
            word = *wp;
            ;
            if (ioff < off) {
                ioff++;
                // cout <<"*";
            } else {
                // if (idr<16) {
                // ib=0;
                //  cout << hex << "*************" << word << endl;
                for (ib = 0; ib < nb; ib++) {
                    //	for (const auto &bit : dbl) {
                    ich = iw + nch * ib / nb;
                    /* if (ich>1060) */
                    /* cout << iw << " " << idr << " " << ib << " " << bit << "
                     * " << ich << " " << val[ich] << " **  " ; */
                    bit = dbl[ib];
                    ww = one << bit;
                    if (word & (ww) && ich < nch) {
                        // cout << "+" ;
                        val[ich] |= (1 << idr);
                    }
                    /* if (ich>1060) */
                    /*   cout << val[ich]  << " " << hex << word << " " << ww <<
                     * " " << (word&(ww)) << dec <<endl; */
                    /* //else {  */
                    // cout << "-" ;
                    // }
                    // ib++;
                }

                idr++;
            }

            if (idr == dr) {
                idr = 0;
                //  cout << dec << " " << iw << " " << val[iw] << " " <<
                //  val[iw+nch/2] << endl;
                // cout <<dec << iw<<endl;
                iw++;
            } // end if()

            // end else()
            wp += 1;
            // ii++;
        } // end for

        return val;
    }

    static int *deserializeList(char *ptr, std::vector<int> dbl, int dr = 24,
                                int nch = 64 * 3, int off = 5) {
        // off=0;
        // int iarg;
        // int64_t word;
        int *val = new int[nch];
        // int ioff=0;
        int idr = 0;
        int ib = 0;
        int iw = 0;
        int ii = 0;
        int ich;
        int nb = dbl.size();
        int bit;
        char *dval;

        idr = 0;
        for (ib = 0; ib < nch; ib++) {
            val[ib] = 0;
        }
        dval = ptr;

        ib = 0;
        ich = 0;
        for (ib = 0; ib < nb; ib++) {
            //	for (const auto &bit : dbl) {

            /* if (ich>1060) */
            /* cout << iw << " " << idr << " " << ib << " " << bit << " " << ich
             * << " " << val[ich] << " **  " ; */
            bit = dbl[ib];
            // ioff=off;
            idr = 0;
            for (iw = 0; iw < (nch * dr / nb) / 8; iw++) {
                val[ich] |= (*dval) << idr;
                idr += 8;
                dval++;
                if (idr >= dr) {
                    idr = 0;
                    ich++;
                }
            }
            ii++;
            // ib++;
        } // end for

        return val;
    }

    virtual int setFrameNumber(int f = 0) {
        if (f >= 0)
            frameNumber = f;
        return frameNumber;
    };
    virtual int setDynamicRange(int d = -1) {
        if (d > 0 && d <= 24)
            dynamicRange = d;
        return dynamicRange;
    };
    virtual int setSerialOffset(int d = -1) {
        if (d >= 0)
            serialOffset = d;
        return serialOffset;
    };
    virtual int setNumberOfCounters(int d = -1) {
        if (d >= 0)
            numberOfCounters = d;
        return numberOfCounters;
    };
    virtual std::vector<int> setDBitList(std::vector<int> dbl) {
        dbitlist = dbl;
        return dbitlist;
    };
    virtual std::vector<int> getDBitList() { return dbitlist; };

  private:
    int dynamicRange;
    int serialOffset;
    int frameNumber;
    int numberOfCounters;
    std::vector<int> dbitlist;
};

#endif
