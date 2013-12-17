#ifndef SINGLE_PHOTON_HIT_H
#define SINGLE_PHOTON_HIT_h

typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;

/* /\** */
/*     @short structure for a single photon hit */
/* *\/ */
/* typedef struct{ */
/* 	double* data;		/\**< data size *\/ */
/* 	int 	x; 			/\**< x-coordinate of the center of hit *\/ */
/* 	int		y; 			/\**< x-coordinate of the center of hit *\/ */
/* 	double	rms; 		/\**< noise of central pixel *\/ */
/* 	double 	ped; 		/\**< pedestal of the central pixel *\/ */
/* 	int		iframe; 	/\**< frame number *\/ */
/* }single_photon_hit; */


class single_photon_hit {

 public:
  single_photon_hit(int nx, int ny=1): dx(nx), dy(ny) {data=new double[dx*dy];};
  ~single_photon_hit(){delete [] data;};
  void write(FILE *myFile) {fwrite((void*)this, 1, 3*sizeof(int)+2*sizeof(double), myFile); fwrite((void*)data, 1, dx*dy*sizeof(double), myFile);};
  void read(FILE *myFile) {fread((void*)this, 1,  3*sizeof(int)+2*sizeof(double), myFile); fread((void*)data, 1, dx*dy*sizeof(double), myFile);};
  void set_data(double v, int ix, int iy=0){data[(iy+dy/2)*dx+ix+dx/2]=v;};
  double get_data(int ix, int iy=0){return data[(iy+dy/2)*dx+ix+dx/2];};

  
	int 	x; 			/**< x-coordinate of the center of hit */
	int	y; 			/**< x-coordinate of the center of hit */
	double	rms; 		/**< noise of central pixel */
	double 	ped; 		/**< pedestal of the central pixel */
	int	iframe; 	/**< frame number */
  	double *data;		/**< data size */
	const int dx;
	const int dy;
};



#endif
