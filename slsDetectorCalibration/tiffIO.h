#ifndef TIFF_IO_H
#define  TIFF_IO_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <fstream>


/*****************************************************************************/
//
//CBFlib must be installed to use this program
//
/*****************************************************************************/
#include "tiffio.h"

#undef cbf_failnez 
#define cbf_failnez(x)				\
  {						\
  int err;	        			\
  err = (x);					\
  if (err) {						\
  fprintf(stderr,"\nCBFlib fatal error %x \n",err);	\
  exit(-1);						\
  }							\
  }

void *WriteToTiff(float * imgData, const char * imgname, int nrow, int ncol){
  int sampleperpixel=1;
  // unsigned char * buff=NULL;
  tsize_t linebytes;

  TIFF * tif = TIFFOpen(imgname,"w");
  if (tif) {
    TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,ncol);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, nrow);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL,sampleperpixel);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(float)*8);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT); 
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    
    linebytes = sampleperpixel*ncol;
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, ncol*sampleperpixel));
    for(int irow=0; irow<nrow; irow++){
      TIFFWriteScanline(tif,&imgData[irow*ncol],irow,0);
    }
    
    TIFFClose(tif);
  } else
    cout << "could not open file " << imgname << " for writing " << endl;

  return NULL;
};

float *ReadFromTiff( const char * imgname, uint32 &nrow, uint32 &ncol){
  // unsigned char * buff=NULL;

  TIFF * tif = TIFFOpen(imgname,"r");
  if (tif){
        uint32 bps;
	uint32 sampleperpixel=1;
  tsize_t linebytes;

 uint32 imagelength;

    TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&ncol);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &nrow);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL,sampleperpixel);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imagelength);

    float * imgData=new float[ncol*nrow];
    linebytes = sampleperpixel*ncol;
    //  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, ncol*sampleperpixel));
    for(int irow=0; irow<nrow; irow++){
      //tiffreadscanline(tif, buf, row);
      TIFFReadScanline(tif,&imgData[irow*ncol],irow);
    }

    TIFFClose(tif);
    return imgData;
  }  else
    cout << "could not open file " << imgname << " for reading " << endl;
  return NULL;
};

#endif
