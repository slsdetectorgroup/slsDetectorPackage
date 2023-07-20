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

// #define VERSION_V2
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
constexpr int nc_rawimg = 1024; // for full images  //256;
constexpr int nr_rawimg = 512;
constexpr int nr_chip = 256;
constexpr int gr = 9;

// Group 1: 25um pitch, groups of 3, 1 column of square pixels
constexpr int g1_ncols{(nc_rawimg - (2 * gr) - 1) / 3}; // 79
constexpr int g1_nrows{((nr_chip / 4) - gr) * 3};       // 165

// Group 2: 15um pitch, groups of 5, 3 columns of square pixels
constexpr int g2_ncols{(nc_rawimg - (2 * gr) - 3) / 5}; // 47
constexpr int g2_nrows{(nr_chip / 4) * 5};              // 320

// Group 3: 18.75um pitch, groups of 4, 2 columns of square pixels (double the
// size of the other groups)
constexpr int g3_ncols{(nc_rawimg - (2 * gr) - 2) / 4}; // 59
constexpr int g3_nrows{(((nr_chip / 4) * 2) - gr) * 4}; // 476

constexpr int nc_strixel =
    2 * gr + 1 + g1_ncols; // group 1 is the "longest" group in x and has one
                           // extra square pixel
constexpr int nr_strixel = 2 * gr + g1_nrows + g2_nrows + g3_nrows;

// chip and group boundaries in ASIC coordinates (pixels at both bounds are
// included in the group) y does NOT take into account the shifts for M408!
constexpr int c1g1_xstart = 256 + gr + 1;          // 266
constexpr int c1g2_xstart = 256 + gr + 3;          // 268
constexpr int c1g3_xstart = 256 + gr + 2;          // 267
constexpr int c1_xend = 255 + 256 - gr;            // 502
constexpr int c1g1_ystart = gr;                    //  9
constexpr int c1g1_yend = 63;                      // 63
constexpr int c1g2_ystart = c1g1_yend + 1;         // 64
constexpr int c1g2_yend = c1g1_yend + 64;          // 127
constexpr int c1g3_ystart = c1g2_yend + 1;         // 128
constexpr int c1g3_yend = c1g2_yend + 2 * 64 - gr; // 246

constexpr int c6_xstart = 256 + 256 + gr;         // 521
constexpr int c6g1_xend = 255 + 2 * 256 - gr - 1; // 757
constexpr int c6g2_xend = 256 + 2 * 256 - gr - 3; // 755
constexpr int c6g3_xend = 256 + 2 * 256 - gr - 2; // 756
constexpr int c6g3_ystart = 256 + gr;             // 265
constexpr int c6g3_yend = 255 + 2 * 64;           // 383
constexpr int c6g2_ystart = c6g3_yend + 1;        // 384
constexpr int c6g2_yend = c6g3_yend + 64;         // 447
constexpr int c6g1_ystart = c6g2_yend + 1;        // 448
constexpr int c6g1_yend = c6g2_yend + 64 - gr;    // 502

// y shift due to faulty bonding (relevant for M408)
constexpr int bond_shift_y = 1; // CHANGE IF YOU CHANGE MODULE!

} // namespace strixelSingleChip

typedef struct {
    uint64_t bunchNumber; /**< is the frame number */
    uint64_t pre;         /**< something */

} jf_header; // Aldo's header

using namespace strixelSingleChip;

class jungfrauLGADStrixelsData : public slsDetectorData<uint16_t> {

  private:
    int iframe;
    int mchip;
    int chip_x0;
    int chip_y0;
    int x0, y0, x1, y1, shifty;

    int getMultiplicator(const int group) {
        int multiplicator;
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
        return multiplicator;
    }

