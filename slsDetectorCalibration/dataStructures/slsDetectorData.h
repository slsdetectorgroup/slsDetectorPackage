// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef SLSDETECTORDATA_H
#define SLSDETECTORDATA_H

#include <fstream>
#include <iostream>

template <class dataType> class slsDetectorData {

  protected:
    const int nx;  /**< Number of pixels in the x direction */
    const int ny;  /**< Number of pixels in the y direction */
    int dataSize;  /**<size of the data constituting one frame */
 
    dataType **dataMask; /**< Array of size nx*ny storing the polarity of the
                            data in the dataset (should be 0 if no inversion is
                            required, 0xffffffff is inversion is required) */
    int **dataROIMask; /**< Array of size nx*ny 1 if channel is good (or in the
                          ROI), 0 if bad channel (or out of ROI)   */
    int *xmap;
    int *ymap;
    dataType **orderedData;
    int isOrdered;

  public:

    int **dataMap; /**< Array of size nx*ny storing the pointers to the data in
                      the dataset (as offset)*/
    /**
    General slsDetectors data structure. Works for data acquired using the
    slsDetectorReceiver. Can be generalized to other detectors (many virtual
    funcs).

    Constructor (no error checking if datasize and offsets are compatible!)
    \param npx number of pixels in the x direction
    \param npy number of pixels in the y direction (1 for strips)
    \param dsize size of the data
    \param dMap array of size nx*ny storing the pointers to the data in the
    dataset (as offset) \param dMask Array of size nx*ny storing the polarity of
    the data in the dataset (should be 0 if no inversion is required, 0xffffffff
    is inversion is required) \param dROI Array of size nx*ny. The elements are
    1s if the channel is good or in the ROI, 0 is bad or out of the ROI. NULL
    (default) means all 1s.
    */
    slsDetectorData(int npx, int npy, int dsize, int **dMap = NULL,
                    dataType **dMask = NULL, int **dROI = NULL);

    virtual ~slsDetectorData();

    // Virtual functions
    virtual int getPointer(int ix, int iy) { return dataMap[iy][ix]; };
    virtual void getPixel(int ip, int &x, int &y);
    virtual dataType **getData(char *ptr, int dsize = -1);
    virtual double **getImage(char *ptr, int dsize = -1);
    virtual dataType getChannel(char *data, int ix, int iy = 0);
    virtual int getGain(char *data, int ix, int iy = 0) { return 0; };
    virtual double getValue(char *data, int ix, int iy = 0) {
        return (double)getChannel(data, ix, iy);
    };

    virtual int getFrameNumber(char *buff) = 0;
    
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
    virtual char *findNextFrame(char *data, int &ndata, int dsize) = 0;

    //Returns a pointer to the next complete frame, if none found nullptr
    //data needs to be deallocated by caller
    virtual char *readNextFrame(std::ifstream &filebin) = 0;





    //END Virtual functions

    /**
       defines the data map (as offset) - no error checking if datasize and
       offsets are compatible! \param dMap array of size nx*ny storing the
       pointers to the data in the dataset (as offset). If NULL (default),the
       data are arranged as if read out row by row
       (dataMap[iy][ix]=(iy*nx+ix)*sizeof(dataType);)
    */
    void setDataMap(int **dMap = NULL);

    /**
       defines the data mask i.e. the polarity of the data
       \param dMask Array of size nx*ny storing the polarity of the data in the
       dataset (should be 0 if no inversion is required, 0xffffffff is inversion
       is required)

    */
    void setDataMask(dataType **dMask = NULL);
    /**
       defines the region of interest and/or the bad channels mask
       \param dROI Array of size nx*ny. The ements are 1s if the channel is
       good or in the ROI, 0 is bad or out of the ROI. NULL (default) means all
       1s.

    */
    void setDataROIMask(int **dROI = NULL);

    /**
       Define bad channel or roi mask for a single channel
       \param ix channel x coordinate
       \param iy channel y coordinate (1 for strips)
       \param i 1 if pixel is good (or in the roi), 0 if bad
       \returns 1 if pixel is good, 0 if it's bad, -1 if pixel is out of range
    */
    int setGood(int ix, int iy, int i = 1);

    // returns 1 if pixel is good, 0 if it's bad, -1 if pixel is out of range
    int isGood(int ix, int iy);

    // returns total number of channels, outparam npx and npy x,y dimensions
    int getDetectorSize(int &npx, int &npy);

    /** Returns the size of the data frame */
    int getDataSize() { return dataSize; };

    /** changes the size of the data frame */
    int setDataSize(int d) {
        dataSize = d;
        return dataSize;
    };

    void newFrame() { isOrdered = 0; };

    /**
       Returns the value of the selected channel for the given dataset. Virtual
       function, can be overloaded. \param data pointer to the dataset
       (including headers etc) \param ix pixel number in the x direction \param
       iy pixel number in the y direction \returns data for the selected
       channel, with inversion if required

    */

};

