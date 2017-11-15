#ifndef MY_TIFF_IO_H
#define  MY_TIFF_IO_H


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

void *WriteToTiff(float * imgData, const char * imgname, int nrow, int ncol);

float *ReadFromTiff( const char * imgname, uint32 &nrow, uint32 &ncol);

#endif
