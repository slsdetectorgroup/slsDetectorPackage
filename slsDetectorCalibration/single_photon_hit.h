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
    
    // if (fwrite((void*)this, 1, sizeof(int)+2*sizeof(int16_t), myFile))
#ifdef OLDFORMAT 
    if (fwrite((void*)&iframe, 1,  sizeof(int), myFile)) 
#endif  
#ifndef WRITE_QUAD
    if (fwrite((void*)&x, 2,  sizeof(int16_t), myFile)) 
      return fwrite((void*)data, 1, dx*dy*sizeof(int), myFile);
#endif 
#ifdef WRITE_QUAD
    int qq[4];
    switch(quad) {
    case TOP_LEFT:
      qq[0]=data[3];
      qq[1]=data[4];
      qq[2]=data[6];
      qq[3]=data[7];
      x=x-1;
      y=y;
      break;
      
    case TOP_RIGHT:
      qq[0]=data[4];
      qq[1]=data[5];
      qq[2]=data[7];
      qq[3]=data[8];
      x=x;
      y=y;
      break;


    case BOTTOM_LEFT:
      qq[0]=data[0];
      qq[1]=data[1];
      qq[2]=data[3];
      qq[3]=data[4];
      x=x-1;
      y=y-1;
      break;
    case BOTTOM_RIGHT:
      qq[0]=data[1];
      qq[1]=data[2];
      qq[2]=data[4];
      qq[3]=data[5];
      x=x;
      y=y-1;
      break;


    default:
      ;
    }
    if (fwrite((void*)&x, 2,  sizeof(int16_t), myFile)) 
      return fwrite((void*)qq, 1, 4*sizeof(int), myFile);
#endif
    return 0;
  };   
  
  /** 
      binary read from file of all elements of the structure, except size of the cluster. The structure is then filled with those args
      \param myFile file descriptor
  */
  size_t read(FILE *myFile) {
    //fread((void*)this, 1,  3*sizeof(int)+4*sizeof(double)+sizeof(quad), myFile); 
    
#ifdef OLDFORMAT 
    if (fread((void*)&iframe, 1,  sizeof(int), myFile))  
#endif   
#ifndef WRITE_QUAD
    if (fread((void*)&x, 2,  sizeof(int16_t), myFile))  
      return fread((void*)data, 1, dx*dy*sizeof(int), myFile);
#endif 
#ifdef WRITE_QUAD
    int qq[4];
    if (fread((void*)&x, 2,  sizeof(int16_t), myFile))  
      if (fread((void*)qq, 1, 4*sizeof(int), myFile)) {

    switch(quad) {
    case TOP_LEFT:
      data[0]=0;
      data[1]=0;
      data[2]=0;
      data[3]=qq[0];
      data[4]=qq[1];
      data[5]=0;
      data[6]=qq[2];
      data[7]=qq[3];
      data[8]=0;
      x=x+1;
      y=y;
      break;
      
    case TOP_RIGHT:
      data[0]=0;
      data[1]=0;
      data[2]=0;
      data[3]=0;
      data[4]=qq[0];
      data[5]=qq[1];
      data[6]=0;
      data[7]=qq[2];
      data[8]=qq[3];
      x=x;
      y=y;
      break;


    case BOTTOM_LEFT:
      data[0]=qq[0];
      data[1]=qq[1];
      data[2]=0;
      data[3]=qq[2];
      data[4]=qq[3];
      data[5]=0;
      data[6]=0;
      data[7]=0;
      data[8]=0;
      x=x+1;
      y=y+1;
      break;
    case BOTTOM_RIGHT:
      data[0]=0;
      data[1]=qq[0];
      data[2]=qq[1];
      data[3]=0;
      data[4]=qq[2];
      data[5]=qq[3];
      data[6]=0;
      data[7]=0;
      data[8]=0;
      x=x;
      y=y+1;
      break;


    default:
      ;
    }
    return 1;
      }
#endif

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
