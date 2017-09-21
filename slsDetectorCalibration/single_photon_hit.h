#ifndef SINGLE_PHOTON_HIT_H
#define SINGLE_PHOTON_HIT_H

typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;

#ifndef DEF_QUAD
#define DEF_QUAD
  enum quadrant {
    TOP_LEFT=0,
    TOP_RIGHT=1,
    BOTTOM_LEFT=2,
    BOTTOM_RIGHT=3,
    UNDEFINED_QUADRANT=-1
  };
#endif


class single_photon_hit {

  /** @short Structure for a single photon hit */

 public: 
  /** constructor, instantiates the data array  -- all class elements are public!
      \param nx cluster size in x direction
      \param ny cluster size in y direction (defaults to 1 for 1D detectors)
  */
  single_photon_hit(int nx=3, int ny=3): dx(nx), dy(ny) {data=new double[dx*dy];};

  ~single_photon_hit(){delete [] data;};  /**< destructor, deletes the data array */

  /** binary write to file of all elements of the structure, except size of the cluster
      \param myFile file descriptor
  */
  void write(FILE *myFile) {fwrite((void*)this, 1, 3*sizeof(int)+4*sizeof(double)+sizeof(quad), myFile); fwrite((void*)data, 1, dx*dy*sizeof(double), myFile);};   
  
  /** 
      binary read from file of all elements of the structure, except size of the cluster. The structure is then filled with those args
      \param myFile file descriptor
  */
  void read(FILE *myFile) {fread((void*)this, 1,  3*sizeof(int)+4*sizeof(double)+sizeof(quad), myFile); fread((void*)data, 1, dx*dy*sizeof(double), myFile);};

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
  const int dx;	                /**< size of data cluster in x */
  const int dy;                 /**< size of data cluster in y */   
  quadrant quad; /**< quadrant where the photon is located */
  double tot; /**< sum of the 3x3 cluster */
  double quadTot; /**< sum of the maximum 2x2cluster */
  double *data;		        /**< pointer to data */
};



#endif
