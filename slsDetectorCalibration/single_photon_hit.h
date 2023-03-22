// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef SINGLE_PHOTON_HIT_H
#define SINGLE_PHOTON_HIT_H
#include <stdint.h>
#include <stdio.h>

typedef double double32_t;
typedef float float32_t;
typedef int int32_t;

#ifndef DEF_QUAD
#define DEF_QUAD
enum quadrant {
    TOP_LEFT = 0,
    TOP_RIGHT = 1,
    BOTTOM_LEFT = 2,
    BOTTOM_RIGHT = 3,
    UNDEFINED_QUADRANT = -1
};
#endif

class single_photon_hit {

    /** @short Structure for a single photon hit */

  public:
    /** constructor, instantiates the data array  -- all class elements are
       public! \param nx cluster size in x direction \param ny cluster size in y
       direction (defaults to 1 for 1D detectors)
    */
    single_photon_hit(int nx = 3, int ny = 3) : dx(nx), dy(ny) {
        data = new int[dx * dy];
    };

    ~single_photon_hit() {
        delete[] data;
    }; /**< destructor, deletes the data array */

    /** binary write to file of all elements of the structure, except size of
       the cluster \param myFile file descriptor
    */
    size_t write(FILE *myFile) {
        // fwrite((void*)this, 1, 3*sizeof(int)+4*sizeof(double)+sizeof(quad),
        // myFile);  // if (fwrite((void*)this, 1,
        // sizeof(int)+2*sizeof(int16_t), myFile))
#ifndef WRITE_QUAD
        // printf("no quad ");
        if ( fwrite( (void*)&x, sizeof(int16_t), 1, myFile ) ) {
	  if ( fwrite( (void*)&y, sizeof(int16_t), 1, myFile ) )
            return fwrite((void *)data, sizeof(int), dx * dy, myFile);
	}
#endif
#ifdef WRITE_QUAD
        //  printf("quad ");
        int qq[4];
        switch (quad) {
        case TOP_LEFT:
            qq[0] = data[3];
            qq[1] = data[4];
            qq[2] = data[6];
            qq[3] = data[7];
            x = x - 1;
            y = y;
            break;

        case TOP_RIGHT:
            qq[0] = data[4];
            qq[1] = data[5];
            qq[2] = data[7];
            qq[3] = data[8];
            x = x;
            y = y;
            break;

        case BOTTOM_LEFT:
            qq[0] = data[0];
            qq[1] = data[1];
            qq[2] = data[3];
            qq[3] = data[4];
            x = x - 1;
            y = y - 1;
            break;
        case BOTTOM_RIGHT:
            qq[0] = data[1];
            qq[1] = data[2];
            qq[2] = data[4];
            qq[3] = data[5];
            x = x;
            y = y - 1;
            break;

        default:;
        }
        if (fwrite((void *)&x, sizeof(int16_t), 2, myFile))
            return fwrite((void *)qq, sizeof(int), 4, myFile);
#endif
        return 0;
    };

