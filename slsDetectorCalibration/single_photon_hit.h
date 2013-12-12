#ifndef SINGLE_PHOTON_HIT_H
#define SINGLE_PHOTON_HIT_h

typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;


class single_photon_hit {

  /** @short Structure for a single photon hit */

 public: 
  /** constructor, instantiates the data array  -- all class elements are public!
      \param nx cluster size in x direction
      \param ny cluster size in y direction (defaults to 1 for 1D detectors)
  */
  single_photon_hit(int nx, int ny=1): dx(nx), dy(ny) {data=new double[dx*dy];};

  ~single_photon_hit(){delete [] data;};  /**< destructor, deletes the data array */

  /** binary write to file of all elements of the structure, except size of the cluster
      \param myFile file descriptor
  */
  void write(FILE *myFile) {fwrite((void*)this, 1, 3*sizeof(int)+2*sizeof(double), myFile); fwrite((void*)data, 1, dx*dy*sizeof(double), myFile);};   
  
  /** 
      binary read from file of all elements of the structure, except size of the cluster. The structure is then filled with those args
      \param myFile file descriptor
  */
  void read(FILE *myFile) {fread((void*)this, 1,  3*sizeof(int)+2*sizeof(double), myFile); fread((void*)data, 1, dx*dy*sizeof(double), myFile);};

    /** 
	assign the value to the element of the cluster matrix, with relative coordinates where the center of the cluster is (0,0)
	\param v value to be set
	\param ix coordinate x within the cluster (center is (0,0))
	\param iy coordinate y within the cluster (center is (0,0))
  */
  void set_data(double v, int ix, int iy=0){data[(iy+dy/2)*dx+ix+dx/2]=v;};


    /** 
	gets the value to the element of the cluster matrix, with relative coordinates where the center of the cluster is (0,0)
	\param ix coordinate x within the cluster (center is (0,0))
	\param iy coordinate y within the cluster (center is (0,0))
	\returns value of the cluster element
  */
  double get_data(int ix, int iy=0){return data[(iy+dy/2)*dx+ix+dx/2];};
  
  int 	x; 			/**< x-coordinate of the center of hit */
  int	y; 			/**< x-coordinate of the center of hit */
  double	rms; 		/**< noise of central pixel l -- at some point it can be removed*/
  double 	ped; 		/**< pedestal of the central pixel -- at some point it can be removed*/
  int	iframe; 	        /**< frame number */
  double *data;		        /**< pointer to data */
  const int dx;	                /**< size of data cluster in x */
  const int dy;                 /**< size of data cluster in y */
};



#endif
