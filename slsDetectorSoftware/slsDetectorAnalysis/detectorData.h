
#include <unistd.h>
#include <cstring>
#ifndef DETECTOR_DATA_H
#define DETECTOR_DATA_H
/**
   @short data structure to hold the detector data after postprocessing 

   (e.g. to plot, store in a root tree etc.)
 */
class detectorData {
 public:
  /** The constructor
      \param val pointer to the data
      \param err pointer to errors
      \param ang pointer to the angles
      \param f_ind file index
      \param fname file name to which the data are saved
      \param np number of points defaults to the number of detector channels
  */
  detectorData(float *val=NULL, float *err=NULL, float *ang=NULL,  float p_ind=-1, const char *fname="", int np=-1) : values(val), errors(err), angles(ang),  progressIndex(p_ind), npoints(np){strcpy(fileName,fname);};
    /** 
	the destructor
	deletes also the arrays pointing to data/errors/angles if not NULL
    */
    ~detectorData() {if (values) delete [] values; if (errors) delete [] errors; if (angles) delete [] angles;};
    //private:
    float *values; /**< pointer to the data */
    float *errors; /**< pointer to the errors */
    float *angles;/**< pointer to the angles */
    float progressIndex;/**< file index */
    char fileName[1000];/**< file name */
    int npoints;/**< number of points */
};


#endif