    void setMappingShifts(const int group) {

        if (mchip == 1) {

            chip_x0 = 256;
            chip_y0 =
                bond_shift_y; // because of bump bonding issues(+1 row)  on M408

            switch (group) {
            default:
            case 1:
                x0 = 10 + chip_x0; // 9 gr + 1 sq pixel
                x1 = 246 + chip_x0;
                y0 = 9 + chip_y0;
                y1 = 63 + chip_y0;
                shifty = 0;
                break;
            case 2:
                x0 = 12 + chip_x0;
                x1 = 247 + chip_x0;
                y0 = 64 + chip_y0;
                y1 = 127 + chip_y0;
                shifty = g1_nrows;
                break;
            case 3:
                x0 = 11 + chip_x0;
                x1 = 247 + chip_x0;
                y0 = 128 + chip_y0;
                y1 = 246 + chip_y0;
                shifty = g2_nrows + g1_nrows;
                break;
            }
        }

        if (mchip == 6) {

            chip_x0 = 512;
            chip_y0 =
                256 - bond_shift_y; // should be 256 but is 255 because of bump
                                    // bonding issues (+1 row) on M408

            switch (group) {
            default:
            case 1:

                x0 = 9 + chip_x0; // 9 gr sq pixel
                x1 = 245 + chip_x0; // was 246
                y0 = 192 + chip_y0;
                y1 = 246 + chip_y0;

                shifty = g1_nrows + 2 * g2_nrows + 2 * g3_nrows;
                break;

            case 2:

                x0 = 9 + chip_x0;
                x1 = 243 + chip_x0; //was 244
                y0 = 128 + chip_y0;
                y1 = 191 + chip_y0;

                shifty = g1_nrows + g2_nrows + 2 * g3_nrows;
                ;
                break;
            case 3:

                x0 = 9 + chip_x0;
                x1 = 244 + chip_x0;
                y0 = 9 + chip_y0;
                y1 = 127 + chip_y0;
                shifty = g1_nrows + g2_nrows + g3_nrows;

                break;
            }
        }
    }

    void remapGroup(const int group) {
        int multiplicator = getMultiplicator(group);
        //int shiftx;
        int ix, iy = 0;

        setMappingShifts(group);

        // remapping loop
        for (int ipy = y0; ipy <= y1; ipy++) {
            for (int ipx = x0; ipx <= x1; ipx++) {

                ix = int((ipx - x0) / multiplicator);
                for (int m = 0; m < multiplicator; m++) {
                    if ((ipx - x0) % multiplicator == m)
                        iy = (ipy - y0) * multiplicator + m + shifty;
                }

                //	  if (iy< 40)	  cout << iy << "  " << ix <<endl;
                dataMap[iy][ix] = sizeof(header) + (nc_rawimg * ipy + ipx) * 2;
                groupmap[iy][ix] = group - 1;
            }
        }
    }

    void remapROI(uint16_t xmin, uint16_t xmax, uint16_t ymin, uint16_t ymax) {
        // determine group and chip selected by ROI
        int group;
        if (ymax <= c1g1_yend + bond_shift_y) {
            group = 1;
            mchip = 1;
        } else if (ymax <= c1g2_yend + bond_shift_y) {
            group = 2;
            mchip = 1;
        } else if (ymax <= c1g3_yend + bond_shift_y) {
            group = 3;
            mchip = 1;
        } else if (ymax <= c6g3_yend - bond_shift_y) {
            group = 3;
            mchip = 6;
        } else if (ymax <= c6g2_yend - bond_shift_y) {
            group = 2;
            mchip = 6;
        } else if (ymax <= c6g1_yend - bond_shift_y) {
            group = 1;
            mchip = 6;
	} else if (ymax <= 511) {
            group = 1;
            mchip = 6;
        } else { //to fix compiler warning
	  group = -1;
	  mchip = -1;
	}
        int multiplicator = getMultiplicator(group);
        setMappingShifts(group);

        std::cout << "chip: " << mchip << ", group: " << group << ", m: " << multiplicator
                  << ", x0: " << x0 << ", x1: " << x1 << ", y0: " << y0
                  << ", y1: " << y1 << std::endl;

        // get ROI raw image number of columns
        int nc_roi = xmax - xmin + 1;
        std::cout << "nc_roi = " << nc_roi << std::endl;

        // make sure loop bounds are correct
        if (y0 < ymin)
            std::cout << "Error ymin" << std::endl;
        if (y1 > ymax)
            std::cout << "Error ymax - normal for G3 since ROI only 64 row"
                      << std::endl;
        if (x0 < xmin)
            std::cout << "Error xmin" << std::endl;
        if (x1 > xmax)
            std::cout << "Error xmax" << std::endl;

        // remapping loop
        int ix, iy = 0;
        for (int ipy = y0; ipy <= y1; ++ipy) {
            for (int ipx = x0; ipx <= x1; ++ipx) {

                ix = int((ipx - x0 /*-xmin*/) / multiplicator);
                for (int m = 0; m < multiplicator; m++) {
                    if ((ipx - x0 /*-xmin*/) % multiplicator == m)
                        iy = (ipy - y0 /*-ymin*/) * multiplicator + m + shifty;
                }

                //	  if (iy< 40)	  cout << iy << "  " << ix <<endl;
                dataMap[iy][ix] =
                    sizeof(header) + (nc_roi * (ipy - ymin) + (ipx - xmin)) * 2;
                groupmap[iy][ix] = group - 1;
            }
        }
    }

