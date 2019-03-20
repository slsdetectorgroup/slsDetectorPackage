
#include <unistd.h>
#include <cstring>
#ifndef DETECTOR_DATA_H
#define DETECTOR_DATA_H
/**
   @short data structure to hold the detector data after postprocessing    (e.g. to plot, store in a root tree etc.)
 */
class detectorData {
 public:
  /** @short The constructor
      \param f_ind file index
      \param fname file name to which the data are saved
      \param np number of points in x coordinate defaults to the number of detector channels (1D detector) or dimension in x (2D detector)
      \param ny dimension in y (2D detector)
      \param cval pointer to data in char* format
      \param dbytes number of bytes of image pointed to by cval pointer
      \param dr dynamic range or bits per pixel
      \param file_ind file index
  */
  detectorData(double f_ind=-1,
		  const char *fname="", int np=-1, int ny=1, char *cval=NULL, int dbytes=0, int dr=0,
		  long long int file_ind=-1) :
			  progressIndex(f_ind),
			  npoints(np), npy(ny), cvalues(cval), databytes(dbytes),
			  dynamicRange(dr), dgainvalues(NULL), fileIndex(file_ind) {
	 strcpy(fileName,fname);
  };

  /**
	@short The destructor
	deletes also the arrays pointing to data/errors/angles if not NULL
	cvalues are deleted by caller
   */
    ~detectorData() {if(dgainvalues) delete [] dgainvalues;};
    //private:
    double progressIndex;/**< @short file index */
    char fileName[1000];/**< @short file name */
    int npoints;/**< @short number of points */
    int npy;/**< @short dimensions in y coordinate*/
    char* cvalues; /**< @short pointer to the data as char arary */
    double* values; //TODO! remove when not used by GUI
    int databytes; /**< @short number of bytes of data. Used with cvalues */
    int dynamicRange; /**< @short dynamic range */
    double* dgainvalues; /**< @short pointer to gain data as double array for Jungfrau only in show gain mode */
    long long int fileIndex; /**< @short file index */
};


#endif
