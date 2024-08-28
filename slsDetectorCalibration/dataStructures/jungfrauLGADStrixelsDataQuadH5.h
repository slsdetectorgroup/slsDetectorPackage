// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef JUNGFRAULGADSTRIXELSDATAQUADH5_H
#define JUNGFRAULGADSTRIXELSDATAQUADH5_H
#ifdef CINT
#include "sls/sls_detector_defs_CINT.h"
#else
#include "sls/sls_detector_defs.h"
#endif
#include "slsDetectorData.h"

//This needs to be linked correctly
#include "HDF5File.h" //this includes hdf5.h and hdf5_hl.h
#include "HDF5File.cpp"

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

#include <algorithm>
#include <numeric>
#include <tuple>

namespace strixelQuad {
  constexpr int nc_rawimg = 1024; // for full images  //256;
  constexpr int nc_quad = 512;
  constexpr int nr_rawimg = 512;
  constexpr int nr_chip = 256;
  constexpr int gr = 9;

  //shift due to extra pixels
  constexpr int shift_x = 2; //left
  
  constexpr int nc_strixel = ( nc_quad - shift_x - 2*gr ) / 3; //164
  constexpr int nr_strixel = ( nr_chip - 1 - gr ) * 3; //one half (-1 because double sided pixel) //738
  constexpr int nr_center = 12; //double sided pixels to be skipped

  // boundaries in ASIC coordinates (pixels at both bounds are included)
  constexpr int xstart = 256 + gr;             // 265
  constexpr int xend = 255 + nc_quad - gr;     // 758
  constexpr int bottom_ystart = gr;            //   9
  constexpr int bottom_yend = nr_chip - 2;     // 254
  constexpr int top_ystart = nr_chip + 1;      // 257
  constexpr int top_yend = nr_chip*2 - gr - 1; // 502

  // x shift because of 2-pixel strixels on one side
  constexpr int shift = 2;

} // namespace strixelQuad

//to account for module rotation
enum rotation {
  NORMAL = 0,
  INVERSE = 1
};

const int rota = NORMAL;

typedef struct {
    uint64_t bunchNumber; /**< is the frame number */
    uint64_t pre;         /**< something */

} jf_header; // Aldo's header

using namespace strixelQuad;

class jungfrauLGADStrixelsDataQuadH5 : public slsDetectorData<uint16_t> {

  private:
    int iframe;
    int x0, y0, x1, y1, shifty;
    struct {
      uint16_t xmin;
      uint16_t xmax;
      uint16_t ymin;
      uint16_t ymax;
      int nc;
    } globalROI;

    //to account for the inverted routing of the two different quad halfs
    enum location {
      BOTTOM = 0,
      TOP = 1
    };

    int multiplicator = 3;
    std::vector<int> mods{ 0, 1, 2 };

    void reverseVector( std::vector<int>& v ) {
      std::reverse( v.begin(), v.end() );
      std::cout << "mods reversed ";
      for ( auto i : v )
	std::cout << i << " ";
      std::cout << '\n';
    }

    void setMappingShifts( const int rot, const int half ) {

      x0 = xstart;
      x1 = xend;

      if (rot==NORMAL) {
	x0 += shift;
      } else {
	x1-=shift;
        reverseVector(mods);
      }

      if (half==BOTTOM) {
	y0 = bottom_ystart;
	y1 = bottom_yend;
	shifty = 0;
      } else {
	y0 = top_ystart;
	y1 = top_yend;
	reverseVector(mods);
	shifty = nr_strixel + nr_center; //double-sided pixels in the center have to be jumped
      }

    }

    void remap( int xmin=0, int xmax=0, int ymin=0, int ymax=0 ) {

      int ix, iy = 0;
      // remapping loop
        for (int ipy = y0; ipy <= y1; ipy++) {
            for (int ipx = x0; ipx <= x1; ipx++) {

                ix = int((ipx - x0) / multiplicator);
                for (int m = 0; m < multiplicator; ++m) {
                    if ((ipx - x0) % multiplicator == m)
                        iy = (ipy - y0) * multiplicator + mods[m] + shifty;
                }

                //	  if (iy< 40)	  cout << iy << "  " << ix <<endl;
		if (xmin < xmax && ymin < ymax) {
		  if ( ipx>=xmin && ipx<=xmax && ipy>=ymin && ipy <=ymax )
		    dataMap[iy][ix] =
		      (globalROI.nc * (ipy - globalROI.ymin) + (ipx - globalROI.xmin)) * 2;
		} else {
		  dataMap[iy][ix] = (nc_rawimg * ipy + ipx) * 2;
		}
            }
        }

    }

