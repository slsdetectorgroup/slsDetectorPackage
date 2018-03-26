#ifndef SINGLE_PHOTON_HIT_H
#define SINGLE_PHOTON_HIT_H
#include <stdio.h>
#include <stdint.h>

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
  single_photon_hit(int nx=3, int ny=3): dx(nx), dy(ny) {data=new int[dx*dy];};

  ~single_photon_hit(){delete [] data;};  /**< destructor, deletes the data array */

  /** binary write to file of all elements of the structure, except size of the cluster
      \param myFile file descriptor
  */
  size_t write(FILE *myFile) {
    //fwrite((void*)this, 1, 3*sizeof(int)+4*sizeof(double)+sizeof(quad), myFile); 
    if (fwrite((void*)this, 1, sizeof(int)+2*sizeof(int16_t), myFile)) 
      return fwrite((void*)data, 1, dx*dy*sizeof(int), myFile);
    return 0;
  };   
  
  /** 
      binary read from file of all elements of the structure, except size of the cluster. The structure is then filled with those args
      \param myFile file descriptor
  */
  size_t read(FILE *myFile) {
    //fread((void*)this, 1,  3*sizeof(int)+4*sizeof(double)+sizeof(quad), myFile); 
    
    if (fread((void*)this, 1,  sizeof(int)+2*sizeof(int16_t), myFile))  
      return fread((void*)data, 1, dx*dy*sizeof(int), myFile);
    return 0;
  };

    /** 
	assign the value to the element of the cluster matrix, with relative coordinates where the center of the cluster is (0,0)
	\param v value to be set
	\param ix coordinate x within the cluster (center is (0,0))
	\param iy coordinate y within the cluster (center is (0,0))
  */
  void set_data(double v, int ix, int iy=0){data[(iy+dy/2)*dx+ix+dx/2]=v;};

  void set_cluster_size(int nx, int ny) {if (nx>0) dx=nx; if (ny>0) dy=ny; delete [] data; data=new int[dx*dy];};
  void get_cluster_size(int &nx, int &ny) {nx=dx; ny=dy;};
  void get_pixel(int &x1, int &y1) {x1=x; y1=y;};

    /** 
	gets the value to the element of the cluster matrix, with relative coordinates where the center of the cluster is (0,0)
	\param ix coordinate x within the cluster (center is (0,0))
	\param iy coordinate y within the cluster (center is (0,0))
	\returns value of the cluster element
  */
  double get_data(int ix, int iy=0){return data[(iy+dy/2)*dx+ix+dx/2];};
  int *get_cluster() {return data;};

  int	iframe; 	        /**< frame number */
  int16_t 	x; 			/**< x-coordinate of the center of hit */
  int16_t	y; 			/**< x-coordinate of the center of hit */
  double	rms; 		/**< noise of central pixel l -- at some point it can be removed*/
  double 	ped; 		/**< pedestal of the central pixel -- at some point it can be removed*/
   double tot; /**< sum of the 3x3 cluster */
   quadrant quad; /**< quadrant where the photon is located */
   double quadTot; /**< sum of the maximum 2x2cluster */ 
  int dx;	                /**< size of data cluster in x */
  int dy;                 /**< size of data cluster in y */  
  int *data;		        /**< pointer to data */
};



#endif