template <typename dataType>
slsDetectorData<dataType>::slsDetectorData(int npx, int npy, int dsize,
                                           int **dMap, dataType **dMask,
                                           int **dROI)
    : nx(npx), ny(npy), dataSize(dsize), orderedData(NULL), isOrdered(0) {

    int el = dsize / sizeof(dataType);
    xmap = new int[el];
    ymap = new int[el];

    orderedData = new dataType *[ny];
    dataMap = new int *[ny];
    dataMask = new dataType *[ny];
    dataROIMask = new int *[ny];
    for (int i = 0; i < ny; i++) {
        dataMap[i] = new int[nx];
        orderedData[i] = new dataType[nx];
        dataMask[i] = new dataType[nx];
        dataROIMask[i] = new int[nx];
        for (int j = 0; j < nx; j++)
            dataROIMask[i][j] = 1;
    }

    for (int ip = 0; ip < el; ip++) {
        xmap[ip] = -1;
        ymap[ip] = -1;
    }

    setDataMap(dMap);
    setDataMask(dMask);
    setDataROIMask(dROI);
}

template <typename dataType> slsDetectorData<dataType>::~slsDetectorData() {
    for (int i = 0; i < ny; i++) {
        delete[] dataMap[i];
        delete[] dataMask[i];
        delete[] dataROIMask[i];
        delete[] orderedData[i];
    }
    delete[] dataMap;
    delete[] dataMask;
    delete[] dataROIMask;
    delete[] orderedData;
    delete[] xmap;
    delete[] ymap;
}

template <typename dataType>
void slsDetectorData<dataType>::setDataMap(int **dMap) {

    // int ip = 0;
    if (dMap == NULL) {
        for (int iy = 0; iy < ny; iy++) {
            for (int ix = 0; ix < nx; ix++) {
                dataMap[iy][ix] = (iy * nx + ix) * sizeof(dataType);
            }
        }
    } else {
        // cout << "set dmap "<< dataMap << " " << dMap << endl;
        for (int iy = 0; iy < ny; iy++) {
            // cout << iy << endl;
            for (int ix = 0; ix < nx; ix++) {
                dataMap[iy][ix] = dMap[iy][ix];
                // cout << ix << " " << iy << endl;
                /*ip=dataMap[ix][iy]/sizeof(dataType);
                  xmap[ip]=ix;
                  ymap[ip]=iy;Annaa*/
            }
        }
    }
    /* //commented this part because it causes out-of-bound issues if nx or ny are larger than dataMap bounds (single-chip readout of strixel with groups of different pitches) VH 2023-02-24
    for (int iy = 0; iy < ny; iy++) {
        for (int ix = 0; ix < nx; ix++) {
            ip = dataMap[iy][ix] / sizeof(dataType);
            xmap[ip] = ix;
            ymap[ip] = iy;
        }
    }
     */
    // cout << "nx:" <<nx << " ny:" << ny << endl;
}