    void remapQuad(const int rot) {

        setMappingShifts( rot, BOTTOM );
	remap();
	setMappingShifts( rot, TOP );
	remap();

    }


    std::tuple< uint16_t, uint16_t, uint16_t, uint16_t > adjustROItoLimits(uint16_t xmin,
									   uint16_t xmax,
									   uint16_t ymin,
									   uint16_t ymax,
									   uint16_t lim_roi_xmin,
									   uint16_t lim_roi_xmax,
									   uint16_t lim_roi_ymin,
									   uint16_t lim_roi_ymax) {
      uint16_t xmin_roi, xmax_roi, ymin_roi, ymax_roi;
      if ( xmin < lim_roi_xmin)
        xmin_roi = lim_roi_xmin;
      else
        xmin_roi = xmin;
      if ( xmax > lim_roi_xmax )
        xmax_roi = lim_roi_xmax;
      else
        xmax_roi = xmax;
      if ( ymin < lim_roi_ymin )
        ymin_roi = lim_roi_ymin;
      else
        ymin_roi = ymin;
      if ( ymax > lim_roi_ymax )
        ymax_roi = lim_roi_ymax;
      else
        ymax_roi = ymax;
      return std::make_tuple(xmin_roi, xmax_roi, ymin_roi, ymax_roi);
    }

    std::vector < std::tuple< int, uint16_t, uint16_t, uint16_t, uint16_t > > mapSubROIs(uint16_t xmin,
											 uint16_t xmax,
											 uint16_t ymin,
											 uint16_t ymax) {
      bool bottom = false;
      bool top = false;

      for ( int x=xmin; x!=xmax+1; ++x ) {
	for ( int y=ymin; y!=ymax; ++y ) {
	  if ( xstart<=x && x<=xend && bottom_ystart<=y && y<=bottom_yend )
	    bottom = true;
	  if ( xstart<=x && x<=xend && top_ystart<=y && y<=top_yend )
	    top = true;
	}
      }

      uint16_t xmin_roi{}, xmax_roi{}, ymin_roi{}, ymax_roi{};
      std::vector < std::tuple< int, uint16_t, uint16_t, uint16_t, uint16_t > > rois{};

      if (bottom) {
	std::tie( xmin_roi, xmax_roi, ymin_roi, ymax_roi ) =
	  adjustROItoLimits( xmin, xmax, ymin, ymax,
			     xstart, xend, bottom_ystart, bottom_yend );
	rois.push_back( std::make_tuple( BOTTOM, xmin_roi, xmax_roi, ymin_roi, ymax_roi ) );
      }
      if (top) {
	std::tie( xmin_roi, xmax_roi, ymin_roi, ymax_roi ) =
	  adjustROItoLimits( xmin, xmax, ymin, ymax,
			     xstart, xend, top_ystart, top_yend );
	rois.push_back( std::make_tuple( TOP, xmin_roi, xmax_roi, ymin_roi, ymax_roi ) );
      }

      return rois;
    }

    void remapROI(std::tuple< int, uint16_t, uint16_t, uint16_t, uint16_t > roi, const int rot ) {

      int half, xmin, xmax, ymin, ymax;
      std::tie( half, xmin, xmax, ymin, ymax ) = roi;
	
      setMappingShifts(rot, half);

      std::cout << "remapping roi: "
		<< ", x0: " << x0 << ", x1: " << x1 << ", y0: " << y0
		<< ", y1: " << y1 << std::endl;
      std::cout << "Adjusted roi: [" << xmin << ", " << xmax << ", " << ymin << ", " << ymax << "]" << std::endl;

      remap( xmin, xmax, ymin, ymax );
    }

    //The following functions are pure virtual in the base class. But I don't want them to be accessible here!
    //Implement the functions as private (to satisfy the linker)
    int getFrameNumber(char* buff){return 0;} //Provided via public method readNextFrame
    int getPacketNumber(char* buff){return 0;} //Not provided

