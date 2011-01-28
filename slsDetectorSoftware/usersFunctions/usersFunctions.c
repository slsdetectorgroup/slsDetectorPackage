#include "usersFunctions.h"
#include <math.h>
#include <stdio.h>

float pos;
float i0=0;

/* 
   contains the conversion channel-angle for a module channel
   conv_r=pitch/radius
*/


float angle(int ichan, float encoder, float totalOffset, float conv_r, float center, float offset, float tilt, int direction) {
 
  (void) offset; // to avoid warning: unused parameter
  (void) tilt; // to avoid warning: unused parameter
  float ang;

  ang=180./PI*(center*conv_r+atan((float)(ichan-center)*conv_r))+encoder+totalOffset; 

  return direction*ang;

}

/* reads the encoder and returns the position */

float get_position() {
#ifdef VERBOSE
  printf("Getting motor position \n");
#endif
  return pos;
}

/* moves the encoder to position p */

int go_to_position(float p) {
#ifdef VERBOSE
  printf("Setting  motor position \n");
#endif
  pos=p;
  return pos;
}

/* moves the encoder to position p without waiting */

int go_to_position_no_wait(float p) {
#ifdef VERBOSE
  printf("Setting  motor position no wait \n");
#endif
  pos=p;
  return pos;
}


/* reads I0 and returns the intensity */

float get_i0() {
#ifdef VERBOSE
  printf("Getting I0 readout \n");
#endif
  return i0++;
}
  
