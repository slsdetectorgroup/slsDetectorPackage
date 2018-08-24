
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
      \param val pointer to the data in double data type(valid only for MYTHEN)
      \param err pointer to errors
      \param ang pointer to the angles
      \param f_ind file index
      \param fname file name to which the data are saved
      \param np number of points in x coordinate defaults to the number of detector channels (1D detector) or dimension in x (2D detector)
      \param ny dimension in y (2D detector)
      \param cval pointer to data in char* format (valid only for non MYTHEN detectors)
      \param dbytes number of bytes of image pointed to by cval pointer (valid only for non MYTHEN detectors)
      \param dr dynamic range or bits per pixel (valid only for non MYTHEN detectors)
      \param file_ind file index
  */
  detectorData(double *val=NULL, double *err=NULL, double *ang=NULL,  double f_ind=-1,
		  const char *fname="", int np=-1, int ny=1, char *cval=NULL, int dbytes=0, int dr=0,
		  long long int file_ind=-1) :
			  values(val), errors(err), angles(ang),  progressIndex(f_ind),
			  npoints(np), npy(ny), cvalues(cval), databytes(dbytes),
			  dynamicRange(dr), dgainvalues(NULL), fileIndex(file_ind) {
	 strcpy(fileName,fname);
  };

  /**
	@short The destructor
	deletes also the arrays pointing to data/errors/angles if not NULL
	cvalues are deleted by caller
   */
    ~detectorData() {if (values) delete [] values; if (errors) delete [] errors;
    if (angles) delete [] angles; if(dgainvalues) delete [] dgainvalues;};
    //private:
    double *values; /**< @short pointer to the data as double array (MYTHEN only) */
    double *errors; /**< @short pointer to the errors */
    double *angles;/**< @short pointer to the angles (NULL if no angular conversion) */
    double progressIndex;/**< @short file index */
    char fileName[1000];/**< @short file name */
    int npoints;/**< @short number of points */
    int npy;/**< @short dimensions in y coordinate*/
    char* cvalues; /**< @short pointer to the data as char arary (non MYTHEN detectors) */
    int databytes; /**< @short number of bytes of data. Used with cvalues */
    int dynamicRange; /**< @short dynamic range */
    double* dgainvalues; /**< @short pointer to gain data as double array */
    long long int fileIndex; /**< @short file index */
};


#endif