    //Mark overwritten functions as override final
    char* readNextFrame( std::ifstream &filebin ) override final {return nullptr;}

  public:

    using header = sls::defs::sls_receiver_header;

    jungfrauLGADStrixelsDataQuadH5(uint16_t xmin = 0, uint16_t xmax = 0,
				   uint16_t ymin = 0, uint16_t ymax = 0)
        : slsDetectorData<uint16_t>(
              nc_strixel,
              nr_strixel * 2 + nr_center,
              nc_strixel * ( nr_strixel * 2 + nr_center ) * 2 ) {
        std::cout << "Jungfrau strixels quad with full module data "
                  << std::endl;

        // Fill all strixels with dummy values
        for (int ix = 0; ix != nc_strixel; ++ix) {
            for (int iy = 0; iy != nr_strixel * 2 + nr_center; ++iy) {
	      dataMap[iy][ix] = sizeof(header); //mayb another value is safer
            }
        }

	globalROI.xmin = xmin;
	globalROI.xmax = xmax;
	globalROI.ymin = ymin;
	globalROI.ymax = ymax;

        //std::cout << "sizeofheader = " << sizeof(header) << std::endl;
        std::cout << "Jungfrau strixels quad with full module data "
                  << std::endl;

        if (xmin < xmax && ymin < ymax) {

	  // get ROI raw image number of columns
	  globalROI.nc = xmax - xmin + 1;
	  std::cout << "nc_roi = " << globalROI.nc << std::endl;
	  
	  dataSize =
	    (xmax - xmin + 1) * (ymax - ymin + 1) * 2;
	  std::cout << "datasize " << dataSize << std::endl;

	  auto rois = mapSubROIs(xmin, xmax, ymin, ymax);
	  //function to fill vector of rois from globalROI

	  for ( auto roi : rois )
	    remapROI(roi, rota);

        } else {

	  remapQuad( rota );

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
    virtual double getValue(char* data, int ix, int iy = 0) {

        uint16_t val = getChannel(data, ix, iy) & 0x3fff;
        return val;
    };   
    

    char* readNextFrame( HDF5File& hfile ) {
        int fn = 0, iframe = 0;
        return readNextFrame(hfile, fn, iframe);
    };

    char* readNextFrame( HDF5File& hfile, int& fn ) {
        int iframe = 0;
        return readNextFrame(hfile, fn, iframe);
    };

    char* readNextFrame( HDF5File& hfile, int& fn, int& iframe ) {

      // Ensure dataSize is a valid size for allocation
      if (dataSize <= 0) {
        // Handle error case appropriately, e.g., log an error message
        return nullptr;
      }

      char* data = new char[dataSize];
      char* readResult = readNextFrame(hfile, fn, iframe, data);
      
      // Check if reading failed
      if (readResult == nullptr) {
        delete[] data;  // Free allocated memory
        data = nullptr;  // Set to nullptr to avoid dangling pointer
      }
      
      return data; //returning data is equivalent to returning reinterpret_cast<char*>(data_ptr) as they both point to the same memory
      
    };


    /*
     * This is the most recent function. This is used in the cluster finder!
     * The overloads are legacy!
     * Note that caller has to allocate and deallocate memory for data!
     */
    char* readNextFrame( HDF5File& hfile, int& framenumber, int& iframe, char* data ) {

      if (iframe >= 0) {
        std::cout << "*";

	//Storing the reinterpret_cast in the variable data_ptr ensures that I can pass it to a function that expects at uint16_t*
	uint16_t* data_ptr = reinterpret_cast<uint16_t*>(data); //now data_ptr points where data points (thus modifies the same memory)

        iframe = hfile.ReadImage( data_ptr, framenumber );
        return data; // return reinterpret_cast<char*>(data_ptr); // Equivalent
      }

      std::cout << "#";
      return nullptr;
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
    virtual char* findNextFrame(char* data, int& ndata, int dsize) {
        if (dsize < dataSize)
            ndata = dsize;
        else
            ndata = dataSize;
        return data;
    };

    // int getPacketNumber(int x, int y) {return dataMap[y][x]/packetSize;};
};

#endif
