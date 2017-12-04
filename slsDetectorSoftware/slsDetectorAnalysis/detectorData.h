
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
      \param val pointer to the data
      \param err pointer to errors
      \param ang pointer to the angles
      \param f_ind file index
      \param fname file name to which the data are saved
      \param np number of points in x coordinate defaults to the number of detector channels (1D detector)
      \param ny dimension in y (1D detector)
  */
  detectorData(double *val=NULL, double *err=NULL, double *ang=NULL,  double p_ind=-1, const char *fname="", int np=-1, int ny=1) : values(val), errors(err), angles(ang),  progressIndex(p_ind), npoints(np), npy(ny){
	 strcpy(fileName,fname);
  };
    /** 
	@short The destructor
	deletes also the arrays pointing to data/errors/angles if not NULL
    */
    ~detectorData() {if (values) delete [] values; if (errors) delete [] errors; if (angles) delete [] angles;};
    //private:
    double *values; /**< @short pointer to the data */
    double *errors; /**< @short pointer to the errors */
    double *angles;/**< @short pointer to the angles (NULL if no angular conversion) */
    double progressIndex;/**< @short file index */
    char fileName[1000];/**< @short file name */
    int npoints;/**< @short number of points */
    int npy;/**< @short dimensions in y coordinate*/
};


#endif