template <typename dataType>
void slsDetectorData<dataType>::setDataMask(dataType **dMask) {

    if (dMask != NULL) {

        for (int iy = 0; iy < ny; iy++)
            for (int ix = 0; ix < nx; ix++)
                dataMask[iy][ix] = dMask[iy][ix];
    } else {
        for (int iy = 0; iy < ny; iy++)
            for (int ix = 0; ix < nx; ix++)
                dataMask[iy][ix] = 0;
    }
}

template <typename dataType>
void slsDetectorData<dataType>::setDataROIMask(int **dROI) {
    if (dROI != NULL) {

        for (int iy = 0; iy < ny; iy++)
            for (int ix = 0; ix < nx; ix++)
                dataROIMask[iy][ix] = dROI[iy][ix];
    } else {

        for (int iy = 0; iy < ny; iy++)
            for (int ix = 0; ix < nx; ix++)
                dataROIMask[iy][ix] = 1;
    }
}

template <typename dataType>
int slsDetectorData<dataType>::setGood(int ix, int iy, int i) {
    if (ix >= 0 && ix < nx && iy >= 0 && iy < ny)
        dataROIMask[iy][ix] = i;
    return isGood(ix, iy);
}

template <typename dataType>
int slsDetectorData<dataType>::isGood(int ix, int iy) {
    if (ix >= 0 && ix < nx && iy >= 0 && iy < ny)
        return dataROIMask[iy][ix];
    else
        return -1;
};

template <typename dataType>
int slsDetectorData<dataType>::getDetectorSize(int &npx, int &npy) {
    npx = nx;
    npy = ny;
    return nx * ny;
}

template <typename dataType>
void slsDetectorData<dataType>::getPixel(int ip, int &x, int &y) {
    x = xmap[ip];
    y = ymap[ip];
}

template <typename dataType>
dataType **slsDetectorData<dataType>::getData(char *ptr, int dsize) {
    int el = dsize / sizeof(dataType);
    // dataType **data;
    // data=new dataType*[ny];
    // for(int i = 0; i < ny; i++) {
    //   data[i]=new dataType[nx];
    // }
    isOrdered = 0;
    if (dsize <= 0 || dsize > dataSize)
        dsize = dataSize;

    for (int ip = 0; ip < (el); ip++) {
        int ix, iy;
        getPixel(ip, ix, iy);
        if (ix >= 0 && ix < nx && iy >= 0 && iy < ny) {
            // data[iy][ix]=getChannel(ptr,ix,iy);
            orderedData[iy][ix] = *(ptr + ip); // getChannel(ptr,ix,iy);
        }
    }
    isOrdered = 1;
    return orderedData;
}

template <typename dataType>
double **slsDetectorData<dataType>::getImage(char *ptr, int dsize) {

    double **data;
    int ix, iy;
    data = new double *[ny];
    for (int i = 0; i < ny; i++) {
        data[i] = new double[nx];
    }
    int el = dsize / sizeof(dataType);
    if (dsize <= 0 || dsize > dataSize)
        dsize = dataSize;
    for (int ip = 0; ip < el; ip++) {
        getPixel(ip, ix, iy);
        if (ix >= 0 && ix < nx && iy >= 0 && iy < ny) {
            data[iy][ix] = getValue(ptr, ix, iy);
        }
    }
    return data;
}
template <typename dataType>
dataType slsDetectorData<dataType>::getChannel(char *data, int ix, int iy) {
    dataType m = 0, d = 0;
    if (ix >= 0 && ix < nx && iy >= 0 && iy < ny && dataMap[iy][ix] >= 0 &&
        dataMap[iy][ix] < dataSize) {
        m = dataMask[iy][ix];
        if (isOrdered == 0)
            d = *((dataType *)(data + getPointer(ix, iy)));
        else
            d = orderedData[iy][ix];
    }
    return d ^ m;
};

#endif