    /**
        binary read from file of all elements of the structure, except size of
       the cluster. The structure is then filled with those args \param myFile
       file descriptor
    */
    size_t read(FILE *myFile) {
        // fread((void*)this, 1,  3*sizeof(int)+4*sizeof(double)+sizeof(quad),
        // myFile);

#ifndef WRITE_QUAD
        //printf( "no quad \n");
	//This reads two values of size int16_t into x
	//If x is located next to y (int16_t distance), this reads the values into x and y
	//How can I be sure, this is always the case?
	//If, e.g., the memory is padded after int16_t x, do we read the padding instead of y?
	//How can I be sure the memory is packed and y follows right after x with no padding?
	//Anyway, this is dangerous if anyone, at any point, changes the order of variable declaration,
	//or uses another architecture (64 bit vs 32 bit for instance).
	/*
        if (fread((void *)&x, sizeof(int16_t), 2, myFile))
            return fread((void *)data, sizeof(int), dx * dy, myFile);
	*/
	
	//Suggestion
	if ( fread( (void*)&x, sizeof(int16_t), 1, myFile) ) {     //reads x
	  if ( fread( (void*)&y, sizeof(int16_t), 1, myFile ) )    //reads y
	  return fread( (void*)data, sizeof(int), dx*dy, myFile ); //reads and returns data
	}
	
	
#endif
#ifdef WRITE_QUAD
        int qq[4];
        printf("quad \n");
        if (fread((void *)&x, sizeof(int16_t), 2, myFile))
            if (fread((void *)qq, sizeof(int), 4, myFile)) {

                quad = TOP_RIGHT;
                /*	int mm=qq[0]; */
                /* for (int i=1; i<4; i++) { */
                /*   if (qq[i]>mm) { */
                /*     switch (i) { */
                /*     case 1:  */
                /*       quad=TOP_LEFT; */
                /*       break; */
                /*     case 2: */
                /*       quad=BOTTOM_RIGHT; */
                /*       break; */
                /*     case 3: */
                /*       quad=BOTTOM_LEFT; */
                /*       break; */
                /*     default: */
                /*       ; */
                /*     } */

                /*   } */

                /* } */

                /* switch(quad) { */
                /* case TOP_LEFT: */
                /*   data[0]=0; */
                /*   data[1]=0; */
                /*   data[2]=0; */
                /*   data[3]=qq[0]; */
                /*   data[4]=qq[1]; */
                /*   data[5]=0; */
                /*   data[6]=qq[2]; */
                /*   data[7]=qq[3]; */
                /*   data[8]=0; */
                /*   x=x+1; */
                /*   y=y; */
                /*   break; */

                /* case TOP_RIGHT: */
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
                data[3] = 0;
                data[4] = qq[0];
                data[5] = qq[1];
                data[6] = 0;
                data[7] = qq[2];
                data[8] = qq[3];
                x = x;
                y = y;
                /*  break; */

                /* case BOTTOM_LEFT: */
                /*   data[0]=qq[0]; */
                /*   data[1]=qq[1]; */
                /*   data[2]=0; */
                /*   data[3]=qq[2]; */
                /*   data[4]=qq[3]; */
                /*   data[5]=0; */
                /*   data[6]=0; */
                /*   data[7]=0; */
                /*   data[8]=0; */
                /*   x=x+1; */
                /*   y=y+1; */
                /*   break; */
                /* case BOTTOM_RIGHT: */
                /*   data[0]=0; */
                /*   data[1]=qq[0]; */
                /*   data[2]=qq[1]; */
                /*   data[3]=0; */
                /*   data[4]=qq[2]; */
                /*   data[5]=qq[3]; */
                /*   data[6]=0; */
                /*   data[7]=0; */
                /*   data[8]=0; */
                /*   x=x; */
                /*   y=y+1; */
                /*   break; */

                /* default: */
                /*   ; */
                /* } */
                return 1;
            }
#endif

        return 0;
    };

    void print() {

        // int ix, iy;

      printf("***************\n");
      printf("** %d %d ** %d %d **\n", x, y, dx, dy);
        for (int iy = 0; iy < dy; iy++) {
            for (int ix = 0; ix < dx; ix++) {
                printf("%d \t", data[ix + iy * dx]);
            }
            printf("\n");
        }
      printf("***************\n");
    }


    /**
        assign the value to the element of the cluster matrix, with relative
       coordinates where the center of the cluster is (0,0) \param v value to be
       set \param ix coordinate x within the cluster (center is (0,0)) \param iy
       coordinate y within the cluster (center is (0,0))
  */
    void set_data(double v, int ix, int iy = 0) {
        data[(iy + dy / 2) * dx + ix + dx / 2] = v;
    };

    void set_cluster_size(int nx, int ny) {
        if (nx > 0)
            dx = nx;
        if (ny > 0)
            dy = ny;
        delete[] data;
        data = new int[dx * dy];
    };
    void get_cluster_size(int &nx, int &ny) {
        nx = dx;
        ny = dy;
    };
    void get_pixel(int &x1, int &y1) {
        x1 = x;
        y1 = y;
    };

    /**
        gets the value to the element of the cluster matrix, with relative
       coordinates where the center of the cluster is (0,0) \param ix coordinate
       x within the cluster (center is (0,0)) \param iy coordinate y within the
       cluster (center is (0,0)) \returns value of the cluster element
  */
    //Why not make these const? VH
    double get_data(int ix, int iy = 0) const {
      return data[(iy + dy / 2) * dx + ix + dx / 2]; //NOTE: those are int divisions
    };
    int *get_cluster() const { return data; };

    int iframe; /**< frame number */
    double
        rms; /**< noise of central pixel l -- at some point it can be removed*/
    double ped;    /**< pedestal of the central pixel -- at some point it can be
                      removed*/
    double tot;    /**< sum of the 3x3 cluster */
    quadrant quad; /**< quadrant where the photon is located */
    double quadTot; /**< sum of the maximum 2x2cluster */
    int dx;         /**< size of data cluster in x */
    int dy;         /**< size of data cluster in y */
    int16_t x;      /**< x-coordinate of the center of hit */
    int16_t y;      /**< x-coordinate of the center of hit */
    int *data;      /**< pointer to data */
};

#endif