  public:
    int groupmap[512 * 5][1024 / 3];

    using header = sls::defs::sls_receiver_header;

    jungfrauLGADStrixelsData(uint16_t xmin = 0, uint16_t xmax = 0,
                             uint16_t ymin = 0, uint16_t ymax = 0)
        : slsDetectorData<uint16_t>(
              /*nc_strixel*/ g1_ncols,
              /*nr_strixel*/ 2 * g1_nrows + 2 * g2_nrows + 2 * g3_nrows,
              g1_ncols * (2 * g1_nrows + 2 * g2_nrows + 2 * g3_nrows) * 2 +
                  sizeof(header)) {
        std::cout << "Jungfrau strixels 2X single chip with full module data "
                  << std::endl;

        // Fill all strixels with dummy values
        for (int ix = 0; ix != g1_ncols; ++ix) {
            for (int iy = 0; iy != 2 * g1_nrows + 2 * g2_nrows + 2 * g3_nrows;
                 ++iy) {
                dataMap[iy][ix] = sizeof(header);
            }
        }

        std::cout << "sizeofheader = " << sizeof(header) << std::endl;
        std::cout << "Jungfrau strixels 2X single chip with full module data "
                  << std::endl;

        if (xmin < xmax && ymin < ymax) {

            dataSize =
                (xmax - xmin + 1) * (ymax - ymin + 1) * 2 + sizeof(header);
            std::cout << "datasize " << dataSize << std::endl;
            remapROI(xmin, xmax, ymin, ymax);

        } else {

            mchip = 1;
	    for (int group=1; group!=4; ++group)
	      remapGroup(group);

            mchip = 6;
	    for (int group=1; group!=4; ++group)
	      remapGroup(group);

        }

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
#ifdef ALDO                                      // VH
        return ((jf_header *)buff)->bunchNumber; // VH
#endif                                           // VH
        return ((header *)buff)->detHeader.frameNumber;
    };

    /**

       Returns the packet number for the given dataset. purely virtual func
       \param buff pointer to the dataset
       \returns packet number number



    */
    int getPacketNumber(char *buff) {
#ifdef ALDO // VH
        // uint32_t fakePacketNumber = 1000;
        // return fakePacketNumber; //VH //TODO: Keep in mind in case of bugs!
        // //This is definitely bad!
        return 1000;
#endif // VH
        return ((header *)buff)->detHeader.packetNumber;
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

    char *readNextFrame(std::ifstream &filebin, int &ff, int &np, char *data) {
      //char *retval = 0;
      //int nd;
      //int fnum = -1;
        np = 0;
        //int pn;

        //std::cout << dataSize << std::endl;
        //if (ff >= 0) {
          //  fnum = ff; }

        if (filebin.is_open()) {
            if (filebin.read(data, dataSize)) {
                std::cout << "*";
                ff = getFrameNumber(data);
                np = getPacketNumber(data);
                return data;
            }
	    std::cout << "#";
        } else {
	  std::cout << "File not open" << std::endl;
	}
        return NULL;
    };

    /*    Loops over a memory slot until a complete frame is found (i.e. all */
    /*    packets 0 to nPackets, same frame number). purely virtual func \param
     */
    /*    data pointer to the memory to be analyzed \param ndata reference to
     * the */
    /*    amount of data found for the frame, in case the frame is incomplete at
     */
    /*    the end of the memory slot \param dsize size of the memory slot to be
     */
    /*    analyzed \returns pointer to the beginning of the last good frame
     * (might */
    /*    be incomplete if ndata smaller than dataSize), or NULL if no frame is
     */
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
